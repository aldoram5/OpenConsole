#pragma once
#ifndef ES_APP_OPENCONSOLE_SYSTEM_H
#define ES_APP_OPENCONSOLE_SYSTEM_H

#include "SystemData.h"
#include "db/DatabaseManager.h"
#include <memory>

// OpenConsole game system
// Provides games from the database instead of XML files
class OpenConsoleSystem
{
public:
	// Initialize OpenConsole systems
	static bool init();

	// Shutdown OpenConsole systems
	static void deinit();

	// Refresh game list from database
	static void refreshGameList();

	// Create OpenConsole system data
	static SystemData* createSystem();

	// Check if OpenConsole is initialized
	static bool isInitialized() { return sInitialized; }

private:
	// Create FileData tree from database
	static void populateGameListFromDatabase(SystemData* system);

	// Convert GameMetadata to FileData
	static FileData* createFileDataFromGame(SystemData* system, const OpenConsole::GameMetadata& game);

	static bool sInitialized;
	static SystemData* sOpenConsoleSystem;
};

#endif // ES_APP_OPENCONSOLE_SYSTEM_H
