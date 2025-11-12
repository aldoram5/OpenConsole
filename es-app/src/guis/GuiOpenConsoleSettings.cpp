#include "guis/GuiOpenConsoleSettings.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiVirtualKeyboard.h"
#include "views/ViewController.h"
#include "SystemData.h"
#include "GameScanner.h"
#include "db/DatabaseManager.h"
#include "plugins/PluginManager.h"
#include "plugins/LocalFilesystemPlugin.h"
#include "components/SwitchComponent.h"
#include "components/SliderComponent.h"
#include "Window.h"
#include "Log.h"

using namespace OpenConsole;

GuiOpenConsoleSettings::GuiOpenConsoleSettings(Window* window)
	: GuiSettings(window, "OPENCONSOLE SETTINGS")
{
	// Database section
	addEntry("DATABASE", 0x777777FF, false);
	addDatabaseSettings();

	// Plugin section
	addEntry("PLUGINS", 0x777777FF, false);
	addPluginSettings();

	// Scanning section
	addEntry("SCANNING", 0x777777FF, false);
	addScanningSettings();

	// Maintenance section
	addEntry("MAINTENANCE", 0x777777FF, false);
	addMaintenanceSettings();
}

GuiOpenConsoleSettings::~GuiOpenConsoleSettings()
{
}

void GuiOpenConsoleSettings::addDatabaseSettings()
{
	// Database path (read-only)
	auto& db = DatabaseManager::getInstance();
	std::string dbPath = db.isInitialized() ? "~/.openconsole/games.db" : "Not initialized";

	addWithLabel("Database Location", std::make_shared<TextComponent>(mWindow,
		dbPath, Font::get(FONT_SIZE_SMALL), 0x777777FF));

	// Database stats
	addWithLabel("View Database Statistics",
		std::make_shared<TextComponent>(mWindow, ">", Font::get(FONT_SIZE_SMALL), 0x777777FF));
	mMenu.getCursorRow()->makeAcceptInputHandler([this] {
		viewDatabaseStats();
	});
}

void GuiOpenConsoleSettings::addPluginSettings()
{
	auto& pm = PluginManager::getInstance();

	// Plugin count
	std::string pluginCount = std::to_string(pm.getPluginCount()) + " plugins loaded";
	addWithLabel("Plugins", std::make_shared<TextComponent>(mWindow,
		pluginCount, Font::get(FONT_SIZE_SMALL), 0x777777FF));

	// Local filesystem plugin settings
	auto localPlugin = std::dynamic_pointer_cast<LocalFilesystemPlugin>(pm.getPlugin("local"));
	if (localPlugin)
	{
		// Recursive scan toggle
		auto recursiveScan = std::make_shared<SwitchComponent>(mWindow);
		recursiveScan->setState(localPlugin->isRecursiveScan());
		addWithLabel("Recursive Scanning", recursiveScan);
		addSaveFunc([localPlugin, recursiveScan] {
			localPlugin->setRecursiveScan(recursiveScan->getState());
		});

		// Max scan depth
		addWithLabel("Configure Scan Depth",
			std::make_shared<TextComponent>(mWindow, ">", Font::get(FONT_SIZE_SMALL), 0x777777FF));
		mMenu.getCursorRow()->makeAcceptInputHandler([this] {
			configureScanDepth();
		});

		// Manage scan paths
		addWithLabel("Manage Scan Paths",
			std::make_shared<TextComponent>(mWindow, ">", Font::get(FONT_SIZE_SMALL), 0x777777FF));
		mMenu.getCursorRow()->makeAcceptInputHandler([this] {
			addScanPath();
		});
	}
}

void GuiOpenConsoleSettings::addScanningSettings()
{
	// Scan for games now
	addWithLabel("Scan for Games Now",
		std::make_shared<TextComponent>(mWindow, "START SCAN", Font::get(FONT_SIZE_SMALL), 0x00FF00FF));
	mMenu.getCursorRow()->makeAcceptInputHandler([this] {
		scanGames();
	});

	// Auto-scan on startup (future feature)
	auto autoScan = std::make_shared<SwitchComponent>(mWindow);
	autoScan->setState(false);  // Default off for now
	addWithLabel("Auto-scan on Startup", autoScan);
	// TODO: Save this setting
}

void GuiOpenConsoleSettings::addMaintenanceSettings()
{
	// Refresh database
	addWithLabel("Refresh Database",
		std::make_shared<TextComponent>(mWindow, "REFRESH", Font::get(FONT_SIZE_SMALL), 0xFFFF00FF));
	mMenu.getCursorRow()->makeAcceptInputHandler([this] {
		refreshDatabase();
	});

	// Clear database
	addWithLabel("Clear Database",
		std::make_shared<TextComponent>(mWindow, "CLEAR", Font::get(FONT_SIZE_SMALL), 0xFF0000FF));
	mMenu.getCursorRow()->makeAcceptInputHandler([this] {
		clearDatabase();
	});
}

void GuiOpenConsoleSettings::scanGames()
{
	// Show progress message
	auto msgBox = new GuiMsgBox(mWindow, "SCANNING FOR GAMES...\n\nThis may take a few moments.",
		"CANCEL", nullptr);
	mWindow->pushGui(msgBox);

	// Perform scan
	GameScanner scanner;
	ScanStats stats = scanner.scanAllSources([this](int current, int total, const std::string& name) {
		// Progress callback - could update UI here
		LOG(LogInfo) << "Scanning " << current << "/" << total << ": " << name;
	});

	// Close progress message
	delete msgBox;

	// Show results
	std::stringstream ss;
	ss << "SCAN COMPLETE!\n\n";
	ss << "Games found: " << stats.totalGamesFound << "\n";
	ss << "New games added: " << stats.newGamesAdded << "\n";
	ss << "Duplicates skipped: " << stats.duplicatesSkipped << "\n";
	ss << "Errors: " << stats.errorsEncountered << "\n";
	ss << "Time: " << stats.scanDurationSeconds << "s";

	mWindow->pushGui(new GuiMsgBox(mWindow, ss.str(), "OK", nullptr));

	LOG(LogInfo) << "Game scan completed: " << stats.newGamesAdded << " new games";
}

void GuiOpenConsoleSettings::refreshDatabase()
{
	mWindow->pushGui(new GuiMsgBox(mWindow,
		"This will rescan all games and update the database.\n\nMissing games will be removed.\n\nContinue?",
		"YES", [this] {
			GameScanner scanner;
			ScanStats stats = scanner.refreshDatabase();

			std::stringstream ss;
			ss << "DATABASE REFRESHED!\n\n";
			ss << "Games updated: " << stats.newGamesAdded << "\n";
			ss << "Time: " << stats.scanDurationSeconds << "s";

			mWindow->pushGui(new GuiMsgBox(mWindow, ss.str(), "OK", nullptr));
		},
		"NO", nullptr));
}

void GuiOpenConsoleSettings::clearDatabase()
{
	mWindow->pushGui(new GuiMsgBox(mWindow,
		"WARNING: This will delete ALL games from the database!\n\nThis cannot be undone.\n\nAre you sure?",
		"YES, DELETE ALL", [this] {
			auto& db = DatabaseManager::getInstance();

			// Get all games and delete them
			auto games = db.getAllGames();
			int deleted = 0;

			for (const auto& game : games)
			{
				if (db.deleteGame(game.id))
					deleted++;
			}

			std::stringstream ss;
			ss << "Deleted " << deleted << " games from database.";

			mWindow->pushGui(new GuiMsgBox(mWindow, ss.str(), "OK", nullptr));
			LOG(LogInfo) << "Cleared database: " << deleted << " games deleted";
		},
		"NO, CANCEL", nullptr));
}

void GuiOpenConsoleSettings::viewDatabaseStats()
{
	auto& db = DatabaseManager::getInstance();

	if (!db.isInitialized())
	{
		mWindow->pushGui(new GuiMsgBox(mWindow, "Database not initialized!", "OK", nullptr));
		return;
	}

	// Get stats
	auto allGames = db.getAllGames();
	auto localGames = db.getGamesBySource(GameSource::LOCAL);
	auto recentGames = db.getRecentlyPlayed(10);

	std::stringstream ss;
	ss << "DATABASE STATISTICS\n\n";
	ss << "Total games: " << allGames.size() << "\n";
	ss << "Local games: " << localGames.size() << "\n";
	ss << "Recently played: " << recentGames.size() << "\n\n";

	// Count by type
	int appImageCount = 0, renpyCount = 0, debCount = 0, otherCount = 0;
	for (const auto& game : allGames)
	{
		switch (game.gameType)
		{
			case GameType::APPIMAGE: appImageCount++; break;
			case GameType::RENPY: renpyCount++; break;
			case GameType::DEB: debCount++; break;
			default: otherCount++; break;
		}
	}

	ss << "By Type:\n";
	ss << "  AppImage: " << appImageCount << "\n";
	ss << "  Ren'Py: " << renpyCount << "\n";
	ss << "  .deb: " << debCount << "\n";
	ss << "  Other: " << otherCount;

	mWindow->pushGui(new GuiMsgBox(mWindow, ss.str(), "OK", nullptr));
}

void GuiOpenConsoleSettings::addScanPath()
{
	// Show virtual keyboard to enter path
	mWindow->pushGui(new GuiVirtualKeyboard(mWindow, "ADD SCAN PATH", "",
		[this](const std::string& path) {
			if (path.empty())
				return;

			auto& pm = PluginManager::getInstance();
			auto localPlugin = std::dynamic_pointer_cast<LocalFilesystemPlugin>(pm.getPlugin("local"));

			if (localPlugin)
			{
				localPlugin->addScanPath(path);
				LOG(LogInfo) << "Added scan path: " << path;

				mWindow->pushGui(new GuiMsgBox(mWindow,
					"Scan path added:\n" + path, "OK", nullptr));
			}
		}, false, "ADD"));
}

void GuiOpenConsoleSettings::removeScanPath()
{
	// TODO: Show list of scan paths to remove
	mWindow->pushGui(new GuiMsgBox(mWindow,
		"This feature is not yet implemented.", "OK", nullptr));
}

void GuiOpenConsoleSettings::configureScanDepth()
{
	auto& pm = PluginManager::getInstance();
	auto localPlugin = std::dynamic_pointer_cast<LocalFilesystemPlugin>(pm.getPlugin("local"));

	if (!localPlugin)
		return;

	int currentDepth = localPlugin->getMaxScanDepth();

	// Create a simple dialog with depth options
	std::stringstream ss;
	ss << "Current max scan depth: " << currentDepth << "\n\n";
	ss << "Select new max depth:\n";
	ss << "0 = Unlimited\n";
	ss << "1-10 = Limited depth";

	// For now, just show current value
	// TODO: Implement depth selector
	mWindow->pushGui(new GuiMsgBox(mWindow, ss.str(), "OK", nullptr));
}
