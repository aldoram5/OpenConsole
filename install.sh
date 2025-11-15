#!/bin/bash
#
# OpenConsole Installation Script
# Installs OpenConsole and configures the system for console mode operation
#
# Usage: sudo ./install.sh [--console-mode]
#   --console-mode: Full console mode with auto-login and boot-to-OpenConsole

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
OPENCONSOLE_USER="${OPENCONSOLE_USER:-openconsole}"
INSTALL_DIR="${INSTALL_DIR:-/usr/local}"
CONSOLE_MODE=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --console-mode)
            CONSOLE_MODE=true
            shift
            ;;
        --help)
            echo "Usage: sudo ./install.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --console-mode    Enable full console mode (auto-login, boot to OpenConsole)"
            echo "  --help            Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Run with --help for usage information"
            exit 1
            ;;
    esac
done

# Helper functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_root() {
    if [[ $EUID -ne 0 ]]; then
        log_error "This script must be run as root (use sudo)"
        exit 1
    fi
}

detect_platform() {
    log_info "Detecting platform..."

    if [[ -f /proc/device-tree/model ]]; then
        MODEL=$(cat /proc/device-tree/model)
        if [[ $MODEL == *"Raspberry Pi"* ]]; then
            echo "raspberry-pi"
            return
        fi
    fi

    echo "generic-linux"
}

install_dependencies() {
    log_info "Installing dependencies..."

    apt-get update
    apt-get install -y \
        build-essential \
        cmake \
        libsdl2-dev \
        libfreeimage-dev \
        libfreetype6-dev \
        libcurl4-openssl-dev \
        rapidjson-dev \
        libasound2-dev \
        libgl1-mesa-dev \
        libssl-dev \
        libsqlite3-dev \
        libvlc-dev \
        libvlccore-dev \
        vlc-bin \
        fonts-droid-fallback \
        libboost-system-dev \
        libboost-filesystem-dev \
        libboost-locale-dev \
        git \
        xorg \
        openbox \
        pulseaudio \
        alsa-utils

    log_info "Dependencies installed successfully"
}

build_openconsole() {
    log_info "Building OpenConsole..."

    if [[ ! -d build ]]; then
        mkdir build
    fi

    cd build
    cmake ..
    make -j$(nproc)

    log_info "Build completed successfully"
    cd ..
}

install_openconsole() {
    log_info "Installing OpenConsole to ${INSTALL_DIR}..."

    # Install binary
    install -m 755 build/emulationstation "${INSTALL_DIR}/bin/openconsole"

    # Create symbolic link for compatibility
    ln -sf "${INSTALL_DIR}/bin/openconsole" "${INSTALL_DIR}/bin/emulationstation"

    # Install resources
    if [[ -d resources ]]; then
        mkdir -p "${INSTALL_DIR}/share/openconsole"
        cp -r resources/* "${INSTALL_DIR}/share/openconsole/"
    fi

    log_info "OpenConsole installed successfully"
}

create_user() {
    log_info "Creating openconsole user..."

    if id "${OPENCONSOLE_USER}" &>/dev/null; then
        log_warn "User ${OPENCONSOLE_USER} already exists, skipping creation"
    else
        useradd -m -s /bin/bash -G video,audio,input,render "${OPENCONSOLE_USER}"
        log_info "User ${OPENCONSOLE_USER} created"
    fi

    # Create necessary directories
    sudo -u "${OPENCONSOLE_USER}" mkdir -p "/home/${OPENCONSOLE_USER}/.openconsole"
    sudo -u "${OPENCONSOLE_USER}" mkdir -p "/home/${OPENCONSOLE_USER}/.emulationstation"
    sudo -u "${OPENCONSOLE_USER}" mkdir -p "/home/${OPENCONSOLE_USER}/Games"
}

install_systemd_service() {
    log_info "Installing systemd service..."

    cp system/openconsole.service /etc/systemd/system/

    systemctl daemon-reload
    systemctl enable openconsole.service

    log_info "Systemd service installed and enabled"
}

configure_autologin() {
    log_info "Configuring auto-login for ${OPENCONSOLE_USER}..."

    # Create systemd override directory
    mkdir -p /etc/systemd/system/getty@tty1.service.d/

    # Create auto-login override
    cat > /etc/systemd/system/getty@tty1.service.d/autologin.conf <<EOF
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin ${OPENCONSOLE_USER} --noclear %I \$TERM
EOF

    log_info "Auto-login configured"
}

configure_xserver() {
    log_info "Configuring X server..."

    # Create .xinitrc for openconsole user
    cat > "/home/${OPENCONSOLE_USER}/.xinitrc" <<'EOF'
#!/bin/sh
# Disable screen blanking
xset s off
xset -dpms
xset s noblank

# Hide mouse cursor
unclutter -idle 0.1 &

# Start window manager
openbox &

# Start OpenConsole
/usr/local/bin/openconsole
EOF

    chmod +x "/home/${OPENCONSOLE_USER}/.xinitrc"
    chown "${OPENCONSOLE_USER}:${OPENCONSOLE_USER}" "/home/${OPENCONSOLE_USER}/.xinitrc"

    # Create .bash_profile to auto-start X
    cat > "/home/${OPENCONSOLE_USER}/.bash_profile" <<'EOF'
# Auto-start X on login to tty1
if [[ -z $DISPLAY ]] && [[ $(tty) = /dev/tty1 ]]; then
    exec startx
fi
EOF

    chown "${OPENCONSOLE_USER}:${OPENCONSOLE_USER}" "/home/${OPENCONSOLE_USER}/.bash_profile"

    # Install unclutter for hiding mouse cursor
    apt-get install -y unclutter

    log_info "X server configuration complete"
}

optimize_boot() {
    log_info "Optimizing boot configuration..."

    # Disable unnecessary services
    SERVICES_TO_DISABLE=(
        "bluetooth.service"
        "avahi-daemon.service"
        "triggerhappy.service"
        "keyboard-setup.service"
    )

    for service in "${SERVICES_TO_DISABLE[@]}"; do
        if systemctl is-enabled "$service" &>/dev/null; then
            systemctl disable "$service" || true
            log_info "Disabled $service"
        fi
    done

    # Raspberry Pi specific optimizations
    if [[ $(detect_platform) == "raspberry-pi" ]]; then
        log_info "Applying Raspberry Pi specific optimizations..."

        # Update /boot/config.txt for optimal performance
        if [[ -f /boot/config.txt ]]; then
            # GPU memory split
            if ! grep -q "^gpu_mem=" /boot/config.txt; then
                echo "gpu_mem=256" >> /boot/config.txt
            fi

            # Disable rainbow splash
            if ! grep -q "^disable_splash=" /boot/config.txt; then
                echo "disable_splash=1" >> /boot/config.txt
            fi

            log_info "Raspberry Pi config.txt updated"
        fi
    fi

    log_info "Boot optimization complete"
}

configure_controllers() {
    log_info "Configuring controller support..."

    # Ensure udev rules for controllers
    cat > /etc/udev/rules.d/99-openconsole-input.rules <<'EOF'
# Game controllers
SUBSYSTEM=="input", GROUP="input", MODE="0660"
SUBSYSTEM=="usb", ATTRS{idVendor}=="054c", ATTRS{idProduct}=="05c4", GROUP="input", MODE="0660"
SUBSYSTEM=="usb", ATTRS{idVendor}=="054c", ATTRS{idProduct}=="09cc", GROUP="input", MODE="0660"
KERNEL=="hidraw*", SUBSYSTEM=="hidraw", GROUP="input", MODE="0660"
EOF

    udevadm control --reload-rules
    udevadm trigger

    log_info "Controller configuration complete"
}

print_summary() {
    echo ""
    echo "========================================="
    echo "  OpenConsole Installation Complete!"
    echo "========================================="
    echo ""
    echo "Installation directory: ${INSTALL_DIR}"
    echo "OpenConsole user: ${OPENCONSOLE_USER}"
    echo ""

    if [[ $CONSOLE_MODE == true ]]; then
        echo "Console mode: ENABLED"
        echo "  - Auto-login configured for ${OPENCONSOLE_USER}"
        echo "  - X server will start automatically"
        echo "  - OpenConsole will launch on boot"
        echo ""
        echo "To start using OpenConsole:"
        echo "  1. Reboot the system: sudo reboot"
        echo "  2. System will boot directly into OpenConsole"
        echo ""
    else
        echo "Console mode: DISABLED"
        echo ""
        echo "To start OpenConsole manually:"
        echo "  1. Run: openconsole"
        echo ""
        echo "To enable console mode later:"
        echo "  1. Run: sudo systemctl enable openconsole.service"
        echo "  2. Configure auto-login for ${OPENCONSOLE_USER}"
        echo ""
    fi

    echo "Configuration directories:"
    echo "  - User data: /home/${OPENCONSOLE_USER}/.openconsole"
    echo "  - EmulationStation data: /home/${OPENCONSOLE_USER}/.emulationstation"
    echo "  - Games directory: /home/${OPENCONSOLE_USER}/Games"
    echo ""
    echo "Logs:"
    echo "  - View logs: sudo journalctl -u openconsole.service -f"
    echo "  - App log: /home/${OPENCONSOLE_USER}/.emulationstation/es_log.txt"
    echo ""
    echo "========================================="
}

# Main installation flow
main() {
    log_info "Starting OpenConsole installation..."
    log_info "Platform: $(detect_platform)"
    log_info "Console mode: $CONSOLE_MODE"
    echo ""

    check_root
    install_dependencies
    build_openconsole
    install_openconsole
    create_user
    install_systemd_service
    configure_controllers

    if [[ $CONSOLE_MODE == true ]]; then
        configure_autologin
        configure_xserver
        optimize_boot
    fi

    print_summary
}

# Run main installation
main
