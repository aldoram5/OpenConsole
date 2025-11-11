# Game Launcher System

The Game Launcher System provides a flexible, type-specific approach to executing different game formats. It uses a factory pattern for automatic launcher selection and supports process management for running games.

## Overview

- **Pattern**: Factory Pattern + Strategy Pattern
- **Location**: `es-app/src/launchers/`
- **Factory**: `GameLauncherFactory`
- **Interface**: `IGameLauncher`
- **Built-in Launchers**: AppImage, Ren'Py, .deb packages

## Architecture

```
┌────────────────────────────────┐
│   GameLauncherFactory          │
│                                │
│  + createLauncher(GameType)    │
│  + detectGameType(path)        │
│  + isAppImage(path)            │
│  + isRenpyGame(path)           │
└────────────────────────────────┘
            │
            │ creates
            ↓
┌────────────────────────────────┐
│      IGameLauncher             │
│       (interface)              │
├────────────────────────────────┤
│  + validate(GameMetadata)      │
│  + launch(GameMetadata)        │
│  + isRunning()                 │
│  + killProcess()               │
│  + getProcessId()              │
└────────────────────────────────┘
            △
            │ implements
    ┌───────┴────────┬───────────┐
    │                │           │
┌───────────┐  ┌──────────┐  ┌──────────┐
│ AppImage  │  │  Ren'Py  │  │   Deb    │
│ Launcher  │  │ Launcher │  │ Launcher │
└───────────┘  └──────────┘  └──────────┘
```

## Launcher Interface

### IGameLauncher

All launchers must implement:

```cpp
class IGameLauncher {
public:
    // Identification
    virtual std::string getName() const = 0;
    virtual GameType getSupportedType() const = 0;

    // Validation
    virtual ValidationResult validate(const GameMetadata& game) = 0;

    // Execution
    virtual LaunchResult launch(const GameMetadata& game) = 0;

    // Process Management
    virtual bool isRunning() const = 0;
    virtual bool killProcess() = 0;
    virtual int getProcessId() const = 0;

    // Error Handling
    virtual std::string getLastError() const = 0;
};
```

### Supporting Structures

```cpp
struct ValidationResult {
    bool valid;
    std::string errorMessage;
};

struct LaunchResult {
    bool success;
    int exitCode;
    std::string errorMessage;
    std::string stdOutput;
    std::string stdError;
};
```

## Game Type Detection

### Auto-Detection Flow

```cpp
GameType type = GameLauncherFactory::detectGameType("/path/to/game");

switch (type) {
    case GameType::APPIMAGE:
        // .AppImage file
        break;
    case GameType::RENPY:
        // Ren'Py visual novel (has renpy/ directory)
        break;
    case GameType::DEB:
        // Debian package
        break;
    case GameType::DIRECTORY:
        // Generic game directory
        break;
    default:
        // Unknown type
        break;
}
```

### Detection Logic

**AppImage**:
- File ends with `.appimage` (case-insensitive)

**Ren'Py**:
- Directory contains `renpy/` subdirectory
- OR directory contains `game/` with `script.rpy`/`script.rpyc`

**Deb Package**:
- File ends with `.deb`

**Electron**:
- Directory contains `package.json`
- OR directory contains `resources/app.asar`

## Built-In Launchers

### AppImageLauncher

Launches AppImage games.

**Features**:
- Automatic executable permissions
- Direct execution via `execl()`
- Process monitoring
- Clean termination (SIGTERM → SIGKILL)

**Usage**:
```cpp
auto launcher = std::make_shared<AppImageLauncher>();

// Validate before launch
ValidationResult validation = launcher->validate(game);
if (!validation.valid) {
    LOG(LogError) << validation.errorMessage;
    return;
}

// Launch game
LaunchResult result = launcher->launch(game);
if (result.success) {
    LOG(LogInfo) << "Game exited with code: " << result.exitCode;
} else {
    LOG(LogError) << "Launch failed: " << result.errorMessage;
}
```

**Implementation Details**:
- Uses `chmod()` to set executable bit if needed
- Uses `fork()` + `execl()` for execution
- Parent process waits with `waitpid()`
- Captures exit code and signal information

**See**: `es-app/src/launchers/AppImageLauncher.cpp`

### RenpyLauncher

Launches Ren'Py visual novel games.

**Features**:
- Auto-detection of Ren'Py directory structure
- Executable discovery (`.sh`, `.py` files)
- Working directory management
- Script execution

**Usage**:
```cpp
auto launcher = std::make_shared<RenpyLauncher>();

LaunchResult result = launcher->launch(game);
```

**Detection**:
1. Checks for `renpy/` subdirectory
2. Looks for `game/script.rpy` or `game/script.rpyc`
3. Searches for launcher scripts (`.sh`, `.py`)

**Executable Discovery**:
Searches for in order:
- `*.sh` files
- `renpy.sh`
- `start.sh`
- `game.sh`
- `launch.sh`
- `*.py` files

**See**: `es-app/src/launchers/RenpyLauncher.cpp`

### DebLauncher

Installs and launches Debian packages.

**Features**:
- Package installation via `dpkg`
- Binary discovery in `/usr/bin`
- Package name extraction
- Installation status checking

**Usage**:
```cpp
auto launcher = std::make_shared<DebLauncher>();

// Validate (checks if dpkg is available)
ValidationResult validation = launcher->validate(game);

// Launch (installs if needed, then executes)
LaunchResult result = launcher->launch(game);
```

**Installation Flow**:
1. Extract package name from .deb file
2. Check if already installed
3. If not installed, run `dpkg -i package.deb`
4. Find installed binary in `/usr/bin` or `/usr/local/bin`
5. Execute binary

**⚠️ Note**: Installing .deb packages typically requires root privileges. In production, consider:
- Pre-installed games
- Flatpak/Snap alternatives
- User-space package managers

**See**: `es-app/src/launchers/DebLauncher.cpp`

## Using the Factory

### Creating Launchers

```cpp
// Create launcher for specific type
auto launcher = GameLauncherFactory::createLauncher(GameType::APPIMAGE);

if (launcher) {
    // Use launcher
    launcher->launch(game);
} else {
    LOG(LogError) << "No launcher available for this game type";
}
```

### Auto-Detection + Launch

```cpp
// Detect game type
GameType type = GameLauncherFactory::detectGameType(game.executablePath);

// Create appropriate launcher
auto launcher = GameLauncherFactory::createLauncher(type);

if (launcher) {
    // Validate first
    ValidationResult validation = launcher->validate(game);
    if (validation.valid) {
        // Launch
        LaunchResult result = launcher->launch(game);
    }
}
```

## Process Management

### Monitoring Running Games

```cpp
auto launcher = GameLauncherFactory::createLauncher(gameType);

// Launch in background (would need modification for async)
// Currently, launch() blocks until game exits

if (launcher->isRunning()) {
    int pid = launcher->getProcessId();
    LOG(LogInfo) << "Game running with PID: " << pid;
}
```

### Terminating Games

```cpp
if (launcher->isRunning()) {
    bool killed = launcher->killProcess();
    if (killed) {
        LOG(LogInfo) << "Game terminated";
    }
}
```

**Termination Strategy**:
1. Send `SIGTERM` (graceful shutdown)
2. Wait 2 seconds
3. If still running, send `SIGKILL` (force kill)

## Validation

### Pre-Launch Validation

Always validate before launching:

```cpp
ValidationResult result = launcher->validate(game);

if (!result.valid) {
    LOG(LogError) << "Validation failed: " << result.errorMessage;
    // Display error to user
    return;
}

// Proceed with launch
launcher->launch(game);
```

### Common Validation Checks

- **File Exists**: Executable path is valid
- **Is File**: Path is not a directory (unless expected)
- **Permissions**: File is executable
- **Extension**: Correct file extension
- **Dependencies**: Required system tools available

## Error Handling

### Launch Errors

```cpp
LaunchResult result = launcher->launch(game);

if (!result.success) {
    LOG(LogError) << "Launch failed: " << result.errorMessage;

    if (result.exitCode != 0) {
        LOG(LogError) << "Exit code: " << result.exitCode;
    }

    // Check last error for details
    std::string details = launcher->getLastError();
    LOG(LogError) << "Details: " << details;
}
```

### Process Errors

```cpp
if (WIFEXITED(status)) {
    // Normal exit
    int exitCode = WEXITSTATUS(status);
    LOG(LogInfo) << "Game exited with code: " << exitCode;
}
else if (WIFSIGNALED(status)) {
    // Terminated by signal
    int signal = WTERMSIG(status);
    LOG(LogError) << "Game terminated by signal: " << signal;
}
```

## Creating a Custom Launcher

### Step 1: Implement Interface

```cpp
#include "launchers/IGameLauncher.h"

class MyCustomLauncher : public IGameLauncher {
public:
    std::string getName() const override {
        return "My Custom Launcher";
    }

    GameType getSupportedType() const override {
        return GameType::CUSTOM;  // Add to GameType enum
    }

    ValidationResult validate(const GameMetadata& game) override {
        ValidationResult result;

        // Validation logic
        if (!Utils::FileSystem::exists(game.executablePath)) {
            result.valid = false;
            result.errorMessage = "File not found";
            return result;
        }

        result.valid = true;
        return result;
    }

    LaunchResult launch(const GameMetadata& game) override {
        LaunchResult result;

        // Launch logic
        pid_t pid = fork();

        if (pid == 0) {
            // Child process
            execl(game.executablePath.c_str(),
                  game.executablePath.c_str(),
                  (char*)nullptr);
            _exit(1);
        }
        else if (pid > 0) {
            // Parent process
            mProcessId = pid;
            int status;
            waitpid(pid, &status, 0);

            mProcessId = -1;
            result.success = WIFEXITED(status) && WEXITSTATUS(status) == 0;
            result.exitCode = WEXITSTATUS(status);
        }
        else {
            result.success = false;
            result.errorMessage = "Fork failed";
        }

        return result;
    }

    bool isRunning() const override {
        return mProcessId > 0;
    }

    bool killProcess() override {
        if (!isRunning()) return false;
        kill(mProcessId, SIGTERM);
        mProcessId = -1;
        return true;
    }

    int getProcessId() const override {
        return mProcessId;
    }

    std::string getLastError() const override {
        return mLastError;
    }

private:
    int mProcessId = -1;
    std::string mLastError;
};
```

### Step 2: Register in Factory

Modify `GameLauncherFactory::createLauncher()`:

```cpp
std::shared_ptr<IGameLauncher> GameLauncherFactory::createLauncher(GameType type) {
    switch (type) {
        case GameType::CUSTOM:
            return std::make_shared<MyCustomLauncher>();
        // ... other cases
    }
}
```

### Step 3: Add Detection Logic

Modify `GameLauncherFactory::detectGameType()`:

```cpp
GameType GameLauncherFactory::detectGameType(const std::string& path) {
    // Your detection logic
    if (isMyCustomType(path)) {
        return GameType::CUSTOM;
    }

    // ... other checks
}
```

## Best Practices

### 1. Always Validate

```cpp
ValidationResult validation = launcher->validate(game);
if (!validation.valid) {
    // Handle error
    return;
}
```

### 2. Handle Working Directory

Some games need to run from their installation directory:

```cpp
// Change to game directory before executing
std::string workingDir = Utils::FileSystem::getParent(game.executablePath);
chdir(workingDir.c_str());

// Execute game
execl(game.executablePath.c_str(), ...);
```

### 3. Capture Exit Codes

```cpp
int status;
waitpid(pid, &status, 0);

if (WIFEXITED(status)) {
    result.exitCode = WEXITSTATUS(status);
    result.success = (result.exitCode == 0);
}
```

### 4. Clean Process Termination

```cpp
// Try graceful shutdown first
kill(pid, SIGTERM);
sleep(2);

// Force kill if still running
if (kill(pid, 0) == 0) {
    kill(pid, SIGKILL);
}
```

### 5. Log Important Events

```cpp
LOG(LogInfo) << "Launching game: " << game.name;
LOG(LogDebug) << "Executable: " << game.executablePath;
LOG(LogInfo) << "Game exited with code: " << exitCode;
```

## Platform Considerations

### Linux-Specific

Current implementation uses POSIX APIs:
- `fork()` / `execl()` for process creation
- `waitpid()` for process monitoring
- `kill()` / signals for termination
- `chmod()` for permissions

### Future Windows Support

For Windows compatibility, consider:
- `CreateProcess()` instead of fork/exec
- `WaitForSingleObject()` for monitoring
- `TerminateProcess()` for killing
- Different permission model

## Performance

### Launch Time

Typical launch times:
- **AppImage**: <1 second (direct execution)
- **Ren'Py**: 1-3 seconds (script interpretation)
- **.deb**: Variable (depends on installation)

### Memory Usage

Launchers themselves are lightweight (<1MB). Memory usage comes from:
- The launched game process
- OS process management

### Optimization

- Reuse launcher instances (don't recreate for each launch)
- Cache validation results if game files don't change
- Consider async launch for non-blocking UI

## Security Considerations

### Path Validation

```cpp
// Prevent path traversal
if (path.find("..") != std::string::npos) {
    return ValidationResult{false, "Invalid path"};
}
```

### Permission Checks

```cpp
// Verify file is owned by user or trusted source
struct stat st;
stat(path.c_str(), &st);
if (st.st_uid != getuid() && st.st_uid != 0) {
    LOG(LogWarning) << "File not owned by user or root";
}
```

### Sandboxing

Consider using sandboxing for untrusted games:
- Firejail
- Bubblewrap
- Flatpak runtime

## Troubleshooting

### Game Won't Launch

1. Check validation errors
2. Verify file permissions (`ls -l`)
3. Test manual execution in terminal
4. Check logs for error details

### Game Crashes Immediately

1. Check exit code in LaunchResult
2. Run game manually to see error output
3. Verify dependencies are installed
4. Check compatibility (32-bit vs 64-bit)

### Process Won't Terminate

1. Check if process is still running (`ps aux | grep`)
2. Try manual kill (`kill -9 <pid>`)
3. Check for zombie processes
4. Verify `killProcess()` logic

## Testing

### Unit Tests

```cpp
TEST(AppImageLauncher, ValidateSuccess) {
    AppImageLauncher launcher;
    GameMetadata game;
    game.executablePath = "/path/to/valid.AppImage";

    ValidationResult result = launcher.validate(game);

    EXPECT_TRUE(result.valid);
}

TEST(AppImageLauncher, ValidateFileNotFound) {
    AppImageLauncher launcher;
    GameMetadata game;
    game.executablePath = "/invalid/path.AppImage";

    ValidationResult result = launcher.validate(game);

    EXPECT_FALSE(result.valid);
    EXPECT_FALSE(result.errorMessage.empty());
}
```

## Related Documentation

- [Launcher Creation Guide](../guides/ADDING_LAUNCHERS.md)
- [Launcher API Reference](../api/LAUNCHER_API.md)
- [Architecture Overview](../ARCHITECTURE.md)
- [Process Management](../guides/PROCESS_MANAGEMENT.md)
