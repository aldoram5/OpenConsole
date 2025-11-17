#pragma once
#ifndef ES_APP_GAME_SCANNER_H
#define ES_APP_GAME_SCANNER_H

#include "db/DatabaseManager.h"
#include "plugins/PluginManager.h"
#include <functional>
#include <string>
#include <vector>

namespace OpenConsole
{
	// Progress callback for scanning
	// Parameters: current game number, total games, game name
	using ScanProgressCallback = std::function<void(int, int, const std::string&)>;

	// Scan statistics
	struct ScanStats
	{
		int totalGamesFound;
		int newGamesAdded;
		int duplicatesSkipped;
		int errorsEncountered;
		double scanDurationSeconds;

		ScanStats() : totalGamesFound(0), newGamesAdded(0),
			duplicatesSkipped(0), errorsEncountered(0),
			scanDurationSeconds(0.0) {}
	};

	// Game scanner - coordinates plugin scanning and database population
	class GameScanner
	{
	public:
		GameScanner();
		~GameScanner();

		// Scan all games from all authenticated plugins
		ScanStats scanAllSources(ScanProgressCallback progressCallback = nullptr);

		// Scan games from specific plugin
		ScanStats scanFromPlugin(const std::string& pluginId,
			ScanProgressCallback progressCallback = nullptr);

		// Scan specific directory (using local plugin)
		ScanStats scanDirectory(const std::string& path,
			ScanProgressCallback progressCallback = nullptr);

		// Refresh database - rescan all and update existing entries
		ScanStats refreshDatabase(ScanProgressCallback progressCallback = nullptr);

		// Remove games that no longer exist on filesystem
		int cleanupMissingGames();

		// Set whether to skip duplicate games (by executable path)
		void setSkipDuplicates(bool skip) { mSkipDuplicates = skip; }

		// Set whether to update existing games
		void setUpdateExisting(bool update) { mUpdateExisting = update; }

		// Get last error
		std::string getLastError() const { return mLastError; }

	private:
		// Check if game already exists in database
		bool gameExists(const GameMetadata& game);

		// Add or update game in database
		bool addOrUpdateGame(const GameMetadata& game, ScanStats& stats);

		// Verify game file still exists
		bool verifyGameExists(const GameMetadata& game);

		bool mSkipDuplicates;
		bool mUpdateExisting;
		std::string mLastError;
	};

} // namespace OpenConsole

#endif // ES_APP_GAME_SCANNER_H
