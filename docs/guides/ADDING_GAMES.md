# Adding Games to OpenConsole

This guide covers all the ways you can add and organize games in OpenConsole.

## Table of Contents

1. [itch.io Games (Recommended)](#itchio-games-recommended)
2. [Local Game Files](#local-game-files)
3. [Supported File Formats](#supported-file-formats)
4. [Organizing Your Library](#organizing-your-library)
5. [Making Games Executable](#making-games-executable)
6. [Troubleshooting](#troubleshooting)

## itch.io Games (Recommended)

The easiest way to get games is through the built-in itch.io integration.

### Setup itch.io (One-Time)

1. **Get Your API Key**
   - Visit: https://itch.io/user/settings/api-keys
   - Click **"Generate new API key"**
   - Copy the entire key

2. **Configure in OpenConsole**
   - Main Menu → **OpenConsole Settings**
   - Navigate to **Plugins** section
   - Select **itch.io Authentication** → **CONFIGURE**
   - Enter your API key using the virtual keyboard

3. **Browse Your Library**
   - Your itch.io games will now appear in OpenConsole
   - Download button appears for games you haven't installed yet

### Download Games

1. Select a game from your itch.io library
2. Press **A** to view details
3. Select **"Download"**
4. Wait for download and installation
5. Game is ready to play!

Downloaded games are saved to: `~/Games/itchio/`

📖 **Full Guide:** [itch.io Integration](ITCHIO_INTEGRATION.md)

## Local Game Files

You can add games manually by copying them to your OpenConsole system.

### Directory Structure

OpenConsole scans these directories by default:

```
~/Games/
  ├── indie/          # General indie games
  ├── visualnovels/   # Ren'Py visual novels
  ├── itchio/         # itch.io downloads (auto-created)
  └── games/          # Catch-all directory
```

### Adding Games

#### Method 1: Direct Copy (Local Access)

If you have access to the OpenConsole terminal/SSH:

```bash
# Example: Add an AppImage game
cp /path/to/your/game.AppImage ~/Games/indie/

# Make it executable
chmod +x ~/Games/indie/game.AppImage
```

#### Method 2: USB Transfer (Raspberry Pi)

1. **On Your Computer:**
   - Format USB drive as FAT32 or exFAT
   - Create folder: `Games/indie/`
   - Copy game files into this folder

2. **On OpenConsole:**
   - Plug in USB drive
   - Use file manager or terminal to copy files:
     ```bash
     cp /media/usb/Games/indie/* ~/Games/indie/
     chmod +x ~/Games/indie/*.AppImage
     ```

3. **Reload Games:**
   - Main Menu → **Reload**
   - Or restart OpenConsole

#### Method 3: Network Transfer

Using SCP from your computer:

```bash
# Replace 'openconsole' with your device's hostname/IP
scp game.AppImage openconsole@openconsole:~/Games/indie/
```

## Supported File Formats

### Native Linux Games

| Format | Description | Example |
|--------|-------------|---------|
| `.AppImage` | Self-contained Linux apps | `celeste.AppImage` |
| `.sh` | Shell script launchers | `game.sh` |
| `.x86_64` | 64-bit Linux executable | `SuperMeatBoy.x86_64` |
| `.x86` | 32-bit Linux executable | `game.x86` |

### Visual Novels

| Format | Description | Example |
|--------|-------------|---------|
| `.py` | Python/Ren'Py games | `visualnovel.py` |
| `.sh` | Ren'Py launcher scripts | `start.sh` |

### What About ZIP files?

OpenConsole **does not automatically extract ZIP files**. You must extract them first:

```bash
# Extract a game archive
unzip game.zip -d ~/Games/indie/GameName/

# Make the main executable file executable
cd ~/Games/indie/GameName/
chmod +x game.sh
```

## Organizing Your Library

### By Category

Create subdirectories to organize by genre or type:

```
~/Games/
  ├── indie/
  │   ├── platformers/
  │   ├── puzzles/
  │   └── roguelikes/
  ├── visualnovels/
  │   ├── horror/
  │   └── romance/
  └── itchio/
```

> **Note:** If you use subdirectories, make sure "Recursive Scanning" is enabled in OpenConsole Settings → Plugins → Local Filesystem.

### By Platform/Source

Separate games by where they came from:

```
~/Games/
  ├── itchio/      # Games from itch.io
  ├── steam/       # Steam games (if you set that up)
  ├── gamejolt/    # GameJolt downloads
  └── indie/       # Other sources
```

### Using Collections

OpenConsole has a built-in collections feature:

1. **Create Collection**
   - Main Menu → **Game Collection Settings**
   - Create custom collections (Favorites, Platformers, etc.)

2. **Add Games to Collection**
   - Select a game
   - Press **Y** to add to favorites
   - Or edit metadata to assign categories

## Making Games Executable

On Linux, files need the "executable" permission to run. If a game doesn't launch, it probably needs this permission.

### Quick Fix

```bash
# Make a single file executable
chmod +x ~/Games/indie/game.AppImage

# Make all .AppImage files in a directory executable
chmod +x ~/Games/indie/*.AppImage

# Make all .sh scripts executable
chmod +x ~/Games/**/*.sh
```

### Check Permissions

```bash
# List files with permissions
ls -lh ~/Games/indie/

# Look for files starting with -rwxr-xr-x (executable)
# Files starting with -rw-r--r-- need chmod +x
```

### Why This Happens

When you download or copy files, Linux doesn't automatically mark them as executable for security reasons. You have to do it manually.

## Troubleshooting

### Games Don't Appear

**Check file extensions:**
```bash
# See all files in directory
ls -la ~/Games/indie/

# Make sure extensions match config
# Default: .AppImage, .appimage, .sh, .x86_64, .x86
```

**Verify directory paths:**
```bash
# Check that Games directory exists
ls -la ~/Games/

# It should show: indie/, visualnovels/, etc.
```

**Check recursive scanning:**
- If games are in subdirectories, enable recursive scanning:
  - Main Menu → OpenConsole Settings → Plugins
  - Local Filesystem → Recursive Scanning → ON

### Game Won't Launch

**Not executable:**
```bash
chmod +x ~/Games/indie/game.AppImage
```

**Wrong file:**
- Some games extract to a folder with multiple files
- You need to run the main executable (usually `game.sh` or `start.sh`)
- Edit `es_systems.cfg` if needed to point to the right file

**Missing dependencies:**
- Some games need additional libraries
- Check the game's itch.io page or README for requirements
- Install with: `sudo apt install <library-name>`

### Games Launch But Don't Work

**Check game logs:**
- Most games create log files in their directory
- Look for errors in: `~/.openconsole/es_log.txt`

**Verify game integrity:**
- Re-download the game
- Check if it works outside OpenConsole first
- Some games may not be compatible with your platform

### Database Issues

**Rescan games:**
```bash
# Delete database and rescan
rm ~/.openconsole/games.db
# Restart OpenConsole
```

**View database stats:**
- Main Menu → OpenConsole Settings → Database
- View Database Statistics

## Advanced: Custom Systems

You can add custom "systems" (categories) by editing the config file.

**Edit config:**
```bash
nano ~/.openconsole/es_systems.cfg
```

**Add a new system:**
```xml
<system>
    <name>horror</name>
    <fullname>Horror Games</fullname>
    <path>~/Games/horror</path>
    <extension>.AppImage .sh .x86_64</extension>
    <command>%ROM%</command>
    <platform>pc</platform>
    <theme>pc</theme>
</system>
```

**Restart OpenConsole** for changes to take effect.

## Best Practices

### ✅ Do:
- Organize games into subdirectories
- Use descriptive folder names
- Keep itch.io games in the `itchio/` folder
- Make files executable before adding
- Use the itch.io integration when possible

### ❌ Don't:
- Mix different game types in one folder
- Use spaces in filenames (use underscores: `my_game.sh`)
- Forget to extract ZIP files first
- Store games in system directories (use `~/Games/`)

## Examples

### Adding a Ren'Py Visual Novel

```bash
# 1. Download and extract
unzip visualnovel.zip -d ~/Games/visualnovels/MyNovel/

# 2. Find the launcher script
cd ~/Games/visualnovels/MyNovel/
ls -la
# Look for: MyNovel.sh or start.sh

# 3. Make it executable
chmod +x MyNovel.sh

# 4. Reload OpenConsole
# Main Menu → Reload
```

### Adding an AppImage

```bash
# 1. Copy the file
cp celeste.AppImage ~/Games/indie/

# 2. Make executable
chmod +x ~/Games/indie/celeste.AppImage

# 3. Reload OpenConsole
```

### Adding Multiple Games at Once

```bash
# Copy all AppImages from Downloads
cp ~/Downloads/*.AppImage ~/Games/indie/

# Make them all executable
chmod +x ~/Games/indie/*.AppImage

# Reload OpenConsole
```

## Need Help?

- 📖 **Configuration Guide**: [CONFIGURATION.md](CONFIGURATION.md)
- 🎨 **itch.io Integration**: [ITCHIO_INTEGRATION.md](ITCHIO_INTEGRATION.md)
- 🐛 **Report Issues**: [GitHub Issues](https://github.com/aldoram5/OpenConsole/issues)

---

**Next:** Learn how to [customize themes](THEMES.md) to make OpenConsole look amazing!
