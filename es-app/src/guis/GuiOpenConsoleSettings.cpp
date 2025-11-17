#include "guis/GuiOpenConsoleSettings.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiVirtualKeyboard.h"
#include "guis/GuiItchIoAuth.h"
#include "views/ViewController.h"
#include "SystemData.h"
#include "db/DatabaseManager.h"
#include "plugins/PluginManager.h"
#include "plugins/LocalFilesystemPlugin.h"
#include "plugins/ItchIoPlugin.h"
#include "components/SwitchComponent.h"
#include "components/ButtonComponent.h"
#include "Window.h"
#include "Log.h"

using namespace OpenConsole;

GuiOpenConsoleSettings::GuiOpenConsoleSettings(Window* window)
	: GuiSettings(window, "OPENCONSOLE SETTINGS")
{
	// Database section header
	addWithLabel("--- DATABASE ---", std::make_shared<TextComponent>(mWindow,
		"", Font::get(FONT_SIZE_SMALL), 0x777777FF));

	addDatabaseSettings();

	// Plugin section header
	addWithLabel("--- PLUGINS ---", std::make_shared<TextComponent>(mWindow,
		"", Font::get(FONT_SIZE_SMALL), 0x777777FF));

	addPluginSettings();

	// Scanning section header
	addWithLabel("--- SCANNING ---", std::make_shared<TextComponent>(mWindow,
		"", Font::get(FONT_SIZE_SMALL), 0x777777FF));

	addScanningSettings();

	// Maintenance section header
	addWithLabel("--- MAINTENANCE ---", std::make_shared<TextComponent>(mWindow,
		"", Font::get(FONT_SIZE_SMALL), 0x777777FF));

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

	// Database stats - clickable row
	ComponentListRow statsRow;
	statsRow.addElement(std::make_shared<TextComponent>(mWindow, "VIEW DATABASE STATISTICS",
		Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	statsRow.makeAcceptInputHandler([this] { viewDatabaseStats(); });
	addRow(statsRow);
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

		// Add scan path - clickable row
		ComponentListRow addPathRow;
		addPathRow.addElement(std::make_shared<TextComponent>(mWindow, "ADD SCAN PATH",
			Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		addPathRow.makeAcceptInputHandler([this] { addScanPath(); });
		addRow(addPathRow);
	}

	// itch.io plugin settings
	auto itchPlugin = std::dynamic_pointer_cast<ItchIoPlugin>(pm.getPlugin("itch_io"));
	if (itchPlugin)
	{
		// Authentication status
		std::string authStatus = itchPlugin->isAuthenticated() ? "Authenticated" : "Not authenticated";
		unsigned int statusColor = itchPlugin->isAuthenticated() ? 0x00FF00FF : 0xFF0000FF;

		addWithLabel("itch.io Status",
			std::make_shared<TextComponent>(mWindow, authStatus, Font::get(FONT_SIZE_SMALL), statusColor));

		// Authenticate button - clickable row
		ComponentListRow authRow;
		authRow.addElement(std::make_shared<TextComponent>(mWindow, "CONFIGURE ITCH.IO",
			Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
		authRow.makeAcceptInputHandler([this] { openItchIoAuth(); });
		addRow(authRow);
	}
}

void GuiOpenConsoleSettings::addScanningSettings()
{
	// Scan for games now - clickable row
	ComponentListRow scanRow;
	scanRow.addElement(std::make_shared<TextComponent>(mWindow, "SCAN FOR GAMES NOW",
		Font::get(FONT_SIZE_MEDIUM), 0x00FF00FF), true);
	scanRow.makeAcceptInputHandler([this] { scanGames(); });
	addRow(scanRow);

	// Auto-scan on startup (future feature)
	auto autoScan = std::make_shared<SwitchComponent>(mWindow);
	autoScan->setState(false);  // Default off for now
	addWithLabel("Auto-scan on Startup", autoScan);
	// TODO: Save this setting
}

void GuiOpenConsoleSettings::addMaintenanceSettings()
{
	// Refresh database - clickable row
	ComponentListRow refreshRow;
	refreshRow.addElement(std::make_shared<TextComponent>(mWindow, "REFRESH DATABASE",
		Font::get(FONT_SIZE_MEDIUM), 0xFFFF00FF), true);
	refreshRow.makeAcceptInputHandler([this] { refreshDatabase(); });
	addRow(refreshRow);

	// Clear database - clickable row
	ComponentListRow clearRow;
	clearRow.addElement(std::make_shared<TextComponent>(mWindow, "CLEAR DATABASE",
		Font::get(FONT_SIZE_MEDIUM), 0xFF0000FF), true);
	clearRow.makeAcceptInputHandler([this] { clearDatabase(); });
	addRow(clearRow);
}

void GuiOpenConsoleSettings::scanGames()
{
	mWindow->pushGui(new GuiMsgBox(mWindow, "Game scanning is not yet fully implemented.\n\nThis will be available in a future update.",
		"OK", nullptr));

	LOG(LogInfo) << "Game scan requested (not yet implemented)";
}

void GuiOpenConsoleSettings::refreshDatabase()
{
	mWindow->pushGui(new GuiMsgBox(mWindow,
		"This will rescan all games and update the database.\n\nMissing games will be removed.\n\nContinue?",
		"YES", [this] {
			mWindow->pushGui(new GuiMsgBox(mWindow, "Database refresh is not yet fully implemented.", "OK", nullptr));
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

void GuiOpenConsoleSettings::openItchIoAuth()
{
	// Open itch.io authentication dialog
	auto authDialog = new GuiItchIoAuth(mWindow, [this](bool success, const std::string& username) {
		if (success)
		{
			LOG(LogInfo) << "itch.io authentication successful for user: " << username;

			// Show success message
			mWindow->pushGui(new GuiMsgBox(mWindow,
				"Successfully authenticated with itch.io!\n\nYou can now scan for your itch.io games.",
				"OK", nullptr));
		}
		else
		{
			LOG(LogInfo) << "itch.io authentication cancelled or failed";
		}
	});

	mWindow->pushGui(authDialog);
}
