#!/bin/bash
#
# OpenConsole Raspberry Pi OS Image Builder
# Creates a bootable Raspberry Pi OS image with OpenConsole pre-installed
#
# Usage: sudo ./build-pi-image.sh

set -e

# Configuration
WORK_DIR="${WORK_DIR:-./build-image}"
IMAGE_NAME="openconsole-pi5-$(date +%Y%m%d).img"
BASE_IMAGE_URL="https://downloads.raspberrypi.org/raspios_lite_arm64/images/raspios_lite_arm64-2024-03-15/2024-03-15-raspios-bookworm-arm64-lite.img.xz"
MOUNT_POINT="${WORK_DIR}/mnt"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_dependencies() {
    log_info "Checking dependencies..."

    local deps=("wget" "xz" "kpartx" "qemu-aarch64-static" "parted")
    local missing=()

    for dep in "${deps[@]}"; do
        if ! command -v "$dep" &> /dev/null; then
            missing+=("$dep")
        fi
    done

    if [[ ${#missing[@]} -gt 0 ]]; then
        log_error "Missing dependencies: ${missing[*]}"
        log_info "Install with: sudo apt-get install wget xz-utils kpartx qemu-user-static parted"
        exit 1
    fi

    log_info "All dependencies satisfied"
}

check_root() {
    if [[ $EUID -ne 0 ]]; then
        log_error "This script must be run as root"
        exit 1
    fi
}

download_base_image() {
    log_info "Downloading Raspberry Pi OS base image..."

    mkdir -p "$WORK_DIR"
    cd "$WORK_DIR"

    local base_image_file=$(basename "$BASE_IMAGE_URL")

    if [[ ! -f "$base_image_file" ]]; then
        wget -O "$base_image_file" "$BASE_IMAGE_URL"
    else
        log_info "Base image already downloaded, skipping..."
    fi

    # Extract if needed
    if [[ ! -f "base.img" ]]; then
        log_info "Extracting base image..."
        xz -d -k "$base_image_file"
        mv *.img base.img
    fi

    cd -
}

expand_image() {
    log_info "Expanding image size..."

    # Add 4GB to the image for OpenConsole and games
    dd if=/dev/zero bs=1M count=4096 >> "${WORK_DIR}/base.img"

    # Expand partition
    parted "${WORK_DIR}/base.img" resizepart 2 100%
}

mount_image() {
    log_info "Mounting image..."

    mkdir -p "$MOUNT_POINT"

    # Use kpartx to create loop devices
    local loop_device=$(kpartx -av "${WORK_DIR}/base.img" | grep -o 'loop[0-9]*' | head -1)

    # Mount root partition
    mount "/dev/mapper/${loop_device}p2" "$MOUNT_POINT"

    # Mount boot partition
    mount "/dev/mapper/${loop_device}p1" "${MOUNT_POINT}/boot"

    # Set up qemu for chroot
    cp /usr/bin/qemu-aarch64-static "${MOUNT_POINT}/usr/bin/"

    log_info "Image mounted at $MOUNT_POINT"
    echo "$loop_device" > "${WORK_DIR}/loop_device.txt"
}

install_openconsole_to_image() {
    log_info "Installing OpenConsole to image..."

    # Copy OpenConsole source to image
    mkdir -p "${MOUNT_POINT}/tmp/openconsole"
    cp -r . "${MOUNT_POINT}/tmp/openconsole/"

    # Chroot and install
    chroot "$MOUNT_POINT" /bin/bash <<'CHROOT_SCRIPT'
set -e

# Update package lists
apt-get update

# Run OpenConsole installation
cd /tmp/openconsole
chmod +x install.sh
./install.sh --console-mode

# Cleanup
cd /
rm -rf /tmp/openconsole

# Clean package cache
apt-get clean
rm -rf /var/lib/apt/lists/*

CHROOT_SCRIPT

    log_info "OpenConsole installed successfully"
}

configure_image() {
    log_info "Configuring image..."

    # Set hostname
    echo "openconsole" > "${MOUNT_POINT}/etc/hostname"

    # Update hosts file
    sed -i 's/127.0.1.1.*/127.0.1.1\topenconsole/g' "${MOUNT_POINT}/etc/hosts"

    # Enable SSH (optional, for debugging)
    touch "${MOUNT_POINT}/boot/ssh"

    # Configure Wi-Fi (template - users will need to edit)
    cat > "${MOUNT_POINT}/boot/wpa_supplicant.conf" <<'EOF'
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=US

network={
    ssid="YOUR_WIFI_SSID"
    psk="YOUR_WIFI_PASSWORD"
}
EOF

    # Set default password (should be changed on first boot)
    chroot "$MOUNT_POINT" /bin/bash <<'CHROOT_SCRIPT'
# Set password for openconsole user
echo "openconsole:openconsole" | chpasswd
CHROOT_SCRIPT

    # Create first-boot configuration script
    cat > "${MOUNT_POINT}/etc/rc.local" <<'EOF'
#!/bin/bash
# First boot configuration

# Expand filesystem on first boot
if [[ ! -f /var/log/firstboot.done ]]; then
    raspi-config --expand-rootfs
    touch /var/log/firstboot.done
    reboot
fi

exit 0
EOF

    chmod +x "${MOUNT_POINT}/etc/rc.local"

    log_info "Image configuration complete"
}

unmount_image() {
    log_info "Unmounting image..."

    # Remove qemu binary
    rm -f "${MOUNT_POINT}/usr/bin/qemu-aarch64-static"

    # Unmount filesystems
    umount "${MOUNT_POINT}/boot" || true
    umount "$MOUNT_POINT" || true

    # Remove loop devices
    if [[ -f "${WORK_DIR}/loop_device.txt" ]]; then
        local loop_device=$(cat "${WORK_DIR}/loop_device.txt")
        kpartx -d "/dev/${loop_device}" || true
        rm "${WORK_DIR}/loop_device.txt"
    fi

    log_info "Image unmounted"
}

compress_image() {
    log_info "Compressing final image..."

    cd "$WORK_DIR"
    xz -z -9 -T 0 base.img
    mv base.img.xz "../${IMAGE_NAME}.xz"
    cd -

    log_info "Image compressed: ${IMAGE_NAME}.xz"
}

cleanup() {
    log_info "Cleaning up..."

    unmount_image || true

    if [[ -d "$WORK_DIR" ]]; then
        rm -rf "$WORK_DIR"
    fi

    log_info "Cleanup complete"
}

print_summary() {
    echo ""
    echo "========================================="
    echo "  Image Build Complete!"
    echo "========================================="
    echo ""
    echo "Image file: ${IMAGE_NAME}.xz"
    echo "Size: $(du -h "${IMAGE_NAME}.xz" | cut -f1)"
    echo ""
    echo "To flash the image to an SD card:"
    echo "  1. Insert SD card"
    echo "  2. Identify device: lsblk"
    echo "  3. Flash: xzcat ${IMAGE_NAME}.xz | sudo dd of=/dev/sdX bs=4M status=progress"
    echo "  4. Sync: sync"
    echo ""
    echo "Default credentials:"
    echo "  User: openconsole"
    echo "  Password: openconsole"
    echo "  (CHANGE THIS ON FIRST BOOT!)"
    echo ""
    echo "First boot:"
    echo "  - System will auto-expand filesystem"
    echo "  - Reboot automatically"
    echo "  - OpenConsole will start on second boot"
    echo ""
    echo "========================================="
}

# Trap errors and cleanup
trap cleanup EXIT ERR

# Main build flow
main() {
    log_info "Starting OpenConsole Pi image build..."
    echo ""

    check_root
    check_dependencies
    download_base_image
    expand_image
    mount_image
    install_openconsole_to_image
    configure_image
    unmount_image
    compress_image
    cleanup

    print_summary
}

# Run main
main
