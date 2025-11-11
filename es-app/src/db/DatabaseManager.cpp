#include "db/DatabaseManager.h"
#include "Log.h"
#include <chrono>
#include <sstream>

namespace OpenConsole
{

DatabaseManager::DatabaseManager()
	: mDb(nullptr), mInitialized(false),
	  mInsertGameStmt(nullptr), mUpdateGameStmt(nullptr),
	  mGetGameStmt(nullptr), mGetAllGamesStmt(nullptr)
{
}

DatabaseManager::~DatabaseManager()
{
	close();
}

DatabaseManager& DatabaseManager::getInstance()
{
	static DatabaseManager instance;
	return instance;
}

bool DatabaseManager::init(const std::string& dbPath)
{
	if (mInitialized)
	{
		LOG(LogWarning) << "DatabaseManager already initialized";
		return true;
	}

	// Open database connection
	int rc = sqlite3_open(dbPath.c_str(), &mDb);
	if (rc != SQLITE_OK)
	{
		LOG(LogError) << "Failed to open database: " << sqlite3_errmsg(mDb);
		sqlite3_close(mDb);
		mDb = nullptr;
		return false;
	}

	LOG(LogInfo) << "Database opened successfully: " << dbPath;

	// Enable foreign keys
	if (!executeSql("PRAGMA foreign_keys = ON;"))
	{
		LOG(LogError) << "Failed to enable foreign keys";
		close();
		return false;
	}

	// Create schema if needed
	if (!createSchema())
	{
		LOG(LogError) << "Failed to create database schema";
		close();
		return false;
	}

	// Prepare statements
	if (!prepareStatements())
	{
		LOG(LogError) << "Failed to prepare SQL statements";
		close();
		return false;
	}

	mInitialized = true;
	LOG(LogInfo) << "DatabaseManager initialized successfully";
	return true;
}

void DatabaseManager::close()
{
	// Finalize prepared statements
	if (mInsertGameStmt) sqlite3_finalize(mInsertGameStmt);
	if (mUpdateGameStmt) sqlite3_finalize(mUpdateGameStmt);
	if (mGetGameStmt) sqlite3_finalize(mGetGameStmt);
	if (mGetAllGamesStmt) sqlite3_finalize(mGetAllGamesStmt);

	mInsertGameStmt = nullptr;
	mUpdateGameStmt = nullptr;
	mGetGameStmt = nullptr;
	mGetAllGamesStmt = nullptr;

	// Close database
	if (mDb)
	{
		sqlite3_close(mDb);
		mDb = nullptr;
		LOG(LogInfo) << "Database closed";
	}

	mInitialized = false;
}

bool DatabaseManager::createSchema()
{
	// Create games table
	const char* createGamesTable = R"(
		CREATE TABLE IF NOT EXISTS games (
			id INTEGER PRIMARY KEY AUTOINCREMENT,
			name TEXT NOT NULL,
			description TEXT,
			cover_art_path TEXT,
			executable_path TEXT NOT NULL,
			game_type TEXT NOT NULL,
			source TEXT NOT NULL,
			install_path TEXT,
			last_played INTEGER DEFAULT 0,
			play_count INTEGER DEFAULT 0,
			created_at INTEGER DEFAULT 0
		);
	)";

	if (!executeSql(createGamesTable))
	{
		LOG(LogError) << "Failed to create games table";
		return false;
	}

	// Create itch_games table
	const char* createItchGamesTable = R"(
		CREATE TABLE IF NOT EXISTS itch_games (
			game_id INTEGER PRIMARY KEY,
			itch_id INTEGER NOT NULL,
			upload_id INTEGER,
			download_url TEXT,
			version TEXT,
			FOREIGN KEY (game_id) REFERENCES games(id) ON DELETE CASCADE
		);
	)";

	if (!executeSql(createItchGamesTable))
	{
		LOG(LogError) << "Failed to create itch_games table";
		return false;
	}

	// Create indices for better query performance
	executeSql("CREATE INDEX IF NOT EXISTS idx_games_source ON games(source);");
	executeSql("CREATE INDEX IF NOT EXISTS idx_games_type ON games(game_type);");
	executeSql("CREATE INDEX IF NOT EXISTS idx_games_last_played ON games(last_played DESC);");
	executeSql("CREATE INDEX IF NOT EXISTS idx_itch_games_itch_id ON itch_games(itch_id);");

	LOG(LogInfo) << "Database schema created successfully";
	return true;
}

bool DatabaseManager::prepareStatements()
{
	// Prepare insert game statement
	const char* insertGameSql = R"(
		INSERT INTO games (name, description, cover_art_path, executable_path,
			game_type, source, install_path, last_played, play_count, created_at)
		VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
	)";

	if (sqlite3_prepare_v2(mDb, insertGameSql, -1, &mInsertGameStmt, nullptr) != SQLITE_OK)
	{
		LOG(LogError) << "Failed to prepare insert statement: " << sqlite3_errmsg(mDb);
		return false;
	}

	// Prepare update game statement
	const char* updateGameSql = R"(
		UPDATE games SET name=?, description=?, cover_art_path=?, executable_path=?,
			game_type=?, source=?, install_path=?, last_played=?, play_count=?
		WHERE id=?;
	)";

	if (sqlite3_prepare_v2(mDb, updateGameSql, -1, &mUpdateGameStmt, nullptr) != SQLITE_OK)
	{
		LOG(LogError) << "Failed to prepare update statement: " << sqlite3_errmsg(mDb);
		return false;
	}

	// Prepare get game statement
	const char* getGameSql = "SELECT * FROM games WHERE id=?;";
	if (sqlite3_prepare_v2(mDb, getGameSql, -1, &mGetGameStmt, nullptr) != SQLITE_OK)
	{
		LOG(LogError) << "Failed to prepare get statement: " << sqlite3_errmsg(mDb);
		return false;
	}

	// Prepare get all games statement
	const char* getAllGamesSql = "SELECT * FROM games ORDER BY name ASC;";
	if (sqlite3_prepare_v2(mDb, getAllGamesSql, -1, &mGetAllGamesStmt, nullptr) != SQLITE_OK)
	{
		LOG(LogError) << "Failed to prepare get all statement: " << sqlite3_errmsg(mDb);
		return false;
	}

	return true;
}

bool DatabaseManager::executeSql(const std::string& sql)
{
	char* errMsg = nullptr;
	int rc = sqlite3_exec(mDb, sql.c_str(), nullptr, nullptr, &errMsg);

	if (rc != SQLITE_OK)
	{
		LOG(LogError) << "SQL error: " << errMsg;
		sqlite3_free(errMsg);
		return false;
	}

	return true;
}

long long DatabaseManager::getCurrentTimestamp()
{
	return std::chrono::duration_cast<std::chrono::seconds>(
		std::chrono::system_clock::now().time_since_epoch()).count();
}

int DatabaseManager::insertGame(const GameMetadata& game)
{
	if (!mInitialized || !mInsertGameStmt)
		return -1;

	// Reset statement
	sqlite3_reset(mInsertGameStmt);

	// Bind parameters
	sqlite3_bind_text(mInsertGameStmt, 1, game.name.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mInsertGameStmt, 2, game.description.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mInsertGameStmt, 3, game.coverArtPath.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mInsertGameStmt, 4, game.executablePath.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mInsertGameStmt, 5, gameTypeToString(game.gameType).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mInsertGameStmt, 6, gameSourceToString(game.source).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mInsertGameStmt, 7, game.installPath.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int64(mInsertGameStmt, 8, game.lastPlayed);
	sqlite3_bind_int(mInsertGameStmt, 9, game.playCount);
	sqlite3_bind_int64(mInsertGameStmt, 10, game.createdAt > 0 ? game.createdAt : getCurrentTimestamp());

	// Execute
	int rc = sqlite3_step(mInsertGameStmt);
	if (rc != SQLITE_DONE)
	{
		LOG(LogError) << "Failed to insert game: " << sqlite3_errmsg(mDb);
		return -1;
	}

	int gameId = static_cast<int>(sqlite3_last_insert_rowid(mDb));
	LOG(LogInfo) << "Game inserted successfully with ID: " << gameId;
	return gameId;
}

bool DatabaseManager::updateGame(const GameMetadata& game)
{
	if (!mInitialized || !mUpdateGameStmt || game.id < 0)
		return false;

	// Reset statement
	sqlite3_reset(mUpdateGameStmt);

	// Bind parameters
	sqlite3_bind_text(mUpdateGameStmt, 1, game.name.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mUpdateGameStmt, 2, game.description.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mUpdateGameStmt, 3, game.coverArtPath.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mUpdateGameStmt, 4, game.executablePath.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mUpdateGameStmt, 5, gameTypeToString(game.gameType).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mUpdateGameStmt, 6, gameSourceToString(game.source).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(mUpdateGameStmt, 7, game.installPath.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int64(mUpdateGameStmt, 8, game.lastPlayed);
	sqlite3_bind_int(mUpdateGameStmt, 9, game.playCount);
	sqlite3_bind_int(mUpdateGameStmt, 10, game.id);

	// Execute
	int rc = sqlite3_step(mUpdateGameStmt);
	if (rc != SQLITE_DONE)
	{
		LOG(LogError) << "Failed to update game: " << sqlite3_errmsg(mDb);
		return false;
	}

	return true;
}

bool DatabaseManager::deleteGame(int gameId)
{
	if (!mInitialized)
		return false;

	std::stringstream ss;
	ss << "DELETE FROM games WHERE id=" << gameId << ";";

	return executeSql(ss.str());
}

GameMetadata DatabaseManager::getGame(int gameId)
{
	GameMetadata game;

	if (!mInitialized || !mGetGameStmt)
		return game;

	// Reset and bind
	sqlite3_reset(mGetGameStmt);
	sqlite3_bind_int(mGetGameStmt, 1, gameId);

	// Execute
	if (sqlite3_step(mGetGameStmt) == SQLITE_ROW)
	{
		game.id = sqlite3_column_int(mGetGameStmt, 0);
		game.name = reinterpret_cast<const char*>(sqlite3_column_text(mGetGameStmt, 1));

		const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(mGetGameStmt, 2));
		game.description = desc ? desc : "";

		const char* cover = reinterpret_cast<const char*>(sqlite3_column_text(mGetGameStmt, 3));
		game.coverArtPath = cover ? cover : "";

		game.executablePath = reinterpret_cast<const char*>(sqlite3_column_text(mGetGameStmt, 4));
		game.gameType = stringToGameType(reinterpret_cast<const char*>(sqlite3_column_text(mGetGameStmt, 5)));
		game.source = stringToGameSource(reinterpret_cast<const char*>(sqlite3_column_text(mGetGameStmt, 6)));

		const char* install = reinterpret_cast<const char*>(sqlite3_column_text(mGetGameStmt, 7));
		game.installPath = install ? install : "";

		game.lastPlayed = sqlite3_column_int64(mGetGameStmt, 8);
		game.playCount = sqlite3_column_int(mGetGameStmt, 9);
		game.createdAt = sqlite3_column_int64(mGetGameStmt, 10);
	}

	return game;
}

std::vector<GameMetadata> DatabaseManager::getAllGames()
{
	std::vector<GameMetadata> games;

	if (!mInitialized || !mGetAllGamesStmt)
		return games;

	// Reset statement
	sqlite3_reset(mGetAllGamesStmt);

	// Execute and fetch all rows
	while (sqlite3_step(mGetAllGamesStmt) == SQLITE_ROW)
	{
		GameMetadata game;
		game.id = sqlite3_column_int(mGetAllGamesStmt, 0);
		game.name = reinterpret_cast<const char*>(sqlite3_column_text(mGetAllGamesStmt, 1));

		const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(mGetAllGamesStmt, 2));
		game.description = desc ? desc : "";

		const char* cover = reinterpret_cast<const char*>(sqlite3_column_text(mGetAllGamesStmt, 3));
		game.coverArtPath = cover ? cover : "";

		game.executablePath = reinterpret_cast<const char*>(sqlite3_column_text(mGetAllGamesStmt, 4));
		game.gameType = stringToGameType(reinterpret_cast<const char*>(sqlite3_column_text(mGetAllGamesStmt, 5)));
		game.source = stringToGameSource(reinterpret_cast<const char*>(sqlite3_column_text(mGetAllGamesStmt, 6)));

		const char* install = reinterpret_cast<const char*>(sqlite3_column_text(mGetAllGamesStmt, 7));
		game.installPath = install ? install : "";

		game.lastPlayed = sqlite3_column_int64(mGetAllGamesStmt, 8);
		game.playCount = sqlite3_column_int(mGetAllGamesStmt, 9);
		game.createdAt = sqlite3_column_int64(mGetAllGamesStmt, 10);

		games.push_back(game);
	}

	return games;
}

std::vector<GameMetadata> DatabaseManager::getGamesBySource(GameSource source)
{
	std::vector<GameMetadata> games;

	if (!mInitialized)
		return games;

	std::stringstream ss;
	ss << "SELECT * FROM games WHERE source='" << gameSourceToString(source) << "' ORDER BY name ASC;";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(mDb, ss.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return games;

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		GameMetadata game;
		game.id = sqlite3_column_int(stmt, 0);
		game.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

		const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
		game.description = desc ? desc : "";

		const char* cover = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
		game.coverArtPath = cover ? cover : "";

		game.executablePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
		game.gameType = stringToGameType(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
		game.source = source;

		const char* install = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
		game.installPath = install ? install : "";

		game.lastPlayed = sqlite3_column_int64(stmt, 8);
		game.playCount = sqlite3_column_int(stmt, 9);
		game.createdAt = sqlite3_column_int64(stmt, 10);

		games.push_back(game);
	}

	sqlite3_finalize(stmt);
	return games;
}

std::vector<GameMetadata> DatabaseManager::getGamesByType(GameType gameType)
{
	std::vector<GameMetadata> games;

	if (!mInitialized)
		return games;

	std::stringstream ss;
	ss << "SELECT * FROM games WHERE game_type='" << gameTypeToString(gameType) << "' ORDER BY name ASC;";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(mDb, ss.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return games;

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		GameMetadata game;
		game.id = sqlite3_column_int(stmt, 0);
		game.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

		const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
		game.description = desc ? desc : "";

		const char* cover = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
		game.coverArtPath = cover ? cover : "";

		game.executablePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
		game.gameType = gameType;
		game.source = stringToGameSource(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));

		const char* install = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
		game.installPath = install ? install : "";

		game.lastPlayed = sqlite3_column_int64(stmt, 8);
		game.playCount = sqlite3_column_int(stmt, 9);
		game.createdAt = sqlite3_column_int64(stmt, 10);

		games.push_back(game);
	}

	sqlite3_finalize(stmt);
	return games;
}

std::vector<GameMetadata> DatabaseManager::getRecentlyPlayed(int limit)
{
	std::vector<GameMetadata> games;

	if (!mInitialized)
		return games;

	std::stringstream ss;
	ss << "SELECT * FROM games WHERE last_played > 0 ORDER BY last_played DESC LIMIT " << limit << ";";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(mDb, ss.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return games;

	while (sqlite3_step(stmt) == SQLITE_ROW)
	{
		GameMetadata game;
		game.id = sqlite3_column_int(stmt, 0);
		game.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

		const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
		game.description = desc ? desc : "";

		const char* cover = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
		game.coverArtPath = cover ? cover : "";

		game.executablePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
		game.gameType = stringToGameType(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
		game.source = stringToGameSource(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));

		const char* install = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
		game.installPath = install ? install : "";

		game.lastPlayed = sqlite3_column_int64(stmt, 8);
		game.playCount = sqlite3_column_int(stmt, 9);
		game.createdAt = sqlite3_column_int64(stmt, 10);

		games.push_back(game);
	}

	sqlite3_finalize(stmt);
	return games;
}

bool DatabaseManager::updatePlayStats(int gameId)
{
	if (!mInitialized)
		return false;

	std::stringstream ss;
	ss << "UPDATE games SET last_played=" << getCurrentTimestamp()
	   << ", play_count=play_count+1 WHERE id=" << gameId << ";";

	return executeSql(ss.str());
}

bool DatabaseManager::insertItchGameData(int gameId, int itchId, int uploadId,
	const std::string& downloadUrl, const std::string& version)
{
	if (!mInitialized)
		return false;

	sqlite3_stmt* stmt;
	const char* sql = "INSERT INTO itch_games (game_id, itch_id, upload_id, download_url, version) VALUES (?, ?, ?, ?, ?);";

	if (sqlite3_prepare_v2(mDb, sql, -1, &stmt, nullptr) != SQLITE_OK)
		return false;

	sqlite3_bind_int(stmt, 1, gameId);
	sqlite3_bind_int(stmt, 2, itchId);
	sqlite3_bind_int(stmt, 3, uploadId);
	sqlite3_bind_text(stmt, 4, downloadUrl.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 5, version.c_str(), -1, SQLITE_TRANSIENT);

	int rc = sqlite3_step(stmt);
	sqlite3_finalize(stmt);

	return rc == SQLITE_DONE;
}

bool DatabaseManager::updateItchGameData(int gameId, const std::string& downloadUrl,
	const std::string& version)
{
	if (!mInitialized)
		return false;

	std::stringstream ss;
	ss << "UPDATE itch_games SET download_url='" << downloadUrl
	   << "', version='" << version << "' WHERE game_id=" << gameId << ";";

	return executeSql(ss.str());
}

GameMetadata DatabaseManager::getGameByItchId(int itchId)
{
	GameMetadata game;

	if (!mInitialized)
		return game;

	std::stringstream ss;
	ss << "SELECT g.* FROM games g INNER JOIN itch_games ig ON g.id=ig.game_id WHERE ig.itch_id=" << itchId << ";";

	sqlite3_stmt* stmt;
	if (sqlite3_prepare_v2(mDb, ss.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK)
		return game;

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		game.id = sqlite3_column_int(stmt, 0);
		game.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

		const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
		game.description = desc ? desc : "";

		const char* cover = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
		game.coverArtPath = cover ? cover : "";

		game.executablePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
		game.gameType = stringToGameType(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
		game.source = stringToGameSource(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));

		const char* install = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
		game.installPath = install ? install : "";

		game.lastPlayed = sqlite3_column_int64(stmt, 8);
		game.playCount = sqlite3_column_int(stmt, 9);
		game.createdAt = sqlite3_column_int64(stmt, 10);
		game.itchId = itchId;
	}

	sqlite3_finalize(stmt);
	return game;
}

std::string DatabaseManager::gameTypeToString(GameType type)
{
	switch (type)
	{
		case GameType::APPIMAGE: return "appimage";
		case GameType::RENPY: return "renpy";
		case GameType::DEB: return "deb";
		case GameType::ELECTRON: return "electron";
		case GameType::DIRECTORY: return "directory";
		default: return "unknown";
	}
}

GameType DatabaseManager::stringToGameType(const std::string& str)
{
	if (str == "appimage") return GameType::APPIMAGE;
	if (str == "renpy") return GameType::RENPY;
	if (str == "deb") return GameType::DEB;
	if (str == "electron") return GameType::ELECTRON;
	if (str == "directory") return GameType::DIRECTORY;
	return GameType::UNKNOWN;
}

std::string DatabaseManager::gameSourceToString(GameSource source)
{
	switch (source)
	{
		case GameSource::LOCAL: return "local";
		case GameSource::ITCH_IO: return "itch_io";
		case GameSource::USB: return "usb";
		default: return "unknown";
	}
}

GameSource DatabaseManager::stringToGameSource(const std::string& str)
{
	if (str == "local") return GameSource::LOCAL;
	if (str == "itch_io") return GameSource::ITCH_IO;
	if (str == "usb") return GameSource::USB;
	return GameSource::UNKNOWN;
}

} // namespace OpenConsole
