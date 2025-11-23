# itch.io Integration Guide

This guide covers the complete itch.io integration in OpenConsole, including authentication, game library access, downloads, and security.

## Table of Contents

1. [Overview](#overview)
2. [Getting Your API Key](#getting-your-api-key)
3. [Authentication](#authentication)
4. [Browsing Your Library](#browsing-your-library)
5. [Downloading Games](#downloading-games)
6. [Security](#security)
7. [Troubleshooting](#troubleshooting)
8. [Developer Reference](#developer-reference)

## Overview

OpenConsole provides built-in itch.io integration for seamless access to your indie game library. The integration supports:

- **Built-in Feature**: itch.io support is always available, no plugin installation needed
- **API Key Authentication**: Secure access using itch.io API keys
- **Library Browsing**: View all games you own on itch.io
- **Direct Downloads**: Download games with progress tracking
- **Encrypted Storage**: API keys stored securely with AES-256 encryption
- **Auto-Detection**: Automatic game type recognition
- **Controller-First**: All operations possible with just a controller
- **Always Accessible**: Configure itch.io in any UI mode (Full, Kid, Kiosk)

### Architecture

The itch.io integration consists of four main components:

1. **TokenStorage**: Secure credential storage (`es-app/src/utils/`)
2. **ItchIoApiClient**: HTTP client for itch.io API (`es-app/src/api/`)
3. **ItchIoPlugin**: Built-in game source plugin (`es-app/src/plugins/`)
4. **GuiOpenConsoleSettings**: Configuration UI with itch.io section (`es-app/src/guis/`)

## Getting Your API Key

itch.io uses API keys for authentication. Here's how to get yours:

### Step-by-Step Instructions

1. **Open a Web Browser**
   - On any device (phone, computer, tablet)
   - You can do this while OpenConsole is running

2. **Visit itch.io API Settings**
   - Navigate to: https://itch.io/user/settings/api-keys
   - Log in with your itch.io account if prompted

3. **Generate New API Key**
   - Click the **"Generate new API key"** button
   - The key will appear instantly

4. **Copy the API Key**
   - Copy the entire key string
   - It will look something like: `abc123def456ghi789jkl012mno345pqr678`

5. **Keep It Secret**
   - Don't share your API key with anyone
   - Anyone with your key can access your game library

### API Key Permissions

The API key grants access to:
- ✅ Your profile information (username, display name)
- ✅ Your owned games library (purchased and claimed games)
- ✅ Download URLs for your games
- ❌ Cannot make purchases
- ❌ Cannot modify your account
- ❌ Cannot access payment information

## Authentication

### Using the OpenConsole UI

1. **Open OpenConsole Settings**
   - From the main menu, select **"OPENCONSOLE SETTINGS"**
   - This menu is now accessible in all UI modes (Full, Kid, Kiosk, etc.)
   - Navigate to the **itch.io** section

2. **Check itch.io Status**
   - You'll see: **"itch.io Status: Not authenticated"** (in red)
   - Below that: **"Configure itch.io"** → **"CONFIGURE"**

3. **Open Authentication Dialog**
   - Select **"itch.io Authentication"** → **"CONFIGURE"**
   - The itch.io authentication dialog opens

4. **View Instructions (Optional)**
   - Select **"VIEW INSTRUCTIONS"**
   - Read the step-by-step API key instructions

5. **Enter Your API Key**
   - Select **"ENTER API KEY"**
   - The virtual keyboard opens
   - Use your controller to enter the API key:
     - **D-pad**: Navigate the keyboard
     - **A**: Select character
     - **B**: Backspace
     - **X**: Space
     - **Y**: Toggle shift (uppercase/lowercase)
     - **Start**: Submit

6. **Authentication Test**
   - OpenConsole automatically tests your API key
   - Shows: **"Testing API key... Please wait..."**
   - On success: **"Authenticated as: YourUsername"**
   - On failure: Error message with details

7. **Confirmation**
   - Success message: **"Successfully authenticated with itch.io!"**
   - Status changes to: **"itch.io Status: Authenticated"** (in green)

### Testing Your Connection

To verify authentication works:

1. Open **itch.io Authentication** dialog
2. Select **"TEST CONNECTION"** (appears when API key exists)
3. OpenConsole will:
   - Test the API key
   - Fetch your profile
   - Display results

### Logging Out

To remove your API key:

1. Open **itch.io Authentication** dialog
2. Select **"LOGOUT"**
3. Confirms removal
4. API key deleted from encrypted storage
5. Status returns to **"Not authenticated"**

## Browsing Your Library

Once authenticated, your itch.io games appear automatically when scanning.

### Scanning for Games

1. **Open OpenConsole Settings**
   - Navigate to **Scanning** section

2. **Start Scan**
   - Select **"Scan for Games Now"** → **"START SCAN"**
   - OpenConsole scans both:
     - Local directories (LocalFilesystemPlugin)
     - itch.io library (ItchIoPlugin, if authenticated)

3. **Scan Progress**
   - Shows scanning message with progress updates
   - Processes games from all authenticated sources
   - Displays detailed statistics when complete

4. **View Results**
   - **Total games found**: All games discovered from all sources
   - **New games added**: Games newly added to the database
   - **Games skipped**: Duplicate or previously scanned games
   - **Errors**: Any issues encountered during scanning
   - **Scan duration**: Time taken to complete the scan
   - Game list automatically reloads to show new games

### What Gets Scanned

The ItchIoPlugin retrieves:
- ✅ All games you've purchased
- ✅ All games you've claimed (free games)
- ✅ Games from bundles
- ✅ Games from game jams (if claimed)
- ❌ Games you don't own (browsing-only)

### Game Information Retrieved

For each owned game:
- **Title**: Game name
- **Description**: Short description
- **Cover Art**: Game cover image URL
- **Upload ID**: For downloading
- **File Information**: Filename, size
- **Game Type**: Auto-detected from filename

### Supported Game Types

OpenConsole auto-detects these types from itch.io downloads:

| Type | Detection | Example |
|------|-----------|---------|
| AppImage | `.appimage` in filename | `game.AppImage` |
| Ren'Py | `renpy` in filename | `game-renpy-linux.tar.gz` |
| .deb | `.deb` extension | `game_1.0_amd64.deb` |
| Electron | `electron` in filename | `game-electron.zip` |
| Archive | `.zip`, `.tar.gz` | `game-linux.zip` |

## Downloading Games

### Download Process

Games download automatically when you add them from itch.io:

1. **Game Appears in List**
   - After scanning, itch.io games appear
   - Marked with itch.io as source

2. **Select Game to Download**
   - (Future feature: Manual download trigger)
   - Currently downloads during scan if needed

3. **Download Location**
   ```
   ~/.openconsole/downloads/itch.io/{game_id}/
   ```

4. **Progress Tracking**
   - Real-time download progress
   - Shows bytes downloaded / total bytes
   - Can be cancelled

5. **Auto-Install**
   - Downloads are extracted if needed
   - Game type detected
   - Launcher assigned automatically

### Download Workflow

```
User scans for games
    ↓
ItchIoPlugin::fetchGames()
    ↓
Get owned games from API
    ↓
Games added to database
    ↓
[User selects game]
    ↓
ItchIoPlugin::downloadGame()
    ↓
Get temporary download URL
    ↓
Download file with progress
    ↓
Save to install location
    ↓
Game ready to launch
```

### Download URLs

itch.io provides **temporary download URLs** that:
- Expire after a certain time
- Are regenerated on each download request
- Don't require authentication in the URL itself
- Use HTTPS for security

## Security

### Credential Storage

OpenConsole uses military-grade encryption to protect your API key.

#### Encryption Details

- **Algorithm**: AES-256-CBC
- **Key Derivation**: SHA-256 hash of machine-specific data
- **Key Material**:
  - Hostname
  - User home directory path
  - Fixed salt: `"OpenConsole-ItchIO-Token-Storage-v1"`
- **IV**: Randomly generated for each encryption operation (16 bytes)
- **Storage**: `~/.openconsole/credentials.enc`
- **Permissions**: 0600 (owner read/write only)

#### How It Works

1. **Storing an API Key**:
   ```
   User enters API key
       ↓
   TokenStorage::storeToken("itch_io", apiKey)
       ↓
   Generate encryption key from machine data
       ↓
   Generate random IV (16 bytes)
       ↓
   Encrypt API key with AES-256-CBC
       ↓
   Prepend IV to ciphertext
       ↓
   Convert to JSON: {"itch_io": "<encrypted_data>"}
       ↓
   Encrypt JSON with AES-256-CBC
       ↓
   Write to ~/.openconsole/credentials.enc
       ↓
   Set file permissions to 0600
   ```

2. **Retrieving an API Key**:
   ```
   ItchIoPlugin needs API key
       ↓
   TokenStorage::getToken("itch_io")
       ↓
   Read ~/.openconsole/credentials.enc
       ↓
   Generate encryption key from machine data
       ↓
   Extract IV from ciphertext
       ↓
   Decrypt with AES-256-CBC
       ↓
   Parse JSON
       ↓
   Return API key
   ```

#### Security Features

✅ **AES-256 Encryption**: Industry-standard strong encryption
✅ **Machine-Specific Keys**: Credentials don't work if file is copied to another machine
✅ **No Plaintext Storage**: API key never stored in readable form
✅ **Secure File Permissions**: Only your user can read the file
✅ **Random IVs**: Each encryption operation uses a unique initialization vector
✅ **No Key Logging**: API keys not written to log files

#### Limitations

⚠️ **Root Access**: System administrator can read the file
⚠️ **Memory Dumps**: API key is in memory when in use
⚠️ **Process Inspection**: Debugger could extract key from running process
⚠️ **Not Hardware-Backed**: Doesn't use TPM or secure enclave

This level of security is appropriate for:
- ✅ Protecting against casual file access
- ✅ Preventing accidental key exposure
- ✅ Stopping key theft from backups
- ❌ Protecting against determined attacker with root access
- ❌ Protecting against physical disk theft (use disk encryption)

### Best Practices

1. **Revoke Compromised Keys**
   - If you suspect your API key is compromised
   - Visit https://itch.io/user/settings/api-keys
   - Delete the old key
   - Generate a new one
   - Re-authenticate in OpenConsole

2. **Don't Share credentials.enc**
   - Don't include in backups shared with others
   - Don't commit to version control
   - Don't upload to cloud storage

3. **Use Disk Encryption**
   - For additional security
   - Encrypt your home directory or full disk
   - Protects against physical theft

4. **Regular Key Rotation**
   - Regenerate API key periodically
   - itch.io allows multiple keys
   - Old keys can be revoked

## Troubleshooting

### Authentication Fails

**Symptom**: "Authentication failed" message

**Solutions**:

1. **Check API Key**
   - Ensure you copied the entire key
   - No extra spaces at beginning/end
   - No line breaks in the middle

2. **Verify Key is Active**
   - Visit https://itch.io/user/settings/api-keys
   - Ensure key hasn't been revoked
   - Try generating a new key

3. **Check Network Connection**
   - OpenConsole needs internet access
   - Test connection in a browser
   - Check firewall settings

4. **API Key Permissions**
   - Ensure key has necessary permissions
   - Default keys have profile + library access

### Games Don't Appear After Scan

**Symptom**: Scan completes but no itch.io games shown

**Solutions**:

1. **Verify Authentication**
   - Check itch.io status is "Authenticated"
   - Re-test connection
   - Re-scan if needed

2. **Check Game Ownership**
   - Visit https://itch.io/my-purchases
   - Ensure you own games
   - Free games must be "claimed" not just played

3. **Review Scan Logs**
   - Check `~/.emulationstation/es_log.txt`
   - Look for ItchIoPlugin errors
   - Check for API errors

4. **Database Check**
   - OpenConsole Settings → Database → View Statistics
   - Check if games exist with itch.io source
   - May need to refresh game list

### Downloads Fail

**Symptom**: Download starts but fails

**Solutions**:

1. **Check Disk Space**
   - Ensure enough free space in `~/.openconsole/downloads/`
   - Games can be large (multiple GB)

2. **Network Issues**
   - Check internet connection stability
   - Try download again (URLs regenerate)
   - Check for network firewall blocking downloads

3. **File Permissions**
   - Ensure write access to `~/.openconsole/`
   - Check directory ownership

4. **Temporary URLs Expired**
   - Download URLs expire
   - Rescan to get fresh URLs
   - Download promptly after scanning

### "Not Authenticated" After Reboot

**Symptom**: Shows as not authenticated after restarting

**Solutions**:

1. **Check credentials.enc Exists**
   ```bash
   ls -la ~/.openconsole/credentials.enc
   ```
   - Should show file with 0600 permissions

2. **File Permissions Correct**
   ```bash
   chmod 600 ~/.openconsole/credentials.enc
   ```

3. **Not Corrupted**
   - File should not be 0 bytes
   - If corrupted, re-authenticate

4. **Machine-Specific Keys**
   - Credentials tied to machine
   - If hostname changed, may need to re-authenticate

## Developer Reference

### ItchIoPlugin API

#### Authentication

```cpp
#include "plugins/ItchIoPlugin.h"

// Get plugin from manager
auto& pm = PluginManager::getInstance();
auto itchPlugin = std::dynamic_pointer_cast<ItchIoPlugin>(pm.getPlugin("itch_io"));

// Set API key
itchPlugin->setApiKey("your_api_key_here");

// Authenticate
AuthResult result = itchPlugin->authenticate();
if (result.success) {
    std::cout << "Authenticated as: " << result.userName << std::endl;
} else {
    std::cerr << "Auth failed: " << result.errorMessage << std::endl;
}

// Check authentication status
bool isAuth = itchPlugin->isAuthenticated();
```

#### Fetching Games

```cpp
// Get owned games
std::vector<GameMetadata> games = itchPlugin->fetchGames();

for (const auto& game : games) {
    std::cout << game.name << " (" << game.itchId << ")" << std::endl;
    std::cout << "  Type: " << (int)game.gameType << std::endl;
    std::cout << "  Upload ID: " << game.uploadId << std::endl;
}
```

#### Downloading

```cpp
// Download with progress callback
bool success = itchPlugin->downloadGame(
    game,
    "/path/to/install",
    [](size_t downloaded, size_t total) {
        float percent = (float)downloaded / total * 100.0f;
        std::cout << "Progress: " << percent << "%" << std::endl;
    }
);

if (success) {
    std::cout << "Download complete!" << std::endl;
} else {
    std::cerr << "Download failed: " << itchPlugin->getLastError() << std::endl;
}
```

### TokenStorage API

#### Storing Credentials

```cpp
#include "utils/TokenStorage.h"

auto& storage = TokenStorage::getInstance();

// Initialize
if (!storage.initialize()) {
    std::cerr << "Failed to init: " << storage.getLastError() << std::endl;
    return;
}

// Store a token
if (storage.storeToken("service_name", "api_key_value")) {
    std::cout << "Token stored securely" << std::endl;
}
```

#### Retrieving Credentials

```cpp
// Get a token
std::string token = storage.getToken("service_name");
if (!token.empty()) {
    std::cout << "Retrieved token: " << token << std::endl;
}

// Check if token exists
if (storage.hasToken("service_name")) {
    std::cout << "Token exists" << std::endl;
}
```

#### Managing Credentials

```cpp
// Remove a token
storage.removeToken("service_name");

// Clear all tokens
storage.clearAll();
```

### ItchIoApiClient API

#### Making API Calls

```cpp
#include "api/ItchIoApiClient.h"

ItchIoApiClient client;
client.setApiKey("your_api_key");

// Get user profile
ItchIoProfile profile;
if (client.getProfile(profile)) {
    std::cout << "Username: " << profile.username << std::endl;
    std::cout << "Display Name: " << profile.displayName << std::endl;
}

// Get owned games
std::vector<ItchIoGame> games = client.getOwnedGames();
for (const auto& game : games) {
    std::cout << game.title << " - " << game.id << std::endl;
}

// Get download URL
std::string downloadUrl = client.getDownloadUrl(uploadId);
```

#### Downloading Files

```cpp
// Download with progress
bool success = client.downloadFile(
    downloadUrl,
    "/path/to/output.zip",
    [](size_t current, size_t total) {
        std::cout << current << " / " << total << " bytes" << std::endl;
    }
);
```

### Custom Plugin Integration

To integrate itch.io in a custom plugin:

```cpp
#include "plugins/IGameSourcePlugin.h"
#include "api/ItchIoApiClient.h"
#include "utils/TokenStorage.h"

class MyCustomItchPlugin : public IGameSourcePlugin {
private:
    ItchIoApiClient mClient;

public:
    bool initialize() override {
        // Load API key from storage
        auto& storage = TokenStorage::getInstance();
        storage.initialize();

        std::string apiKey = storage.getToken("my_plugin");
        if (!apiKey.empty()) {
            mClient.setApiKey(apiKey);
        }

        return true;
    }

    std::vector<GameMetadata> fetchGames() override {
        std::vector<GameMetadata> result;

        // Use client to fetch games
        auto itchGames = mClient.getOwnedGames();

        // Convert to GameMetadata
        for (const auto& game : itchGames) {
            GameMetadata meta;
            meta.name = game.title;
            meta.source = GameSource::ITCH_IO;
            // ... set other fields
            result.push_back(meta);
        }

        return result;
    }
};
```

## API Reference Links

- **itch.io API Documentation**: https://itch.io/docs/api/overview
- **API Key Management**: https://itch.io/user/settings/api-keys
- **OpenSSL Documentation**: https://www.openssl.org/docs/
- **libCURL Documentation**: https://curl.se/libcurl/

## Related Documentation

- [Plugin System Details](../architecture/PLUGINS.md)
- [Database Layer](../architecture/DATABASE.md)
- [UI Components Guide](UI_COMPONENTS.md)
- [Architecture Overview](../ARCHITECTURE.md)

## Future Enhancements

Planned improvements for itch.io integration:

- **OAuth Authentication**: Alternative to API keys
- **Game Updates**: Detect and download updates
- **Install Manager**: UI for managing installed games
- **Selective Downloads**: Choose specific files from multi-file uploads
- **Cover Art Caching**: Download and cache cover images
- **Game Collections**: Browse by collection/bundle
- **Search**: Search your itch.io library
- **Recommendations**: Show recommended games based on library
