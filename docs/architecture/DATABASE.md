# Database Layer

The OpenConsole database layer provides persistent storage for game metadata, play statistics, and integration data. It's built on SQLite3 for reliability, performance, and zero-configuration deployment.

## Overview

- **Technology**: SQLite3
- **Location**: `~/.openconsole/games.db`
- **Implementation**: `es-app/src/db/DatabaseManager.h/cpp`
- **Pattern**: Singleton with prepared statements
- **Thread Safety**: Single-threaded (main thread only)

## Architecture

```
┌──────────────────────────────┐
│     DatabaseManager          │
│      (Singleton)             │
├──────────────────────────────┤
│  - init()                    │
│  - insertGame()              │
│  - updateGame()              │
│  - getGame()                 │
│  - getAllGames()             │
│  - getGamesBySource()        │
│  - getRecentlyPlayed()       │
└──────────────────────────────┘
            ↓
┌──────────────────────────────┐
│       SQLite3                │
│                              │
│  ┌────────────────────┐      │
│  │   games table      │      │
│  └────────────────────┘      │
│  ┌────────────────────┐      │
│  │ itch_games table   │      │
│  └────────────────────┘      │
└──────────────────────────────┘
```

## Database Schema

### games Table

Stores metadata for all games regardless of source.

```sql
CREATE TABLE games (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    description TEXT,
    cover_art_path TEXT,
    executable_path TEXT NOT NULL,
    game_type TEXT NOT NULL,           -- 'appimage', 'renpy', 'deb', 'electron'
    source TEXT NOT NULL,               -- 'local', 'itch_io', 'usb'
    install_path TEXT,
    last_played INTEGER DEFAULT 0,      -- Unix timestamp
    play_count INTEGER DEFAULT 0,
    created_at INTEGER DEFAULT 0        -- Unix timestamp
);

-- Indices for performance
CREATE INDEX idx_games_source ON games(source);
CREATE INDEX idx_games_type ON games(game_type);
CREATE INDEX idx_games_last_played ON games(last_played DESC);
```

#### Field Descriptions

| Field | Type | Description |
|-------|------|-------------|
| `id` | INTEGER | Auto-incrementing primary key |
| `name` | TEXT | Display name of the game (required) |
| `description` | TEXT | Game description/synopsis (optional) |
| `cover_art_path` | TEXT | Absolute path to cover image (optional) |
| `executable_path` | TEXT | Path to game executable/script (required) |
| `game_type` | TEXT | Type identifier for launcher selection |
| `source` | TEXT | Where the game came from |
| `install_path` | TEXT | Base installation directory |
| `last_played` | INTEGER | Unix timestamp of last play session |
| `play_count` | INTEGER | Number of times game has been launched |
| `created_at` | INTEGER | Unix timestamp when added to database |

### itch_games Table

Stores itch.io-specific data linked to games.

```sql
CREATE TABLE itch_games (
    game_id INTEGER PRIMARY KEY,
    itch_id INTEGER NOT NULL,           -- itch.io's internal game ID
    upload_id INTEGER,                  -- Specific build/upload ID
    download_url TEXT,                  -- Direct download URL
    version TEXT,                       -- Version string
    FOREIGN KEY (game_id) REFERENCES games(id) ON DELETE CASCADE
);

CREATE INDEX idx_itch_games_itch_id ON itch_games(itch_id);
```

#### Field Descriptions

| Field | Type | Description |
|-------|------|-------------|
| `game_id` | INTEGER | Foreign key to games.id |
| `itch_id` | INTEGER | itch.io's game identifier |
| `upload_id` | INTEGER | Specific upload/build identifier |
| `download_url` | TEXT | Direct download link from itch.io API |
| `version` | TEXT | Version string (e.g., "1.0.2") |

## API Reference

### Initialization

```cpp
DatabaseManager& db = DatabaseManager::getInstance();

// Initialize with database path
bool success = db.init("/home/user/.openconsole/games.db");
if (!success) {
    LOG(LogError) << "Failed to initialize database";
}
```

### Inserting Games

```cpp
GameMetadata game;
game.name = "My Awesome Game";
game.description = "An incredible adventure";
game.executablePath = "/home/user/Games/game.AppImage";
game.gameType = GameType::APPIMAGE;
game.source = GameSource::LOCAL;
game.coverArtPath = "/home/user/Games/cover.png";

int gameId = db.insertGame(game);
if (gameId > 0) {
    LOG(LogInfo) << "Game inserted with ID: " << gameId;
}
```

### Updating Games

```cpp
GameMetadata game = db.getGame(gameId);
game.description = "Updated description";
game.playCount++;

bool success = db.updateGame(game);
```

### Querying Games

```cpp
// Get all games
std::vector<GameMetadata> allGames = db.getAllGames();

// Get games by source
std::vector<GameMetadata> localGames = db.getGamesBySource(GameSource::LOCAL);

// Get games by type
std::vector<GameMetadata> appImageGames = db.getGamesByType(GameType::APPIMAGE);

// Get recently played games
std::vector<GameMetadata> recent = db.getRecentlyPlayed(10);

// Get specific game
GameMetadata game = db.getGame(gameId);
```

### Updating Play Statistics

```cpp
// Automatically increments play_count and updates last_played timestamp
bool success = db.updatePlayStats(gameId);
```

### itch.io Integration

```cpp
// Add itch.io-specific data
int gameId = db.insertGame(game);
db.insertItchGameData(
    gameId,
    12345,                                    // itch_id
    67890,                                    // upload_id
    "https://itch.io/downloads/...",         // download_url
    "1.0.0"                                   // version
);

// Query by itch ID
GameMetadata game = db.getGameByItchId(12345);

// Update itch data
db.updateItchGameData(gameId, "https://new-url.com", "1.0.1");
```

### Deleting Games

```cpp
bool success = db.deleteGame(gameId);
// Note: CASCADE will automatically delete itch_games entry
```

## Data Types

### GameMetadata Structure

```cpp
struct GameMetadata {
    int id;                           // Database ID (-1 if not inserted yet)
    std::string name;                 // Display name
    std::string description;          // Description text
    std::string coverArtPath;         // Path to cover image
    std::string executablePath;       // Path to executable
    GameType gameType;                // Type enum
    GameSource source;                // Source enum
    std::string installPath;          // Installation directory
    long long lastPlayed;             // Unix timestamp
    int playCount;                    // Launch counter
    long long createdAt;              // Unix timestamp

    // itch.io specific (optional)
    int itchId;                       // -1 if not from itch.io
    int uploadId;
    std::string downloadUrl;
    std::string version;
};
```

### GameType Enum

```cpp
enum class GameType {
    APPIMAGE,      // Linux AppImage
    RENPY,         // Ren'Py visual novel
    DEB,           // Debian package
    ELECTRON,      // Electron app
    DIRECTORY,     // Generic directory
    UNKNOWN        // Unknown type
};
```

**String Conversion**:
- `DatabaseManager::gameTypeToString(GameType)`
- `DatabaseManager::stringToGameType(const std::string&)`

### GameSource Enum

```cpp
enum class GameSource {
    LOCAL,         // Local filesystem
    ITCH_IO,       // itch.io
    USB,           // USB drive
    UNKNOWN        // Unknown source
};
```

**String Conversion**:
- `DatabaseManager::gameSourceToString(GameSource)`
- `DatabaseManager::stringToGameSource(const std::string&)`

## Performance

### Prepared Statements

The DatabaseManager uses prepared statements for frequently-used queries:

- `mInsertGameStmt` - INSERT INTO games
- `mUpdateGameStmt` - UPDATE games
- `mGetGameStmt` - SELECT by ID
- `mGetAllGamesStmt` - SELECT all games

This provides significant performance benefits:
- **Query compilation** happens once at init
- **Parameter binding** is fast
- **Execution** is optimized

### Indices

Strategic indices improve query performance:

```sql
-- Queries by source (local, itch.io, usb)
CREATE INDEX idx_games_source ON games(source);

-- Queries by type (appimage, renpy, deb)
CREATE INDEX idx_games_type ON games(game_type);

-- Recently played queries (sorted by timestamp DESC)
CREATE INDEX idx_games_last_played ON games(last_played DESC);

-- itch.io lookup
CREATE INDEX idx_itch_games_itch_id ON itch_games(itch_id);
```

### Transaction Support

For bulk operations, wrap in transactions:

```cpp
// Manual transaction (advanced)
db.executeSql("BEGIN TRANSACTION;");

for (auto& game : games) {
    db.insertGame(game);
}

db.executeSql("COMMIT;");
```

## Error Handling

All database operations log errors via the `Log` system:

```cpp
int gameId = db.insertGame(game);
if (gameId < 0) {
    LOG(LogError) << "Failed to insert game: " << game.name;
    // Handle error
}
```

Check initialization status:

```cpp
if (!db.isInitialized()) {
    LOG(LogError) << "Database not initialized!";
    return false;
}
```

## Database Migration

Currently, the schema is created on first run. Future versions may need migrations:

```cpp
// Future: Version-based migrations
CREATE TABLE schema_version (
    version INTEGER PRIMARY KEY
);
```

## Best Practices

### 1. Always Check Initialization

```cpp
auto& db = DatabaseManager::getInstance();
if (!db.isInitialized()) {
    // Initialize or fail gracefully
    return;
}
```

### 2. Set Timestamps Correctly

```cpp
game.createdAt = std::chrono::duration_cast<std::chrono::seconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
```

### 3. Validate Paths

```cpp
if (!Utils::FileSystem::exists(game.executablePath)) {
    LOG(LogWarning) << "Executable path does not exist";
}
```

### 4. Handle Duplicates

```cpp
// Check if game already exists before inserting
std::vector<GameMetadata> existing = db.getAllGames();
for (const auto& g : existing) {
    if (g.executablePath == game.executablePath) {
        // Handle duplicate
        return;
    }
}
```

### 5. Use Foreign Keys

Foreign keys are enabled via `PRAGMA foreign_keys = ON;` at initialization. This ensures:
- Cascading deletes (removing game removes itch_games entry)
- Referential integrity

## Debugging

### View Database Contents

```bash
sqlite3 ~/.openconsole/games.db

# List all games
SELECT name, game_type, source FROM games;

# Show recently played
SELECT name, datetime(last_played, 'unixepoch') FROM games
WHERE last_played > 0
ORDER BY last_played DESC;

# Count by source
SELECT source, COUNT(*) FROM games GROUP BY source;
```

### Enable SQL Logging

Modify `DatabaseManager.cpp` to log SQL statements:

```cpp
LOG(LogDebug) << "Executing SQL: " << sql;
```

## Thread Safety

⚠️ **Important**: DatabaseManager is **not thread-safe**. All database operations must occur on the main thread.

For async operations:
1. Perform database queries on main thread
2. Process results on worker thread
3. Return to main thread for UI updates

## Future Improvements

Planned enhancements:

1. **Connection Pooling**: Multiple database connections
2. **Write-Ahead Logging**: Better concurrency
3. **Schema Versioning**: Automatic migrations
4. **Backup/Restore**: Database backup utilities
5. **Full-Text Search**: FTS5 for game search
6. **Statistics Table**: Detailed play session data

## Related Documentation

- [Architecture Overview](../ARCHITECTURE.md)
- [Plugin System](PLUGINS.md)
- [Database API Reference](../api/DATABASE_API.md)
- [Game Scanner](../guides/GAME_SCANNER.md)
