#!/bin/bash
#
# OpenConsole Raspberry Pi OS Image Builder (Docker Version)
# Builds the Pi image inside a Docker container
# Works on macOS, Windows (WSL), and Linux
#
# Usage: ./build-pi-image-docker.sh
#
# Prerequisites:
#   - Docker Desktop installed and running

set -e

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

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

check_docker() {
    if ! command -v docker &> /dev/null; then
        log_error "Docker is not installed"
        log_info "Please install Docker Desktop:"
        log_info "  macOS: https://docs.docker.com/desktop/install/mac-install/"
        log_info "  Windows: https://docs.docker.com/desktop/install/windows-install/"
        log_info "  Linux: https://docs.docker.com/engine/install/"
        exit 1
    fi

    if ! docker info &> /dev/null; then
        log_error "Docker daemon is not running"
        log_info "Please start Docker Desktop"
        exit 1
    fi

    log_info "Docker is available"
}

build_image() {
    log_info "Building Raspberry Pi image using Docker..."
    log_info "This may take 30-60 minutes on first run"

    # Create output directory
    mkdir -p "${PROJECT_DIR}/output"

    # Run the build in a privileged container
    docker run --rm \
        --privileged \
        -v "${PROJECT_DIR}:/workspace" \
        -v "${PROJECT_DIR}/output:/output" \
        -w /workspace \
        debian:bookworm \
        /bin/bash -c '
            set -e

            echo "[INFO] Installing dependencies..."
            apt-get update
            apt-get install -y \
                wget \
                xz-utils \
                kpartx \
                qemu-user-static \
                parted \
                binfmt-support \
                ca-certificates \
                rsync \
                e2fsprogs

            echo "[INFO] Setting up loop devices..."
            # Create loop devices if they do not exist
            for i in {0..7}; do
                mknod -m 0660 /dev/loop$i b 7 $i 2>/dev/null || true
            done

            echo "[INFO] Running image builder..."
            chmod +x /workspace/system/build-pi-image.sh

            # Run the build script from project root
            cd /workspace
            WORK_DIR=/tmp/build-image ./system/build-pi-image.sh

            # Copy the output (image is created in /tmp)
            cp /tmp/openconsole-pi5-*.img.xz /output/ 2>/dev/null || \
            cp /workspace/openconsole-pi5-*.img.xz /output/ 2>/dev/null || \
            echo "[WARN] Could not find output image"

            echo "[INFO] Build complete!"
        '

    log_info "Docker build finished"
}

print_result() {
    echo ""
    echo "========================================="
    echo "  Build Complete!"
    echo "========================================="
    echo ""

    local image_file=$(ls "${PROJECT_DIR}/output"/openconsole-pi5-*.img.xz 2>/dev/null | head -1)
    if [[ -n "$image_file" ]]; then
        echo "Image file: $image_file"
        echo "Size: $(du -h "$image_file" | cut -f1)"
        echo ""
        echo "To flash the image:"
        echo ""
        echo "  macOS (using Raspberry Pi Imager):"
        echo "    1. Download Raspberry Pi Imager"
        echo "    2. Choose 'Use custom' and select the .img.xz file"
        echo "    3. Select your SD card"
        echo "    4. Write"
        echo ""
        echo "  Linux:"
        echo "    xzcat $image_file | sudo dd of=/dev/sdX bs=4M status=progress"
        echo ""
    else
        log_error "Output image not found in ${PROJECT_DIR}/output/"
        log_info "Check the Docker output for errors"
    fi

    echo "========================================="
}

main() {
    log_info "OpenConsole Pi Image Builder (Docker)"
    echo ""

    check_docker
    build_image
    print_result
}

main "$@"
