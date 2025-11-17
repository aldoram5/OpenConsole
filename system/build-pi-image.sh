#!/bin/bash
#
# OpenConsole Raspberry Pi OS Image Builder
# Usage: sudo WORK_DIR="./build-image" ./system/build-pi-image.sh

set -euo pipefail

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

log_info(){ echo -e "${GREEN}[INFO]${NC} $1"; }
log_warn(){ echo -e "${YELLOW}[WARN]${NC} $1"; }
log_error(){ echo -e "${RED}[ERROR]${NC} $1"; }

check_dependencies() {
    log_info "Checking dependencies..."
    local deps=("wget" "xz" "kpartx" "qemu-aarch64-static" "parted" "losetup" "rsync")
    local missing=()
    for dep in "${deps[@]}"; do
        if ! command -v "$dep" &>/dev/null; then
            missing+=("$dep")
        fi
    done
    if [[ ${#missing[@]} -gt 0 ]]; then
        log_error "Missing dependencies: ${missing[*]}"
        log_info "Install with: sudo apt-get install wget xz-utils kpartx qemu-user-static parted losetup rsync"
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
    local base_image_file
    base_image_file=$(basename "$BASE_IMAGE_URL")
    if [[ ! -f "$base_image_file" ]]; then
        wget -O "$base_image_file" "$BASE_IMAGE_URL"
    else
        log_info "Base image already downloaded, skipping..."
    fi
    if [[ ! -f "base.img" ]]; then
        log_info "Extracting base image..."
        xz -d -k "$base_image_file"
        mv -- *.img base.img
    fi
    cd - >/dev/null
}

expand_image() {
    log_info "Expanding image size..."
    # Add 4GB of space for package installation
    dd if=/dev/zero bs=1M count=4096 >> "${WORK_DIR}/base.img"
    LOOPDEV=$(losetup -f --show "${WORK_DIR}/base.img")
    log_info "Attached ${WORK_DIR}/base.img to ${LOOPDEV}"
    partprobe "$LOOPDEV" || true
    parted --script "$LOOPDEV" resizepart 2 100% || true
    losetup -d "$LOOPDEV"
    log_info "Image partition expanded"
}

mount_image() {
    log_info "Mounting image..."
    mkdir -p "$MOUNT_POINT"
    local loop_device
    loop_device=$(kpartx -av "${WORK_DIR}/base.img" | awk -F' ' '/add map/ {print $3}' | sed 's/p[0-9]*$//' | head -1)
    if [[ -z "$loop_device" ]]; then
        log_error "Failed to create partition mappings with kpartx"
        exit 1
    fi
    sleep 1
    # Resize the filesystem to use the expanded partition
    log_info "Resizing filesystem..."
    e2fsck -f -y "/dev/mapper/${loop_device}p2" || true
    resize2fs "/dev/mapper/${loop_device}p2" || true
    mount "/dev/mapper/${loop_device}p2" "$MOUNT_POINT"
    mkdir -p "${MOUNT_POINT}/boot"
    mount "/dev/mapper/${loop_device}p1" "${MOUNT_POINT}/boot"
    cp /usr/bin/qemu-aarch64-static "${MOUNT_POINT}/usr/bin/" || true
    mount --bind /dev "${MOUNT_POINT}/dev"
    mount --bind /dev/pts "${MOUNT_POINT}/dev/pts"
    mount --bind /proc "${MOUNT_POINT}/proc"
    mount --bind /sys "${MOUNT_POINT}/sys"
    cp /etc/resolv.conf "${MOUNT_POINT}/etc/resolv.conf" || true
    log_info "Image mounted at $MOUNT_POINT (loop: ${loop_device})"
    echo "$loop_device" > "${WORK_DIR}/loop_device.txt"
}

install_openconsole_to_image() {
    log_info "Installing OpenConsole to image..."
    mkdir -p "${MOUNT_POINT}/tmp/openconsole"
    rsync -a \
        --exclude="${WORK_DIR#/}" \
        --exclude="${WORK_DIR}" \
        --exclude="${WORK_DIR}/*" \
        --exclude="build" \
        --exclude="output" \
        --exclude=".git" \
        . "${MOUNT_POINT}/tmp/openconsole/"
    chroot "$MOUNT_POINT" /bin/bash <<'CHROOT_SCRIPT'
set -e
apt-get update
cd /tmp/openconsole || exit 1
if [[ -f install.sh ]]; then
  chmod +x install.sh
  ./install.sh --console-mode
else
  echo "install.sh not found in /tmp/openconsole" >&2
  exit 1
fi
cd /
rm -rf /tmp/openconsole
apt-get clean
rm -rf /var/lib/apt/lists/*
CHROOT_SCRIPT
    log_info "OpenConsole installed successfully"
}

configure_image() {
    log_info "Configuring image..."
    echo "openconsole" > "${MOUNT_POINT}/etc/hostname"
    sed -i 's/127.0.1.1.*/127.0.1.1\topenconsole/g' "${MOUNT_POINT}/etc/hosts" || true
    touch "${MOUNT_POINT}/boot/ssh"
    cat > "${MOUNT_POINT}/boot/wpa_supplicant.conf" <<'EOF'
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=US

network={
    ssid="YOUR_WIFI_SSID"
    psk="YOUR_WIFI_PASSWORD"
}
EOF
    chroot "$MOUNT_POINT" /bin/bash <<'CHROOT_SCRIPT'
echo "openconsole:openconsole" | chpasswd || true
CHROOT_SCRIPT
    cat > "${MOUNT_POINT}/etc/rc.local" <<'EOF'
#!/bin/bash
if [[ ! -f /var/log/firstboot.done ]]; then
    raspi-config --expand-rootfs || true
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
    rm -f "${MOUNT_POINT}/usr/bin/qemu-aarch64-static" || true
    umount "${MOUNT_POINT}/boot" || true
    umount "${MOUNT_POINT}/dev/pts" || true
    umount "${MOUNT_POINT}/dev" || true
    umount "${MOUNT_POINT}/proc" || true
    umount "${MOUNT_POINT}/sys" || true
    umount "$MOUNT_POINT" || true
    if [[ -f "${WORK_DIR}/loop_device.txt" ]]; then
        local loop_device
        loop_device=$(cat "${WORK_DIR}/loop_device.txt")
        kpartx -d "/dev/${loop_device}" || true
        rm -f "${WORK_DIR}/loop_device.txt"
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
    echo "Size: $(du -h "${IMAGE_NAME}.xz" | cut -f1 || echo unknown)"
    echo ""
    echo "Default credentials:"
    echo "  User: openconsole"
    echo "  Password: openconsole"
    echo ""
}

# Preserve original exit code when running cleanup
trap 'rc=$?; cleanup; exit $rc' EXIT

main() {
    log_info "Starting OpenConsole Pi image build..."
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

main
