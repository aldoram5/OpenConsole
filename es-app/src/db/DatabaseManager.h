#pragma once
#ifndef ES_APP_DB_DATABASE_MANAGER_H
#define ES_APP_DB_DATABASE_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <sqlite3.h>

namespace OpenConsole
{
	// Game type enumeration
	enum class GameType
	{
		APPIMAGE,
		RENPY,
		DEB,
		ELECTRON,
		DIRECTORY,
		UNKNOWN
	};

	// Game source enumeration
	enum class GameSource
	{
		LOCAL,
		ITCH_IO,
		USB,
		UNKNOWN
	};

	// Game metadata structure
	struct GameMetadata
	{
		int id;
		std::string name;
		std::string description;
		std::string coverArtPath;
		std::string executablePath;
		GameType gameType;
		GameSource source;
		std::string installPath;
		long long lastPlayed;     // Unix timestamp
		int playCount;
		long long createdAt;      // Unix timestamp

		// itch.io specific fields (optional)
		int itchId;
		int uploadId;
		std::string downloadUrl;
		std::string version;

		GameMetadata() : id(-1), gameType(GameType::UNKNOWN),
			source(GameSource::UNKNOWN), lastPlayed(0), playCount(0),
			createdAt(0), itchId(-1), uploadId(-1) {}
	};

	// Database manager class (Singleton)
	class DatabaseManager
	{
	public:
		// Get singleton instance
		static DatabaseManager& getInstance();

		// Delete copy constructor and assignment operator
		DatabaseManager(const DatabaseManager&) = delete;
		DatabaseManager& operator=(const DatabaseManager&) = delete;

		// Initialize database connection
		bool init(const std::string& dbPath);

		// Close database connection
		void close();

		// Check if database is initialized
		bool isInitialized() const { return mDb != nullptr; }

		// Game operations
		int insertGame(const GameMetadata& game);
		bool updateGame(const GameMetadata& game);
		bool deleteGame(int gameId);
		GameMetadata getGame(int gameId);
		std::vector<GameMetadata> getAllGames();
		std::vector<GameMetadata> getGamesBySource(GameSource source);
		std::vector<GameMetadata> getGamesByType(GameType gameType);
		std::vector<GameMetadata> getRecentlyPlayed(int limit = 10);

		// Update play statistics
		bool updatePlayStats(int gameId);

		// itch.io specific operations
		bool insertItchGameData(int gameId, int itchId, int uploadId,
			const std::string& downloadUrl, const std::string& version);
		bool updateItchGameData(int gameId, const std::string& downloadUrl,
			const std::string& version);
		GameMetadata getGameByItchId(int itchId);

		// Utility functions
		std::string gameTypeToString(GameType type);
		GameType stringToGameType(const std::string& str);
		std::string gameSourceToString(GameSource source);
		GameSource stringToGameSource(const std::string& str);

	private:
		DatabaseManager();
		~DatabaseManager();

		// Create database schema
		bool createSchema();

		// Prepare common SQL statements
		bool prepareStatements();

		// Helper to execute SQL without results
		bool executeSql(const std::string& sql);

		// Helper to get current Unix timestamp
		long long getCurrentTimestamp();

		sqlite3* mDb;
		bool mInitialized;

		// Prepared statements for performance
		sqlite3_stmt* mInsertGameStmt;
		sqlite3_stmt* mUpdateGameStmt;
		sqlite3_stmt* mGetGameStmt;
		sqlite3_stmt* mGetAllGamesStmt;
	};

} // namespace OpenConsole

#endif // ES_APP_DB_DATABASE_MANAGER_H
