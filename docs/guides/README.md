# OpenConsole User Guides

Welcome to the OpenConsole user documentation! These guides will help you get the most out of your indie gaming console.

## 🚀 Quick Start

**New to OpenConsole?** Start here:

- **[Getting Started Guide](GETTING_STARTED.md)** - Complete beginner's guide
  - First boot walkthrough
  - Adding your first games
  - Basic navigation
  - Quick troubleshooting

## 📚 Essential Guides

### Adding and Managing Games

- **[Adding Games](ADDING_GAMES.md)** - How to add games to your library
  - itch.io integration (recommended method)
  - Local file management
  - Supported file formats
  - Organizing your collection
  - USB transfer and network methods

- **[itch.io Integration](ITCHIO_INTEGRATION.md)** - Complete itch.io setup guide
  - Getting your API key
  - Authentication setup
  - Browsing and downloading games
  - Security and privacy
  - Troubleshooting

### Customization

- **[Customizing Themes](CUSTOMIZING_THEMES.md)** - Make OpenConsole yours
  - Installing existing themes
  - Popular theme recommendations
  - Creating custom themes
  - Theme troubleshooting

- **[Configuration Guide](CONFIGURATION.md)** - Advanced configuration
  - System configuration (`es_systems.cfg`)
  - Settings and preferences
  - Input configuration
  - Custom game categories

### User Interface

- **[UI Components Guide](UI_COMPONENTS.md)** - Understanding the interface
  - Navigation basics
  - Menu options
  - Virtual keyboard
  - Collections and filters

## 🔧 Technical Documentation

For developers and advanced users:

- **[Architecture Overview](../ARCHITECTURE.md)** - System design and architecture
- **[Database Layer](../architecture/DATABASE.md)** - Game metadata storage
- **[Plugin System](../architecture/PLUGINS.md)** - Extensibility and plugins
- **[Game Launchers](../architecture/LAUNCHERS.md)** - How games are launched
- **[Creating Plugins](CREATING_PLUGINS.md)** - Build your own plugins

## 📖 Reference Documentation

- **[Systems](../../SYSTEMS.md)** - System configuration reference
- **[Gamelists](../../GAMELISTS.md)** - Gamelist XML format
- **[Themes](../../THEMES.md)** - Theme XML specification
- **[Changelog](../CHANGELOG.md)** - Version history and changes

## 🎯 Common Tasks

### I want to...

**Add games from itch.io**
→ See [itch.io Integration Guide](ITCHIO_INTEGRATION.md)

**Add games from my computer**
→ See [Adding Games - Local Files](ADDING_GAMES.md#local-game-files)

**Change how OpenConsole looks**
→ See [Customizing Themes](CUSTOMIZING_THEMES.md)

**Set up a gamepad/controller**
→ See [Configuration Guide - Input](CONFIGURATION.md)

**Organize games into categories**
→ See [UI Components - Collections](UI_COMPONENTS.md)

**Add emulators (optional)**
→ See [Getting Started - About Emulation](GETTING_STARTED.md#about-emulation)

**Fix games that won't launch**
→ See [Adding Games - Troubleshooting](ADDING_GAMES.md#troubleshooting)

## 💡 Tips and Tricks

### Controller Navigation

- **D-Pad/Left Stick**: Navigate menus and game lists
- **A Button**: Select/Confirm
- **B Button**: Back/Cancel
- **X Button**: Random game (in game list)
- **Y Button**: Add to favorites
- **Start**: Main menu
- **Select**: View options/filters

### Keyboard Shortcuts

- **F1**: Menu
- **F4**: Reload games
- **Ctrl+R**: Restart OpenConsole
- **Ctrl+Q**: Quit
- **Ctrl+F**: Search

### Performance Tips

1. **Use lightweight themes** for faster navigation
2. **Disable video previews** if running slow
3. **Limit game library size** or use collections
4. **Enable recursive scanning** only if needed

### Organization Tips

1. **Use itch.io for downloads** - Automatic organization
2. **Create subdirectories** by genre/type
3. **Use collections** to group favorites
4. **Name files descriptively** - Makes searching easier
5. **Add metadata** via scraper for better browsing

## ❓ Getting Help

### First Steps

1. **Check the logs:**
   ```bash
   tail -f ~/.openconsole/es_log.txt
   ```

2. **Try safe mode:**
   - Restart with default theme
   - Disable plugins temporarily

3. **Search existing issues:**
   - [GitHub Issues](https://github.com/aldoram5/OpenConsole/issues)

### Community Support

- 💬 **Discussions:** [GitHub Discussions](https://github.com/aldoram5/OpenConsole/discussions)
- 🐛 **Bug Reports:** [GitHub Issues](https://github.com/aldoram5/OpenConsole/issues)
- 📧 **Contact:** See [project README](../../README.md)

### Contributing

Found a problem with the docs? Want to add a guide?

1. **Fork the repository**
2. **Edit the markdown files** in `docs/guides/`
3. **Submit a pull request**

We appreciate clear, beginner-friendly documentation!

## 🎮 What is OpenConsole?

OpenConsole is an **open-source gaming console OS** designed specifically for **indie games**:

### ✨ Key Features

- **🎮 Controller-First Design** - Living room gaming without keyboard/mouse
- **🌐 itch.io Integration** - Download and play your itch.io library
- **📦 Multiple Formats** - AppImage, Ren'Py, Electron, native Linux
- **🎨 Themeable** - Customize appearance with themes
- **🔌 Plugin System** - Extensible game source plugins
- **💾 Database Storage** - Fast game library with metadata
- **🔐 Secure** - AES-256 encrypted credential storage

### 🎯 Perfect For:

- Indie game enthusiasts
- Raspberry Pi gaming setups
- Living room consoles
- Controller-only gaming
- itch.io library management

### ❌ Not For:

- Emulation-focused setups (use RetroPie/Batocera instead)
- AAA/Steam gaming (use Steam Big Picture)
- Windows/commercial games

## 📋 Documentation Index

**Getting Started**
- [Getting Started Guide](GETTING_STARTED.md)
- [Adding Games](ADDING_GAMES.md)
- [itch.io Integration](ITCHIO_INTEGRATION.md)

**Customization**
- [Customizing Themes](CUSTOMIZING_THEMES.md)
- [Configuration Guide](CONFIGURATION.md)
- [UI Components](UI_COMPONENTS.md)

**Development**
- [Architecture](../ARCHITECTURE.md)
- [Creating Plugins](CREATING_PLUGINS.md)
- [Database Layer](../architecture/DATABASE.md)
- [Changelog](../CHANGELOG.md)

**Reference**
- [Systems Config](../../SYSTEMS.md)
- [Gamelists Format](../../GAMELISTS.md)
- [Theme Specification](../../THEMES.md)

---

**Ready to start?** Head to the [Getting Started Guide](GETTING_STARTED.md)!
