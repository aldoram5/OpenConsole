# OpenConsole

OpenConsole is an open-source gaming console operating system designed for modern indie games. It provides a controller-friendly interface for launching and managing games from multiple sources, with native support for local games and itch.io integration.

**Forked from:** [EmulationStation](https://github.com/RetroPie/EmulationStation)

## Features

- **Multi-Source Game Library**: Scan local directories, itch.io library, and USB drives
- **Multiple Game Formats**: AppImage, Ren'Py, .deb packages, Electron apps, and more
- **SQLite Database**: Fast, reliable game metadata storage with play statistics
- **Plugin Architecture**: Extensible system for adding new game sources
- **Secure Credential Storage**: AES-256 encrypted storage for API tokens
- **Controller-First UI**: Designed for living room gaming without keyboard/mouse
- **Virtual Keyboard**: On-screen keyboard with full controller navigation
- **itch.io Integration**: Browse and download games from your itch.io library
- **Auto-Boot Support**: Boot directly into OpenConsole on Raspberry Pi
- **Theme Support**: Compatible with existing EmulationStation themes

## Target Platform

- **Primary Target**: Raspberry Pi 5
- **Supported**: Raspberry Pi 4, Generic Linux x86_64
- **Experimental**: Windows, macOS

## Documentation

Comprehensive documentation is available in the [docs/](docs/) directory:

- [Architecture Overview](docs/ARCHITECTURE.md)
- [Database Layer](docs/architecture/DATABASE.md)
- [Plugin System](docs/architecture/PLUGINS.md)
- [Game Launcher System](docs/architecture/LAUNCHERS.md)
- [UI Components Guide](docs/guides/UI_COMPONENTS.md)
- [itch.io Integration Guide](docs/guides/ITCHIO_INTEGRATION.md)
- [Creating Plugins](docs/guides/CREATING_PLUGINS.md)
- [Configuration Guide](docs/guides/CONFIGURATION.md)
- [System Integration](system/README.md)
- [Changelog](docs/CHANGELOG.md)

## Quick Start

### Pre-built Raspberry Pi Image (Recommended)

The easiest way to get started is with a pre-built image:

```bash
# Download the latest image
wget https://github.com/aldoram5/OpenConsole/releases/latest/download/openconsole-pi5.img.xz

# Flash to SD card (replace /dev/sdX with your SD card device)
xzcat openconsole-pi5.img.xz | sudo dd of=/dev/sdX bs=4M status=progress
sync
```

Insert the SD card into your Raspberry Pi 5 and power on. The system will boot directly into OpenConsole.

### Automated Installation (Linux)

For installation on existing Linux systems:

```bash
# Clone the repository
git clone --recursive https://github.com/aldoram5/OpenConsole.git
cd OpenConsole

# Run the installer (standard mode)
sudo ./install.sh

# Or for console appliance mode (auto-boot to OpenConsole)
sudo ./install.sh --console-mode
```

The installer will:
- Install all dependencies
- Build OpenConsole from source
- Create the openconsole system user
- Configure the systemd service
- Set up controller support

## Building from Source

OpenConsole uses C++11 and requires CMake for the build process.

### Dependencies

**Core Dependencies:**
- CMake (3.10+)
- SDL2
- FreeImage
- FreeType
- libVLC (3.0+)
- libCURL
- SQLite3
- OpenSSL
- RapidJSON
- Boost (filesystem, system, locale)

### Building on Debian/Ubuntu

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake git \
    libsdl2-dev libfreeimage-dev libfreetype6-dev \
    libcurl4-openssl-dev rapidjson-dev libasound2-dev \
    libgl1-mesa-dev libssl-dev libsqlite3-dev \
    libvlc-dev libvlccore-dev vlc-bin \
    libboost-system-dev libboost-filesystem-dev libboost-locale-dev \
    fonts-droid-fallback

# Clone repository with submodules
git clone --recursive https://github.com/aldoram5/OpenConsole.git
cd OpenConsole

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make -j$(nproc)

# The binary will be at build/emulationstation
```

### Building on Fedora

```bash
# Install dependencies (enable RPM Fusion for VLC)
sudo dnf install -y \
    cmake gcc-c++ git \
    SDL2-devel freeimage-devel freetype-devel \
    libcurl-devel rapidjson-devel alsa-lib-devel \
    mesa-libGL-devel openssl-devel sqlite-devel \
    vlc-devel boost-devel

# Clone and build
git clone --recursive https://github.com/aldoram5/OpenConsole.git
cd OpenConsole
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Building on macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake sdl2 freeimage freetype curl openssl sqlite boost rapidjson pugixml libvlc

# Clone repository with submodules
git clone --recursive https://github.com/aldoram5/OpenConsole.git
cd OpenConsole

# If you already cloned without --recursive, initialize submodules:
# git submodule update --init --recursive

# Create build directory
mkdir build && cd build

# Configure with Homebrew paths
cmake .. \
    -DOPENSSL_ROOT_DIR=$(brew --prefix openssl) \
    -DFREETYPE_INCLUDE_DIRS=$(brew --prefix freetype)/include/freetype2 \
    -DSDL2_INCLUDE_DIR=$(brew --prefix sdl2)/include/SDL2

# Build
make -j$(sysctl -n hw.ncpu)
```

**macOS Build Notes:**
- The `--recursive` flag is important to initialize git submodules (nanosvg, pugixml). If you skip this, you must run `git submodule update --init --recursive` separately.
- If pugixml submodule is not initialized, the build system will look for it via Homebrew (`brew install pugixml`).
- `libvlc` provides the VLC development libraries needed for video support (different from the VLC.app cask).
- On Apple Silicon Macs, Homebrew libraries are in `/opt/homebrew/`, while Intel Macs use `/usr/local/`. The build system automatically searches both locations.

### Building on Raspberry Pi

For Raspberry Pi OS (Bullseye or later), use Mesa GLES:

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake git \
    libsdl2-dev libfreeimage-dev libfreetype6-dev \
    libcurl4-openssl-dev rapidjson-dev libasound2-dev \
    libgles2-mesa-dev libssl-dev libsqlite3-dev \
    libvlc-dev libvlccore-dev vlc-bin \
    libboost-system-dev libboost-filesystem-dev libboost-locale-dev \
    fonts-droid-fallback

# Clone and build
git clone --recursive https://github.com/aldoram5/OpenConsole.git
cd OpenConsole
mkdir build && cd build

# Configure with Mesa GLES and Pi optimizations
cmake .. -DUSE_MESA_GLES=On -DRPI=On

# Build (use fewer jobs on Pi to avoid memory issues)
make -j2
```

### Building on Windows

See [the original EmulationStation Windows build instructions](#building-on-windows-legacy) for Windows-specific setup. Windows support is experimental and may require additional configuration for OpenConsole-specific features.

### Build Options

Common CMake options:

- `-DCMAKE_BUILD_TYPE=Debug` - Build with debug symbols
- `-DCMAKE_BUILD_TYPE=Release` - Optimized release build
- `-DUSE_MESA_GLES=On` - Use Mesa GLES (required for Pi 4/5)
- `-DRPI=On` - Enable Raspberry Pi optimizations
- `-DUSE_GLES1=On` - Use GLESv1 legacy renderer

## Running OpenConsole

### First Run

```bash
# Run OpenConsole (from build directory)
./emulationstation

# Or if installed system-wide
openconsole
```

On first run, OpenConsole will:
1. Create configuration directories (`~/.emulationstation/`, `~/.openconsole/`)
2. Initialize the SQLite database
3. Prompt for controller configuration
4. Display the main menu

### Configuration Directories

```
~/.emulationstation/     # EmulationStation configuration
├── es_settings.cfg      # User settings
├── es_input.cfg         # Controller mappings
└── es_log.txt           # Application log

~/.openconsole/          # OpenConsole-specific data
├── games.db             # Game database
├── credentials.enc      # Encrypted API tokens
├── downloads/           # Downloaded games
└── config.xml           # OpenConsole settings
```

### Controller Setup

1. Start OpenConsole
2. Hold a button on your controller when prompted
3. Map the buttons as instructed
4. Select "SAVE" to save the configuration

For advanced controller setup, use the provided utility:

```bash
./system/configure-controllers.sh
```

### Setting Up itch.io Integration

1. Open OpenConsole Settings (from main menu)
2. Navigate to "Plugins" section
3. Select "itch.io Authentication" -> "CONFIGURE"
4. Follow the instructions to obtain your API key from https://itch.io/user/settings/api-keys
5. Enter the API key using the virtual keyboard
6. Authentication will be tested automatically

### Scanning for Games

1. Open OpenConsole Settings
2. Navigate to "Scanning" section
3. Select "Scan for Games Now"
4. OpenConsole will scan:
   - Local directories (~/Games, ~/.local/share/games, etc.)
   - itch.io library (if authenticated)
   - USB drives (if configured)

## System Administration

### Systemd Service Management

```bash
# Start OpenConsole
sudo systemctl start openconsole

# Stop OpenConsole
sudo systemctl stop openconsole

# Enable auto-start on boot
sudo systemctl enable openconsole

# View logs
sudo journalctl -u openconsole -f

# Check status
sudo systemctl status openconsole
```

### Building a Raspberry Pi Image

Create a distributable image with OpenConsole pre-installed. There are multiple ways to build the image:

#### Option 1: Download Pre-built Release (Recommended)

The easiest option is to use the automated GitHub Actions releases:

```bash
# Download latest release
wget https://github.com/aldoram5/OpenConsole/releases/latest/download/openconsole-pi5-<version>.img.xz

# Verify checksum
wget https://github.com/aldoram5/OpenConsole/releases/latest/download/openconsole-pi5-<version>.img.xz.sha256
sha256sum -c openconsole-pi5-<version>.img.xz.sha256
```

Releases are automatically created when you push a version tag:

```bash
git tag v1.0.0
git push origin v1.0.0
# GitHub Actions will build and publish the image
```

#### Option 2: Build with Docker (macOS/Windows)

For local builds on non-Linux systems, use the Docker-based script:

```bash
# Requires Docker Desktop installed and running
./system/build-pi-image-docker.sh

# Output will be in ./output/openconsole-pi5-<date>.img.xz
```

This runs the build process inside a Linux container, so it works on macOS, Windows (WSL), and Linux.

#### Option 3: Build on Linux Directly

On a Linux system (Debian/Ubuntu), you can run the build script directly:

```bash
# Install dependencies
sudo apt-get install -y wget xz-utils kpartx qemu-user-static parted

# Build the image
cd system
sudo ./build-pi-image.sh
```

This creates a compressed image file (`openconsole-pi5-YYYYMMDD.img.xz`) ready for flashing.

#### Flashing the Image

```bash
# Linux
xzcat openconsole-pi5-*.img.xz | sudo dd of=/dev/sdX bs=4M status=progress

# macOS (using Raspberry Pi Imager)
# 1. Open Raspberry Pi Imager
# 2. Choose OS → Use custom → select the .img.xz file
# 3. Choose Storage → select your SD card
# 4. Write
```

#### Creating a New Release

To create a new release with GitHub Actions:

1. Tag your version:
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```

2. GitHub Actions will automatically:
   - Build the Raspberry Pi image
   - Create a GitHub Release
   - Attach the image file with checksums

3. Monitor progress at: `https://github.com/aldoram5/OpenConsole/actions`

## Development

### Project Structure

```
OpenConsole/
├── es-core/           # Core EmulationStation components
├── es-app/            # Application-specific code
│   ├── src/
│   │   ├── db/        # Database layer (SQLite)
│   │   ├── plugins/   # Game source plugins
│   │   ├── launchers/ # Game launchers
│   │   ├── api/       # External API clients
│   │   ├── utils/     # Utility classes
│   │   └── guis/      # GUI components
├── docs/              # Documentation
├── system/            # System integration files
└── CMake/             # CMake modules
```

### Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests (when available)
5. Submit a pull request

See the [documentation](docs/) for architecture details and development guides.

## Troubleshooting

### OpenConsole Won't Start

```bash
# Check logs
tail -f ~/.emulationstation/es_log.txt

# Check systemd service (if using)
sudo systemctl status openconsole
sudo journalctl -u openconsole -n 50
```

### Controller Not Detected

```bash
# List input devices
ls -l /dev/input/

# Check user groups
groups $(whoami)
# Should include: input video audio

# Add to input group if missing
sudo usermod -a -G input $(whoami)
# Log out and back in for changes to take effect
```

### Database Issues

```bash
# View database statistics in OpenConsole Settings
# Or manually check the database
sqlite3 ~/.openconsole/games.db ".tables"
sqlite3 ~/.openconsole/games.db "SELECT COUNT(*) FROM games;"
```

For more troubleshooting information, see:
- [UI Components Guide](docs/guides/UI_COMPONENTS.md#troubleshooting)
- [itch.io Integration Guide](docs/guides/ITCHIO_INTEGRATION.md#troubleshooting)
- [System Integration Guide](system/README.md#troubleshooting)

## Credits

OpenConsole is built on top of [EmulationStation](https://github.com/RetroPie/EmulationStation) by Alec "Aloshi" Lofquist.

## License

OpenConsole is licensed under the same license as EmulationStation. See [LICENSE.md](LICENSE.md) for details.

## Links

- **Repository**: https://github.com/aldoram5/OpenConsole
- **Issues**: https://github.com/aldoram5/OpenConsole/issues
- **Documentation**: [docs/](docs/)
- **Original EmulationStation**: https://github.com/RetroPie/EmulationStation

---

## Legacy Documentation

The following sections are retained from the original EmulationStation documentation for reference.

<details>
<summary>Building on Windows (Legacy)</summary>

### Building on Windows (Legacy)

* Install [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/). At a minimum, install the "Desktop development with C++" workload with the default list of optional items.

* Install the latest version of [CMake](https://cmake.org/download/).

* Use git to clone [vcpkg](https://vcpkg.io/en/), then run the bootstrap script:

```batchfile
C:\src>git clone https://github.com/Microsoft/vcpkg.git
C:\src>.\vcpkg\bootstrap-vcpkg.bat
```

* Download [nuget.exe](https://dist.nuget.org/win-x86-commandline/latest/nuget.exe) to a folder in your PATH.

* Use NuGet to download libVLC:

```batchfile
C:\src\OpenConsole>mkdir nuget
C:\src\OpenConsole>cd nuget
C:\src\OpenConsole\nuget>nuget install -ExcludeVersion VideoLAN.LibVLC.Windows
```

* Use vcpkg to install dependencies:

```batchfile
c:\src>.\vcpkg\vcpkg install curl:x86-windows-static-md
c:\src>.\vcpkg\vcpkg install freeimage:x86-windows-static-md
c:\src>.\vcpkg\vcpkg install freetype:x86-windows-static-md
c:\src>.\vcpkg\vcpkg install sdl2:x86-windows-static-md
c:\src>.\vcpkg\vcpkg install rapidjson:x86-windows-static-md
c:\src>.\vcpkg\vcpkg install openssl:x86-windows-static-md
c:\src>.\vcpkg\vcpkg install sqlite3:x86-windows-static-md
c:\src>.\vcpkg\vcpkg install boost-filesystem:x86-windows-static-md
c:\src>.\vcpkg\vcpkg install boost-system:x86-windows-static-md
c:\src>.\vcpkg\vcpkg install boost-locale:x86-windows-static-md
```

* Configure environment variables and generate the Visual Studio project with CMake. See the original EmulationStation documentation for specific paths.

</details>

<details>
<summary>EmulationStation Configuration (Legacy)</summary>

### EmulationStation Configuration

OpenConsole maintains compatibility with EmulationStation configuration files for themes and input.

**~/.emulationstation/es_input.cfg:**
Controller configuration is stored here. You can reconfigure at any time by pressing Start and choosing "CONFIGURE INPUT".

**~/.emulationstation/es_systems.cfg:**
While OpenConsole uses a database for game storage, it still supports the EmulationStation systems configuration format for backward compatibility.

See [GAMELISTS.md](GAMELISTS.md) and [THEMES.md](THEMES.md) for legacy configuration documentation.

</details>
