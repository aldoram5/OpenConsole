# OpenConsole System Integration

This directory contains system integration files for deploying OpenConsole as a standalone gaming console operating system.

## Contents

- `openconsole.service` - Systemd service unit file
- `build-pi-image.sh` - Raspberry Pi OS image builder
- `configure-controllers.sh` - Controller setup utility
- `README.md` - This file

## Quick Start

### Standard Installation (Development/Testing)

Install OpenConsole on an existing Linux system:

```bash
# Clone repository
git clone https://github.com/aldoram5/OpenConsole.git
cd OpenConsole

# Install (requires sudo)
sudo ./install.sh
```

### Console Mode Installation

Install with auto-boot to OpenConsole (console appliance):

```bash
# Install in console mode
sudo ./install.sh --console-mode

# Reboot to start OpenConsole
sudo reboot
```

### Pre-built Pi Image

For easy deployment on Raspberry Pi, use a pre-built image:

```bash
# Download image
wget https://github.com/aldoram5/OpenConsole/releases/latest/openconsole-pi5.img.xz

# Flash to SD card (replace /dev/sdX with your SD card device)
xzcat openconsole-pi5.img.xz | sudo dd of=/dev/sdX bs=4M status=progress
sync

# Insert SD card into Raspberry Pi 5 and power on
```

## Installation Options

### install.sh

Main installation script with options:

```bash
sudo ./install.sh [OPTIONS]

Options:
  --console-mode    Full console mode (auto-login, boot to OpenConsole)
  --help            Show help message
```

**What it does:**

- Installs system dependencies
- Builds OpenConsole from source
- Creates openconsole system user
- Installs systemd service
- Configures controller support

**Console mode additionally:**

- Configures auto-login for openconsole user
- Sets up X server to launch automatically
- Configures OpenConsole to start on login
- Optimizes boot process
- Disables unnecessary services

## System Configuration

### Systemd Service

The `openconsole.service` file provides:

- **Auto-start**: OpenConsole starts automatically on boot
- **Auto-restart**: Restarts on crash
- **Resource limits**: Memory and file handle limits
- **Security**: Sandboxing and permission restrictions
- **Logging**: Journal integration for system logs

#### Service Management

```bash
# Start OpenConsole
sudo systemctl start openconsole

# Stop OpenConsole
sudo systemctl stop openconsole

# Restart OpenConsole
sudo systemctl restart openconsole

# Enable auto-start on boot
sudo systemctl enable openconsole

# Disable auto-start
sudo systemctl disable openconsole

# View logs
sudo journalctl -u openconsole -f

# View status
sudo systemctl status openconsole
```

### User Configuration

The installation creates an `openconsole` user with:

- **Home directory**: `/home/openconsole`
- **Groups**: `video`, `audio`, `input`, `render`
- **Shell**: `/bin/bash`

#### Directory Structure

```
/home/openconsole/
├── .openconsole/            # OpenConsole configuration
│   ├── credentials.enc      # Encrypted API tokens
│   ├── downloads/           # Downloaded games
│   └── config.xml           # OpenConsole settings
├── .emulationstation/       # EmulationStation data
│   ├── es_settings.cfg      # Settings
│   ├── es_input.cfg         # Controller mappings
│   └── es_log.txt           # Application log
└── Games/                   # Default games directory
    ├── AppImages/
    ├── RenPy/
    └── ...
```

## Building Raspberry Pi Images

### Requirements

- Debian/Ubuntu Linux (recommended)
- Root access
- 10GB+ free disk space
- Fast internet connection

### Build Process

```bash
# Install dependencies
sudo apt-get install wget xz-utils kpartx qemu-user-static parted

# Run image builder
cd system
sudo ./build-pi-image.sh
```

**Build steps:**

1. Downloads Raspberry Pi OS Lite base image
2. Expands image size (+4GB for games)
3. Mounts image
4. Installs OpenConsole via chroot
5. Configures system for console mode
6. Unmounts and compresses image

**Output:** `openconsole-pi5-YYYYMMDD.img.xz`

### Customizing the Build

Edit `build-pi-image.sh` to customize:

- Base image URL (`BASE_IMAGE_URL`)
- Image size expansion
- Pre-installed games
- Default configuration
- Wi-Fi credentials template

## Controller Configuration

### Setup Utility

```bash
# Run controller setup
./system/configure-controllers.sh
```

**Features:**

1. **Detect Controllers**: Auto-detect connected gamepads
2. **Check Permissions**: Verify input device access
3. **Test Input**: Real-time controller testing
4. **Bluetooth Pairing**: Guided Bluetooth setup
5. **Show Mappings**: Display button mappings
6. **Install Utilities**: Install testing tools
7. **Troubleshooting**: Common issues and fixes

### Supported Controllers

- **PlayStation**: PS4 DualShock, PS5 DualSense
- **Xbox**: Xbox One, Xbox Series X/S controllers
- **Nintendo**: Switch Pro Controller
- **Generic**: Any SDL2-compatible gamepad

### Bluetooth Pairing

```bash
# Start Bluetooth service
sudo systemctl start bluetooth

# Run bluetoothctl
bluetoothctl

# In bluetoothctl:
power on
agent on
default-agent
scan on

# Put controller in pairing mode, then:
pair XX:XX:XX:XX:XX:XX
connect XX:XX:XX:XX:XX:XX
trust XX:XX:XX:XX:XX:XX
quit
```

## Performance Tuning

### Raspberry Pi 5 Optimization

**GPU Memory** (`/boot/config.txt`):
```
gpu_mem=256
```

**Disable Splash Screen**:
```
disable_splash=1
```

**CPU Governor**:
```bash
# Performance mode
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor
```

### Boot Optimization

Services disabled in console mode:

- Bluetooth (if not using wireless controllers)
- Avahi daemon
- Triggerhappy
- Keyboard setup

**Reduce boot time:**

```bash
# Analyze boot time
systemd-analyze

# List slow services
systemd-analyze blame

# Disable unnecessary services
sudo systemctl disable <service-name>
```

## Security Considerations

### Default Credentials

**IMPORTANT**: Change default password on first boot!

```bash
# Default credentials (CHANGE THESE!)
Username: openconsole
Password: openconsole

# Change password
sudo passwd openconsole
```

### Systemd Hardening

The service includes security features:

- `NoNewPrivileges=true` - Prevents privilege escalation
- `PrivateTmp=true` - Isolated /tmp directory
- `ProtectSystem=strict` - Read-only system directories
- `ProtectHome=read-only` - Protected home directories
- `ReadWritePaths` - Only specific paths writable

### Network Security

**SSH Access:**

- Disabled by default in console mode
- Enable: `touch /boot/ssh` before first boot
- Disable password authentication: Use SSH keys
- Change default SSH port if exposing to internet

**Firewall:**

```bash
# Install ufw
sudo apt-get install ufw

# Allow SSH (if needed)
sudo ufw allow 22

# Enable firewall
sudo ufw enable
```

## Troubleshooting

### OpenConsole Won't Start

**Check service status:**
```bash
sudo systemctl status openconsole
```

**View logs:**
```bash
# System journal
sudo journalctl -u openconsole -n 50

# Application log
tail -f /home/openconsole/.emulationstation/es_log.txt
```

**Common issues:**

1. **X server not starting**:
   - Check `~/.xinitrc` exists and is executable
   - Verify X11 packages installed: `sudo apt-get install xorg`

2. **Permission errors**:
   - Ensure openconsole user in correct groups
   - Check file permissions in `/home/openconsole`

3. **Black screen on boot**:
   - Check `/boot/config.txt` for GPU memory setting
   - Verify HDMI connection
   - Try HDMI port closer to power port on Pi 5

### Controller Issues

**Not detected:**

```bash
# List input devices
ls -l /dev/input/

# Check kernel messages
dmesg | grep input

# Test with evtest
sudo evtest
```

**Not responding:**

```bash
# Check user groups
groups openconsole

# Should include: input video audio

# Add to group if missing
sudo usermod -a -G input openconsole
```

### Performance Issues

**Low FPS:**

- Check CPU temperature: `vcgencmd measure_temp` (Raspberry Pi)
- Monitor CPU usage: `htop`
- Reduce theme complexity
- Close background processes

**Stuttering:**

- Use wired controllers (lower latency)
- Reduce network activity during gaming
- Check SD card speed (use Class 10 or UHS-I)

## Maintenance

### Updates

```bash
# Update OpenConsole
cd OpenConsole
git pull
sudo ./install.sh

# Restart service
sudo systemctl restart openconsole
```

### Backup

```bash
# Backup configuration
tar -czf openconsole-backup.tar.gz \
    /home/openconsole/.openconsole \
    /home/openconsole/.emulationstation

# Backup games
tar -czf games-backup.tar.gz /home/openconsole/Games
```

### Restore

```bash
# Restore configuration
sudo tar -xzf openconsole-backup.tar.gz -C /

# Fix permissions
sudo chown -R openconsole:openconsole /home/openconsole
```

## Support

- **Documentation**: [docs/](../docs/)
- **Issues**: https://github.com/aldoram5/OpenConsole/issues
- **Discussions**: https://github.com/aldoram5/OpenConsole/discussions

## License

OpenConsole is licensed under the same license as EmulationStation.
See [LICENSE.md](../LICENSE.md) for details.
