# OpenConsole Configuration Guide

This guide covers all configuration options for OpenConsole, including database setup, plugin configuration, and system settings.

## Configuration Files

OpenConsole uses multiple configuration files:

```
~/.openconsole/
├── games.db                    # SQLite game database (NEW)
├── credentials.enc             # Encrypted API tokens (NEW, planned)
└── config.xml                  # OpenConsole settings (NEW, planned)

~/.emulationstation/
├── es_settings.cfg             # User settings (inherited)
├── es_systems.cfg              # System definitions (inherited)
├── es_input.cfg                # Controller mappings (inherited)
└── es_log.txt                  # Application log
```

## Database Configuration

### Database Location

Default: `~/.openconsole/games.db`

To use a custom location:

```cpp
DatabaseManager& db = DatabaseManager::getInstance();
db.init("/custom/path/games.db");
```

### Database Schema

The database is automatically created on first run with the following tables:

**games table**:
- Stores all game metadata
- Indexed on source, game_type, last_played

**itch_games table**:
- Stores itch.io-specific data
- Foreign key to games table with CASCADE delete

### Manual Database Operations

```bash
# View database
sqlite3 ~/.openconsole/games.db

# List all games
SELECT name, game_type, source FROM games;

# Recent games
SELECT name, datetime(last_played, 'unixepoch')
FROM games
WHERE last_played > 0
ORDER BY last_played DESC
LIMIT 10;

# Games by source
SELECT source, COUNT(*) as count
FROM games
GROUP BY source;

# Clear all games (destructive!)
DELETE FROM games;

# Backup database
cp ~/.openconsole/games.db ~/.openconsole/games.db.backup
```

## Plugin Configuration

### LocalFilesystemPlugin

Configure scan paths programmatically:

```cpp
auto plugin = std::dynamic_pointer_cast<LocalFilesystemPlugin>(
    PluginManager::getInstance().getPlugin("local")
);

// Add custom scan path
plugin->addScanPath("/mnt/external/Games");

// Remove scan path
plugin->removeScanPath("/media/usb0/Games");

// Configure scanning behavior
plugin->setRecursiveScan(true);       // Enable recursive scanning
plugin->setMaxScanDepth(5);           // Max depth (0 = unlimited)

// Get current paths
std::vector<std::string> paths = plugin->getScanPaths();
for (const auto& path : paths) {
    std::cout << "Scanning: " << path << std::endl;
}
```

**Default Scan Paths**:
- `~/Games`
- `~/.local/share/games`
- `/usr/local/games`
- `/media/usb0/Games`
- `/media/games`

### Future: config.xml

Planned configuration file format:

```xml
<?xml version="1.0"?>
<openconsole>
    <database>
        <path>~/.openconsole/games.db</path>
    </database>

    <plugins>
        <local enabled="true">
            <scanPaths>
                <path>~/Games</path>
                <path>/media/usb0/Games</path>
            </scanPaths>
            <recursive>true</recursive>
            <maxDepth>5</maxDepth>
        </local>

        <itchio enabled="true">
            <autoSync>true</autoSync>
            <syncInterval>3600</syncInterval>
        </itchio>
    </plugins>

    <launchers>
        <appimage>
            <makeExecutable>true</makeExecutable>
        </appimage>

        <deb>
            <autoInstall>false</autoInstall>
        </deb>
    </launchers>

    <scanning>
        <skipDuplicates>true</skipDuplicates>
        <updateExisting>false</updateExisting>
    </scanning>
</openconsole>
```

## Game Scanner Configuration

```cpp
GameScanner scanner;

// Configure duplicate handling
scanner.setSkipDuplicates(true);    // Skip games already in database
scanner.setUpdateExisting(false);    // Don't update existing entries

// Perform scan with progress
scanner.scanAllSources([](int current, int total, const std::string& name) {
    std::cout << "Scanning " << current << "/" << total
              << ": " << name << std::endl;
});
```

## Inherited Settings (EmulationStation)

### Settings File

Location: `~/.emulationstation/es_settings.cfg`

Format: Simple key=value pairs

**Common Settings**:
```ini
# UI Settings
UIMode=Full
UIMode_passkey=
TransitionStyle=slide
EnableSounds=1
ShowHelpPrompts=1

# Display
Windowed=0
FullscreenBorderless=0
WindowWidth=1920
WindowHeight=1080
ScreenWidth=1920
ScreenHeight=1080

# Collections
CollectionSystemsAuto=favorites,lastplayed
CollectionSystemsCustom=

# Scraper
Scraper=TheGamesDB
ScraperResizeWidth=400
ScraperResizeHeight=0

# Screensaver
ScreenSaverTime=300000
ScreenSaverType=dim
ScreenSaverBehavior=random
```

### Modifying Settings

**Programmatically**:
```cpp
#include "Settings.h"

Settings::getInstance()->setBool("EnableSounds", true);
Settings::getInstance()->setString("UIMode", "Full");
Settings::getInstance()->setInt("ScreenSaverTime", 300000);

// Save to disk
Settings::getInstance()->saveFile();
```

**Manually**:
Edit `~/.emulationstation/es_settings.cfg`:
```bash
nano ~/.emulationstation/es_settings.cfg
```

### Systems Configuration

Location: `~/.emulationstation/es_systems.cfg` or `/etc/emulationstation/es_systems.cfg`

**Note**: OpenConsole is transitioning away from system-based organization, but this file is still used for compatibility.

## Input Configuration

Location: `~/.emulationstation/es_input.cfg`

Controller mappings stored as XML:

```xml
<inputList>
    <inputConfig type="joystick" deviceName="Xbox Controller">
        <input name="a" type="button" id="0" value="1"/>
        <input name="b" type="button" id="1" value="1"/>
        <input name="x" type="button" id="2" value="1"/>
        <input name="y" type="button" id="3" value="1"/>
        <input name="start" type="button" id="7" value="1"/>
        <input name="select" type="button" id="6" value="1"/>
        <input name="up" type="axis" id="1" value="-1"/>
        <input name="down" type="axis" id="1" value="1"/>
        <input name="left" type="axis" id="0" value="-1"/>
        <input name="right" type="axis" id="0" value="1"/>
    </inputConfig>
</inputList>
```

### Configuring Controllers

1. Run OpenConsole
2. If no controllers configured, auto-detection dialog appears
3. Press and hold any button on controller
4. Follow on-screen prompts to map buttons
5. Configuration saved automatically

**Reconfigure Controller**:
```bash
# Remove existing config
rm ~/.emulationstation/es_input.cfg

# Restart OpenConsole
./emulationstation
```

## Logging Configuration

### Log Levels

```cpp
#include "Log.h"

// Available log levels
LOG(LogError)   << "Critical errors";
LOG(LogWarning) << "Warnings";
LOG(LogInfo)    << "General information";
LOG(LogDebug)   << "Debug details";  // Only with --debug flag
```

### Viewing Logs

```bash
# Real-time log monitoring
tail -f ~/.emulationstation/es_log.txt

# Filter for errors only
grep "lvl0:" ~/.emulationstation/es_log.txt

# Filter for specific component
grep "DatabaseManager" ~/.emulationstation/es_log.txt

# Last 50 lines
tail -50 ~/.emulationstation/es_log.txt
```

### Log Rotation

Logs are appended to es_log.txt. For log rotation:

```bash
# Manual rotation
mv ~/.emulationstation/es_log.txt ~/.emulationstation/es_log.txt.old

# Or truncate
> ~/.emulationstation/es_log.txt
```

## Theme Configuration

Location: `~/.emulationstation/themes/`

OpenConsole is compatible with EmulationStation themes.

**Installing a Theme**:
```bash
cd ~/.emulationstation/themes/
git clone https://github.com/user/es-theme-example.git
```

**Selecting Theme**:
Via UI: Main Menu → UI Settings → Theme Set

Or edit settings:
```bash
echo 'ThemeSet=es-theme-example' >> ~/.emulationstation/es_settings.cfg
```

## Performance Tuning

### Database Performance

**Enable WAL mode** (Write-Ahead Logging):
```bash
sqlite3 ~/.openconsole/games.db "PRAGMA journal_mode=WAL;"
```

**Analyze and optimize**:
```bash
sqlite3 ~/.openconsole/games.db "ANALYZE;"
sqlite3 ~/.openconsole/games.db "VACUUM;"
```

### Scanning Performance

```cpp
// Limit scan depth for faster scanning
plugin->setMaxScanDepth(3);  // Default is 5

// Disable recursive scanning for flat directories
plugin->setRecursiveScan(false);
```

### Memory Usage

**Limit VRAM usage**:
```ini
# In es_settings.cfg
MaxVRAM=80  # MB
```

**Disable video previews**:
```ini
VideoAudio=0
```

## Security Configuration

### Credential Storage (Planned)

Encrypted token storage using OpenSSL:

```cpp
// Future implementation
CredentialManager::getInstance().storeToken(
    "itch_io",
    apiKey,
    EncryptionType::AES256
);
```

### File Permissions

Ensure correct permissions:

```bash
# Config directory
chmod 700 ~/.openconsole

# Database
chmod 600 ~/.openconsole/games.db

# Credentials (future)
chmod 600 ~/.openconsole/credentials.enc
```

## Platform-Specific Configuration

### Raspberry Pi

**GPU Memory**:
```bash
# Edit /boot/config.txt
gpu_mem=256  # Increase for better performance
```

**Overclocking** (Pi 4/5):
```bash
# /boot/config.txt
over_voltage=6
arm_freq=2000
```

**Auto-start**:
```bash
# Create systemd service
sudo systemctl enable openconsole
```

### Linux Desktop

**Resolution**:
```bash
./emulationstation --resolution 1920 1080
```

**Windowed Mode**:
```bash
./emulationstation --windowed
```

## Troubleshooting Configuration

### Reset to Defaults

```bash
# Backup current config
cp -r ~/.emulationstation ~/.emulationstation.backup
cp -r ~/.openconsole ~/.openconsole.backup

# Remove configs
rm -rf ~/.emulationstation
rm -rf ~/.openconsole

# Restart OpenConsole (will recreate defaults)
./emulationstation
```

### Verify Configuration

```cpp
// Check database
DatabaseManager& db = DatabaseManager::getInstance();
if (!db.isInitialized()) {
    LOG(LogError) << "Database not initialized!";
}

// Check plugins
PluginManager& pm = PluginManager::getInstance();
if (pm.getPluginCount() == 0) {
    LOG(LogError) << "No plugins registered!";
}

// Check settings
if (!Settings::getInstance()->loaded()) {
    LOG(LogError) << "Settings not loaded!";
}
```

### Common Issues

**Database locked**:
```bash
# Check for other processes
ps aux | grep emulationstation

# Kill if needed
killall emulationstation

# Remove lock
rm ~/.openconsole/games.db-shm
rm ~/.openconsole/games.db-wal
```

**Missing games**:
```cpp
// Verify scan paths exist
auto plugin = pm.getPlugin("local");
for (const auto& path : plugin->getScanPaths()) {
    if (!Utils::FileSystem::exists(path)) {
        LOG(LogWarning) << "Scan path not found: " << path;
    }
}
```

**Controller not detected**:
```bash
# Check SDL2 sees controller
sdl2-jstest --list

# Verify permissions
ls -l /dev/input/js*
```

## Environment Variables

```bash
# Override home directory
HOME=/custom/path ./emulationstation

# Enable debugging
ES_DEBUG=1 ./emulationstation

# Set config directory (planned)
OPENCONSOLE_HOME=/custom/config ./emulationstation
```

## Configuration Best Practices

### 1. Regular Backups

```bash
#!/bin/bash
# backup-openconsole.sh

DATE=$(date +%Y%m%d)
BACKUP_DIR=~/openconsole-backups/$DATE

mkdir -p $BACKUP_DIR

# Backup database
cp ~/.openconsole/games.db $BACKUP_DIR/

# Backup settings
cp -r ~/.emulationstation $BACKUP_DIR/

echo "Backup completed: $BACKUP_DIR"
```

### 2. Version Control

```bash
# Track configuration changes
cd ~/.openconsole
git init
git add .
git commit -m "Initial configuration"
```

### 3. Documentation

Keep notes on custom configuration:

```bash
# README.txt in ~/.openconsole
This setup includes:
- Custom scan path: /mnt/games
- Max scan depth: 3
- Disabled screensaver
- Theme: minimal-dark
```

## Related Documentation

- [Architecture Overview](../ARCHITECTURE.md)
- [Development Guide](../development/GETTING_STARTED.md)
- [Plugin Configuration](CREATING_PLUGINS.md)
