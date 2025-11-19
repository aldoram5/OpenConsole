---
layout: default
title: Home
---

# Welcome to OpenConsole

<p class="lead">An open-source gaming console OS designed for indie games, with native itch.io integration and controller-first navigation.</p>

---

## 🚀 Quick Start

<div class="quick-links">
  <a href="guides/GETTING_STARTED" class="btn">
    <strong>Getting Started</strong><br>
    <small>New user? Start here!</small>
  </a>

  <a href="guides/ADDING_GAMES" class="btn">
    <strong>Adding Games</strong><br>
    <small>itch.io & local files</small>
  </a>

  <a href="guides/CUSTOMIZING_THEMES" class="btn">
    <strong>Customize Themes</strong><br>
    <small>Make it yours</small>
  </a>
</div>

---

## 📚 User Guides

### Getting Started
- **[Getting Started Guide](guides/GETTING_STARTED)** - Complete beginner's walkthrough
- **[Adding Games](guides/ADDING_GAMES)** - How to add games from itch.io and local files
- **[itch.io Integration](guides/ITCHIO_INTEGRATION)** - Complete itch.io setup guide

### Customization
- **[Customizing Themes](guides/CUSTOMIZING_THEMES)** - Install and create themes
- **[Configuration Guide](guides/CONFIGURATION)** - Advanced configuration options
- **[UI Components](guides/UI_COMPONENTS)** - Understanding the interface

### All Guides
📖 **[Complete Guide Index](guides/)** - Browse all documentation

---

## 🎮 What is OpenConsole?

OpenConsole is a **controller-friendly gaming interface** for indie games:

<div class="features">
  <div class="feature">
    <h3>🌐 itch.io Integration</h3>
    <p>Browse and download your itch.io library directly through OpenConsole</p>
  </div>

  <div class="feature">
    <h3>🎮 Controller-First</h3>
    <p>Navigate everything with just a controller - perfect for living room gaming</p>
  </div>

  <div class="feature">
    <h3>📦 Multiple Formats</h3>
    <p>AppImage, Ren'Py, native Linux executables, and more</p>
  </div>

  <div class="feature">
    <h3>🎨 Themeable</h3>
    <p>Customize appearance with EmulationStation-compatible themes</p>
  </div>

  <div class="feature">
    <h3>🔌 Extensible</h3>
    <p>Plugin architecture for adding new game sources</p>
  </div>

  <div class="feature">
    <h3>🔐 Secure</h3>
    <p>AES-256 encrypted credential storage</p>
  </div>
</div>

---

## 🎯 Perfect For

- ✅ Indie game enthusiasts
- ✅ Raspberry Pi gaming setups
- ✅ Living room consoles
- ✅ Controller-only gaming
- ✅ itch.io library management

## ❌ Not For

- ❌ Emulation-focused setups (use [RetroPie](https://retropie.org.uk/) or [Batocera](https://batocera.org/) instead)
- ❌ AAA/Steam gaming (use Steam Big Picture)
- ❌ Windows/commercial games

---

## 💻 Installation

### Pre-built Raspberry Pi Image

The easiest way to get started:

```bash
# Download latest image
wget https://github.com/aldoram5/OpenConsole/releases/latest/download/openconsole-pi5.img.xz

# Flash to SD card
xzcat openconsole-pi5.img.xz | sudo dd of=/dev/sdX bs=4M status=progress
```

### Automated Installation (Linux)

For existing Linux systems:

```bash
git clone --recursive https://github.com/aldoram5/OpenConsole.git
cd OpenConsole
sudo ./install.sh --console-mode
```

📖 **[Full Installation Guide](https://github.com/aldoram5/OpenConsole#installation)**

---

## 🔧 Technical Documentation

For developers and advanced users:

- **[Architecture Overview](ARCHITECTURE)** - System design and components
- **[Database Layer](architecture/DATABASE)** - Game metadata storage
- **[Plugin System](architecture/PLUGINS)** - Extensibility framework
- **[Game Launchers](architecture/LAUNCHERS)** - How games are launched
- **[Creating Plugins](guides/CREATING_PLUGINS)** - Build your own plugins

---

## 📖 Reference

- **[Systems Config](../SYSTEMS)** - es_systems.cfg reference
- **[Gamelists](../GAMELISTS)** - Gamelist XML format
- **[Themes](../THEMES)** - Theme specification
- **[Changelog](CHANGELOG)** - Version history

---

## 💬 Community & Support

<div class="community-links">
  <a href="https://github.com/aldoram5/OpenConsole" class="btn btn-primary">
    📦 GitHub Repository
  </a>

  <a href="https://github.com/aldoram5/OpenConsole/issues" class="btn">
    🐛 Report an Issue
  </a>

  <a href="https://github.com/aldoram5/OpenConsole/discussions" class="btn">
    💬 Discussions
  </a>
</div>

---

## 🎮 Screenshots

*Coming soon - check back after first release!*

---

## 📄 License

OpenConsole is open source software licensed under the MIT License.

**Forked from:** [EmulationStation](https://github.com/RetroPie/EmulationStation) (MIT License)

---

<div class="footer-links">
  <small>
    <a href="https://github.com/aldoram5/OpenConsole">GitHub</a> •
    <a href="guides/">Documentation</a> •
    <a href="CHANGELOG">Changelog</a> •
    <a href="https://github.com/aldoram5/OpenConsole/blob/main/LICENSE">License</a>
  </small>
</div>

<style>
.quick-links {
  display: flex;
  gap: 1rem;
  margin: 2rem 0;
  flex-wrap: wrap;
}

.quick-links .btn {
  flex: 1;
  min-width: 200px;
  padding: 1.5rem;
  text-align: center;
  background: #159957;
  color: white;
  text-decoration: none;
  border-radius: 8px;
  transition: background 0.3s;
}

.quick-links .btn:hover {
  background: #128f4e;
}

.quick-links .btn strong {
  display: block;
  font-size: 1.2em;
  margin-bottom: 0.5rem;
}

.features {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
  gap: 1.5rem;
  margin: 2rem 0;
}

.feature {
  padding: 1.5rem;
  background: #f5f5f5;
  border-radius: 8px;
  border-left: 4px solid #159957;
}

.feature h3 {
  margin-top: 0;
  color: #159957;
}

.community-links {
  display: flex;
  gap: 1rem;
  margin: 2rem 0;
  flex-wrap: wrap;
}

.community-links .btn {
  padding: 0.75rem 1.5rem;
  text-decoration: none;
  border-radius: 6px;
  border: 2px solid #159957;
  color: #159957;
  transition: all 0.3s;
}

.community-links .btn:hover {
  background: #159957;
  color: white;
}

.community-links .btn-primary {
  background: #159957;
  color: white;
}

.community-links .btn-primary:hover {
  background: #128f4e;
  border-color: #128f4e;
}

.footer-links {
  text-align: center;
  margin-top: 3rem;
  padding-top: 2rem;
  border-top: 1px solid #ddd;
}

.lead {
  font-size: 1.25em;
  color: #606c71;
  margin-bottom: 2rem;
}
</style>
