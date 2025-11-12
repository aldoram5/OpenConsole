#include "OpenConsoleSystem.h"
#include "db/DatabaseManager.h"
#include "plugins/PluginManager.h"
#include "launchers/GameLauncherFactory.h"
#include "FileData.h"
#include "Log.h"
#include "Settings.h"
#include "utils/FileSystemUtil.h"

using namespace OpenConsole;

bool OpenConsoleSystem::sInitialized = false;
SystemData* OpenConsoleSystem::sOpenConsoleSystem = nullptr;

bool OpenConsoleSystem::init()
{
	if (sInitialized)
	{
		LOG(LogWarning) << "OpenConsoleSystem already initialized";
		return true;
	}

	LOG(LogInfo) << "Initializing OpenConsoleSystem...";

	// Initialize database
	std::string dbPath = Utils::FileSystem::getHomePath() + "/.openconsole/games.db";
	Utils::FileSystem::createDirectory(Utils::FileSystem::getParent(dbPath));

	DatabaseManager& db = DatabaseManager::getInstance();
	if (!db.init(dbPath))
	{
		LOG(LogError) << "Failed to initialize database";
		return false;
	}

	// Initialize plugin manager
	PluginManager& pm = PluginManager::getInstance();
	if (!pm.initialize())
	{
		LOG(LogError) << "Failed to initialize plugin manager";
		return false;
	}

	sInitialized = true;
	LOG(LogInfo) << "OpenConsoleSystem initialized successfully";
	return true;
}

void OpenConsoleSystem::deinit()
{
	if (!sInitialized)
		return;

	LOG(LogInfo) << "Shutting down OpenConsoleSystem";

	// Shutdown plugin manager
	PluginManager::getInstance().shutdown();

	// Close database
	DatabaseManager::getInstance().close();

	sInitialized = false;
}

SystemData* OpenConsoleSystem::createSystem()
{
	if (!sInitialized)
	{
		LOG(LogError) << "OpenConsoleSystem not initialized";
		return nullptr;
	}

	// Create OpenConsole system
	SystemEnvironmentData envData;
	envData.mStartPath = Utils::FileSystem::getHomePath() + "/Games";
	envData.mSearchExtensions.push_back(".appimage");
	envData.mSearchExtensions.push_back(".sh");
	envData.mSearchExtensions.push_back(".deb");
	envData.mLaunchCommand = "";  // We use our launcher system instead
	envData.mPlatformIds.push_back(PlatformIds::PLATFORM_LINUX);

	sOpenConsoleSystem = new SystemData("openconsole", "OpenConsole Games",
		envData, "openconsole", nullptr);

	// Populate from database
	populateGameListFromDatabase(sOpenConsoleSystem);

	LOG(LogInfo) << "OpenConsole system created with " << sOpenConsoleSystem->getRootFolder()->getChildrenListToDisplay().size() << " games";

	return sOpenConsoleSystem;
}

void OpenConsoleSystem::refreshGameList()
{
	if (!sOpenConsoleSystem)
		return;

	LOG(LogInfo) << "Refreshing OpenConsole game list from database";

	// Clear existing games
	sOpenConsoleSystem->getRootFolder()->clear();

	// Repopulate from database
	populateGameListFromDatabase(sOpenConsoleSystem);
}

void OpenConsoleSystem::populateGameListFromDatabase(SystemData* system)
{
	DatabaseManager& db = DatabaseManager::getInstance();

	if (!db.isInitialized())
	{
		LOG(LogWarning) << "Database not initialized, no games to load";
		return;
	}

	// Get all games from database
	std::vector<GameMetadata> games = db.getAllGames();

	LOG(LogInfo) << "Loading " << games.size() << " games from database";

	// Create FileData for each game
	for (const auto& game : games)
	{
		FileData* fileData = createFileDataFromGame(system, game);
		if (fileData)
		{
			system->getRootFolder()->addChild(fileData);
		}
	}

	// Index games for better performance
	system->getRootFolder()->sort(FileSorts::SortTypes.at(0).comparisonFunction, false);
}

FileData* OpenConsoleSystem::createFileDataFromGame(SystemData* system, const GameMetadata& game)
{
	// Verify game file exists
	if (!Utils::FileSystem::exists(game.executablePath))
	{
		LOG(LogWarning) << "Game file not found: " << game.executablePath;
		return nullptr;
	}

	// Create FileData
	FileData* fileData = new FileData(GAME, game.executablePath, system->getSystemEnvData(), system);

	// Set metadata
	MetaDataList& metadata = fileData->metadata;

	metadata.set("name", game.name);

	if (!game.description.empty())
		metadata.set("desc", game.description);

	if (!game.coverArtPath.empty() && Utils::FileSystem::exists(game.coverArtPath))
		metadata.set("image", game.coverArtPath);

	// Set play stats
	if (game.playCount > 0)
		metadata.set("playcount", std::to_string(game.playCount));

	if (game.lastPlayed > 0)
	{
		// Convert Unix timestamp to string
		std::time_t t = static_cast<std::time_t>(game.lastPlayed);
		std::tm* tm = std::localtime(&t);
		char buffer[32];
		std::strftime(buffer, sizeof(buffer), "%Y%m%dT%H%M%S", tm);
		metadata.set("lastplayed", std::string(buffer));
	}

	// Store game type and source as metadata (custom fields)
	metadata.set("source", DatabaseManager::gameSourceToString(game.source));
	metadata.set("gametype", DatabaseManager::gameTypeToString(game.gameType));

	// Store database ID for easy lookup
	metadata.set("dbid", std::to_string(game.id));

	return fileData;
}
