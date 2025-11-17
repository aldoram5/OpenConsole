#include "GameScanner.h"
#include "utils/FileSystemUtil.h"
#include "Log.h"
#include <chrono>

namespace OpenConsole
{

GameScanner::GameScanner()
	: mSkipDuplicates(true)
	, mUpdateExisting(false)
{
}

GameScanner::~GameScanner()
{
}

ScanStats GameScanner::scanAllSources(ScanProgressCallback progressCallback)
{
	ScanStats stats;
	auto startTime = std::chrono::high_resolution_clock::now();

	LOG(LogInfo) << "Starting scan of all game sources...";

	// Get all authenticated plugins
	auto plugins = PluginManager::getInstance().getAuthenticatedPlugins();

	if (plugins.empty())
	{
		LOG(LogWarning) << "No authenticated plugins available for scanning";
		mLastError = "No plugins available";
		return stats;
	}

	// Scan each plugin
	for (auto& plugin : plugins)
	{
		LOG(LogInfo) << "Scanning plugin: " << plugin->getName();

		try
		{
			// Fetch games from plugin
			std::vector<GameMetadata> games = plugin->fetchGames();
			stats.totalGamesFound += games.size();

			LOG(LogInfo) << "Found " << games.size() << " games from " << plugin->getName();

			// Process each game
			int currentGame = 0;
			for (auto& game : games)
			{
				currentGame++;

				// Call progress callback
				if (progressCallback)
				{
					progressCallback(currentGame, games.size(), game.name);
				}

				// Add or update in database
				if (!addOrUpdateGame(game, stats))
				{
					stats.errorsEncountered++;
				}
			}
		}
		catch (const std::exception& e)
		{
			LOG(LogError) << "Error scanning plugin " << plugin->getName() << ": " << e.what();
			mLastError = e.what();
			stats.errorsEncountered++;
		}
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	stats.scanDurationSeconds = std::chrono::duration<double>(endTime - startTime).count();

	LOG(LogInfo) << "Scan complete: " << stats.newGamesAdded << " new games added, "
	             << stats.duplicatesSkipped << " duplicates skipped, "
	             << stats.errorsEncountered << " errors in "
	             << stats.scanDurationSeconds << " seconds";

	return stats;
}

ScanStats GameScanner::scanFromPlugin(const std::string& pluginId,
	ScanProgressCallback progressCallback)
{
	ScanStats stats;
	auto startTime = std::chrono::high_resolution_clock::now();

	LOG(LogInfo) << "Scanning plugin: " << pluginId;

	auto plugin = PluginManager::getInstance().getPlugin(pluginId);
	if (!plugin)
	{
		LOG(LogError) << "Plugin not found: " << pluginId;
		mLastError = "Plugin not found";
		return stats;
	}

	if (plugin->requiresAuthentication() && !plugin->isAuthenticated())
	{
		LOG(LogError) << "Plugin requires authentication: " << pluginId;
		mLastError = "Plugin not authenticated";
		return stats;
	}

	try
	{
		// Fetch games
		std::vector<GameMetadata> games = plugin->fetchGames();
		stats.totalGamesFound = games.size();

		LOG(LogInfo) << "Found " << games.size() << " games from " << plugin->getName();

		// Process each game
		int currentGame = 0;
		for (auto& game : games)
		{
			currentGame++;

			if (progressCallback)
			{
				progressCallback(currentGame, games.size(), game.name);
			}

			if (!addOrUpdateGame(game, stats))
			{
				stats.errorsEncountered++;
			}
		}
	}
	catch (const std::exception& e)
	{
		LOG(LogError) << "Error scanning plugin: " << e.what();
		mLastError = e.what();
		stats.errorsEncountered++;
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	stats.scanDurationSeconds = std::chrono::duration<double>(endTime - startTime).count();

	LOG(LogInfo) << "Plugin scan complete: " << stats.newGamesAdded << " new games added";

	return stats;
}

ScanStats GameScanner::scanDirectory(const std::string& path,
	ScanProgressCallback progressCallback)
{
	ScanStats stats;
	auto startTime = std::chrono::high_resolution_clock::now();

	LOG(LogInfo) << "Scanning directory: " << path;

	// Get local plugin
	auto plugin = PluginManager::getInstance().getPlugin("local");
	if (!plugin)
	{
		LOG(LogError) << "Local filesystem plugin not found";
		mLastError = "Local plugin not available";
		return stats;
	}

	// Downcast to LocalFilesystemPlugin to use scanDirectory method
	// For now, we'll use fetchGames and temporarily add the path
	try
	{
		// This is a simplified approach - in production you'd want to properly
		// handle the plugin interface or add scanDirectory to the base interface
		std::vector<GameMetadata> games = plugin->fetchGames();
		stats.totalGamesFound = games.size();

		int currentGame = 0;
		for (auto& game : games)
		{
			currentGame++;

			if (progressCallback)
			{
				progressCallback(currentGame, games.size(), game.name);
			}

			if (!addOrUpdateGame(game, stats))
			{
				stats.errorsEncountered++;
			}
		}
	}
	catch (const std::exception& e)
	{
		LOG(LogError) << "Error scanning directory: " << e.what();
		mLastError = e.what();
		stats.errorsEncountered++;
	}

	auto endTime = std::chrono::high_resolution_clock::now();
	stats.scanDurationSeconds = std::chrono::duration<double>(endTime - startTime).count();

	return stats;
}

ScanStats GameScanner::refreshDatabase(ScanProgressCallback progressCallback)
{
	LOG(LogInfo) << "Refreshing game database...";

	// Set to update existing games
	bool oldUpdateSetting = mUpdateExisting;
	mUpdateExisting = true;

	// Scan all sources
	ScanStats stats = scanAllSources(progressCallback);

	// Restore setting
	mUpdateExisting = oldUpdateSetting;

	// Clean up missing games
	int cleaned = cleanupMissingGames();
	LOG(LogInfo) << "Removed " << cleaned << " missing games from database";

	return stats;
}

int GameScanner::cleanupMissingGames()
{
	LOG(LogInfo) << "Cleaning up missing games...";

	int removedCount = 0;
	auto& db = DatabaseManager::getInstance();

	if (!db.isInitialized())
	{
		LOG(LogWarning) << "Database not initialized";
		return 0;
	}

	// Get all local games
	std::vector<GameMetadata> localGames = db.getGamesBySource(GameSource::LOCAL);

	for (const auto& game : localGames)
	{
		if (!verifyGameExists(game))
		{
			LOG(LogInfo) << "Removing missing game: " << game.name;

			if (db.deleteGame(game.id))
			{
				removedCount++;
			}
		}
	}

	return removedCount;
}

bool GameScanner::gameExists(const GameMetadata& game)
{
	auto& db = DatabaseManager::getInstance();

	if (!db.isInitialized())
		return false;

	// Check by executable path (most reliable)
	std::vector<GameMetadata> allGames = db.getAllGames();

	for (const auto& existing : allGames)
	{
		if (existing.executablePath == game.executablePath)
			return true;
	}

	return false;
}

bool GameScanner::addOrUpdateGame(const GameMetadata& game, ScanStats& stats)
{
	auto& db = DatabaseManager::getInstance();

	if (!db.isInitialized())
	{
		LOG(LogError) << "Database not initialized";
		return false;
	}

	// Check if game already exists
	bool exists = gameExists(game);

	if (exists)
	{
		if (mSkipDuplicates && !mUpdateExisting)
		{
			LOG(LogDebug) << "Skipping duplicate game: " << game.name;
			stats.duplicatesSkipped++;
			return true;
		}

		if (mUpdateExisting)
		{
			// Update existing game
			LOG(LogInfo) << "Updating game: " << game.name;

			// Note: We need to get the existing game ID first
			// This is a simplified version - in production you'd fetch the existing record
			GameMetadata updateGame = game;
			// updateGame.id = existing.id; // Would need to fetch existing first

			if (db.updateGame(updateGame))
			{
				stats.newGamesAdded++; // Could track updates separately
				return true;
			}
			else
			{
				LOG(LogError) << "Failed to update game: " << game.name;
				return false;
			}
		}
		else
		{
			stats.duplicatesSkipped++;
			return true;
		}
	}
	else
	{
		// Add new game
		LOG(LogInfo) << "Adding new game: " << game.name;

		int gameId = db.insertGame(game);
		if (gameId > 0)
		{
			stats.newGamesAdded++;

			// If it's from itch.io, add itch-specific data
			if (game.source == GameSource::ITCH_IO && game.itchId > 0)
			{
				db.insertItchGameData(gameId, game.itchId, game.uploadId,
					game.downloadUrl, game.version);
			}

			return true;
		}
		else
		{
			LOG(LogError) << "Failed to insert game: " << game.name;
			return false;
		}
	}
}

bool GameScanner::verifyGameExists(const GameMetadata& game)
{
	// Check if executable path still exists
	if (game.executablePath.empty())
		return false;

	return Utils::FileSystem::exists(game.executablePath);
}

} // namespace OpenConsole
