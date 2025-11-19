# Getting Started with OpenConsole

Welcome to OpenConsole! This guide will help you set up your indie gaming console and start playing games.

## What is OpenConsole?

OpenConsole is a **controller-friendly gaming interface** designed for indie games. It's perfect for:

- 🎮 **Living room gaming** - Control everything with just a controller
- 🎨 **Indie games** - Built specifically for modern indie titles
- 🌐 **itch.io integration** - Download and play your itch.io library
- 📦 **Multiple formats** - AppImage, Ren'Py, native Linux games, and more

> **Note:** OpenConsole is NOT primarily an emulation frontend. While you *can* add emulators if you want, it's designed for modern indie games.

## Quick Start

### 1. First Boot

When you first boot OpenConsole, you'll see the main interface even if you have no games installed. This is normal!

The system has already created a default configuration for you at `~/.openconsole/es_systems.cfg`.

### 2. Add Games (Two Ways)

#### Option A: Use itch.io Integration (Recommended)

1. **Open Settings**
   - Press **Start** on your controller
   - Navigate to **"OPENCONSOLE SETTINGS"**

2. **Configure itch.io**
   - Scroll to the **Plugins** section
   - Select **"itch.io Authentication"** → **"CONFIGURE"**

3. **Get Your API Key**
   - On any device with a web browser, visit: https://itch.io/user/settings/api-keys
   - Click **"Generate new API key"**
   - Copy the key

4. **Enter API Key**
   - Back in OpenConsole, select **"ENTER API KEY"**
   - Use the on-screen keyboard to type your key:
     - **D-pad**: Navigate keyboard
     - **A**: Select character
     - **B**: Backspace
     - **Start**: Finish

5. **Browse Your Library**
   - Once authenticated, your itch.io games will appear
   - Download games directly through OpenConsole!

📖 **Detailed Guide:** See [itch.io Integration Guide](ITCHIO_INTEGRATION.md) for complete instructions.

#### Option B: Add Local Games

Simply copy game files to the `~/Games/` directory:

```bash
~/Games/
  ├── indie/          # AppImage games, Linux executables
  ├── visualnovels/   # Ren'Py games
  ├── itchio/         # itch.io downloads
  └── games/          # Any other games
```

**Supported Formats:**
- `.AppImage` - Portable Linux applications
- `.sh` - Shell scripts (like Ren'Py launchers)
- `.x86_64`, `.x86` - Native Linux executables
- `.py` - Python-based games

After adding files, restart OpenConsole or select **"Reload"** from the menu.

📖 **Detailed Guide:** See [Adding Games Guide](ADDING_GAMES.md)

### 3. Navigate the Interface

**Controller Basics:**
- **D-Pad/Left Stick**: Navigate menus
- **A Button**: Select
- **B Button**: Back/Cancel
- **Start**: Open main menu
- **Select**: Open options (when available)

**Main Menu Options:**
- **Scraper**: Download game metadata/artwork
- **UI Settings**: Customize appearance
- **Sound Settings**: Adjust audio
- **OpenConsole Settings**: Configure plugins, database
- **Quit**: Exit or restart

### 4. Customize Your Experience

**Change Themes:**
Themes control how OpenConsole looks. See the [Themes Guide](THEMES.md) for customization options.

**Adjust Settings:**
- **Sort games**: By name, last played, rating, etc.
- **Configure screensaver**: Slideshows, video previews
- **Set up collections**: Group games by genre, favorites, etc.

## Next Steps

### Add More Games

- **From itch.io**: Download directly through OpenConsole
- **Local files**: Copy to `~/Games/` directories
- **USB drives**: Plug in and they'll be scanned automatically (if enabled)

### Customize Look & Feel

- **Install themes**: Download EmulationStation themes (they're compatible!)
- **Add artwork**: Scrape metadata for cover art and descriptions
- **Create collections**: Custom game groupings

### Advanced Configuration

Edit `~/.openconsole/es_systems.cfg` to:
- Add new game directories
- Change file extension filters
- Customize launch commands
- Add custom systems

📖 **See:** [Configuration Guide](CONFIGURATION.md)

## Troubleshooting

### "No games found"

This is normal on first boot! Add games using one of the methods above.

### Games don't appear after adding them

1. Make sure they're in the correct directory (`~/Games/`)
2. Check file extensions match the config (`.AppImage`, `.sh`, etc.)
3. Try **"Reload"** from the main menu
4. Restart OpenConsole

### itch.io authentication fails

1. Make sure you copied the entire API key
2. Check your internet connection
3. Verify the key at: https://itch.io/user/settings/api-keys

### Can't navigate with controller

1. Make sure your controller is plugged in before starting OpenConsole
2. Check **"Input Configuration"** in the main menu
3. Some controllers may need manual configuration

## Need More Help?

- 📖 **Full Documentation**: [OpenConsole Docs](https://github.com/aldoram5/OpenConsole/tree/main/docs)
- 🐛 **Report Issues**: [GitHub Issues](https://github.com/aldoram5/OpenConsole/issues)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/aldoram5/OpenConsole/discussions)

## About Emulation

While OpenConsole *can* be used with emulators (like RetroArch), **this is not its primary purpose**.

If you want to add emulation support:
1. Install the emulator manually (e.g., `sudo apt install retroarch`)
2. Install the cores/games you need
3. Edit `~/.openconsole/es_systems.cfg` to add emulator systems
4. See the commented examples at the bottom of the default config

**We recommend using RetroPie or Batocera if emulation is your main focus.**

---

**Ready to play?** Check out the [Adding Games Guide](ADDING_GAMES.md) next!
