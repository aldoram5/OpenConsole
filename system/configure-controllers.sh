#!/bin/bash
#
# OpenConsole Controller Configuration Utility
# Helps detect and configure game controllers
#

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
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

print_header() {
    echo -e "${BLUE}=========================================${NC}"
    echo -e "${BLUE}  OpenConsole Controller Setup${NC}"
    echo -e "${BLUE}=========================================${NC}"
    echo ""
}

detect_controllers() {
    log_info "Detecting connected controllers..."
    echo ""

    local found=false

    # Check for connected input devices
    if [[ -d /dev/input ]]; then
        for device in /dev/input/event*; do
            if [[ -e "$device" ]]; then
                local device_name=$(cat /sys/class/input/$(basename $device | sed 's/event//;s/^/event/')/device/name 2>/dev/null || echo "Unknown")

                # Filter for likely game controllers
                if [[ $device_name == *"Gamepad"* ]] || \
                   [[ $device_name == *"Controller"* ]] || \
                   [[ $device_name == *"Joystick"* ]] || \
                   [[ $device_name == *"Xbox"* ]] || \
                   [[ $device_name == *"PlayStation"* ]] || \
                   [[ $device_name == *"Nintendo"* ]]; then

                    echo -e "${GREEN}✓${NC} Found: $device_name"
                    echo "  Device: $device"
                    found=true
                fi
            fi
        done
    fi

    echo ""

    if [[ $found == false ]]; then
        log_warn "No game controllers detected"
        echo ""
        echo "Troubleshooting:"
        echo "  1. Ensure controller is connected (USB or Bluetooth)"
        echo "  2. For Bluetooth controllers, pair them first"
        echo "  3. Check dmesg output: dmesg | tail -20"
        echo ""
        return 1
    fi

    return 0
}

check_permissions() {
    log_info "Checking input device permissions..."
    echo ""

    local current_user=$(whoami)
    local groups=$(groups)

    if [[ $groups == *"input"* ]]; then
        echo -e "${GREEN}✓${NC} User '$current_user' is in 'input' group"
    else
        log_warn "User '$current_user' is NOT in 'input' group"
        echo "  Fix: sudo usermod -a -G input $current_user"
        echo "  Then log out and back in"
    fi

    echo ""
}

test_controller_input() {
    log_info "Testing controller input..."
    echo ""
    echo "This will show input events from your controllers"
    echo "Press any button on your controller to test"
    echo "Press Ctrl+C to stop"
    echo ""

    sleep 2

    # Use evtest if available, otherwise show raw input
    if command -v evtest &> /dev/null; then
        # List devices and let user choose
        evtest
    else
        log_warn "evtest not installed. Install with: sudo apt-get install evtest"
        echo ""
        echo "Showing raw input events (basic test)..."
        echo "Press Ctrl+C to stop"
        echo ""

        # Show input events from /dev/input
        cat /dev/input/event* 2>/dev/null &
        local cat_pid=$!

        # Wait for Ctrl+C
        trap "kill $cat_pid 2>/dev/null; exit 0" INT
        wait
    fi
}

configure_bluetooth_controllers() {
    log_info "Bluetooth Controller Pairing"
    echo ""
    echo "Common Bluetooth controllers:"
    echo "  - PS4/PS5 DualShock/DualSense"
    echo "  - Xbox One/Series controller"
    echo "  - Nintendo Switch Pro Controller"
    echo "  - Generic Bluetooth gamepads"
    echo ""

    if ! command -v bluetoothctl &> /dev/null; then
        log_error "bluetoothctl not found. Install with: sudo apt-get install bluetooth"
        return 1
    fi

    echo "Pairing Instructions:"
    echo ""
    echo "1. Put your controller in pairing mode:"
    echo "   - PS4/PS5: Hold Share + PS button until light flashes"
    echo "   - Xbox: Hold pairing button until LED flashes"
    echo "   - Switch Pro: Hold sync button until LEDs flash"
    echo ""
    echo "2. Run these commands:"
    echo "   $ bluetoothctl"
    echo "   [bluetooth]# power on"
    echo "   [bluetooth]# agent on"
    echo "   [bluetooth]# default-agent"
    echo "   [bluetooth]# scan on"
    echo "   [bluetooth]# pair XX:XX:XX:XX:XX:XX (controller MAC address)"
    echo "   [bluetooth]# connect XX:XX:XX:XX:XX:XX"
    echo "   [bluetooth]# trust XX:XX:XX:XX:XX:XX"
    echo "   [bluetooth]# quit"
    echo ""

    read -p "Do you want to start bluetoothctl now? (y/n) " -n 1 -r
    echo ""

    if [[ $REPLY =~ ^[Yy]$ ]]; then
        bluetoothctl
    fi
}

show_controller_mapping() {
    log_info "OpenConsole Controller Mappings"
    echo ""
    echo "Default SDL2 Controller Mappings:"
    echo ""
    echo "  Navigation:"
    echo "    D-Pad / Left Stick → Navigate menus"
    echo "    A / B buttons → Select / Back"
    echo ""
    echo "  Virtual Keyboard:"
    echo "    D-Pad → Move cursor"
    echo "    A → Select character"
    echo "    B → Backspace"
    echo "    X → Space"
    echo "    Y → Shift (toggle)"
    echo "    Start → Submit"
    echo ""
    echo "  In-Game:"
    echo "    Mappings depend on the game"
    echo "    Most games use standard SDL2 gamepad layout"
    echo ""
}

install_utilities() {
    log_info "Installing controller utilities..."
    echo ""

    if [[ $EUID -ne 0 ]]; then
        log_error "This option requires root. Run with sudo."
        return 1
    fi

    apt-get update
    apt-get install -y \
        evtest \
        jstest-gtk \
        bluetooth \
        bluez \
        bluez-tools

    log_info "Utilities installed successfully"
    echo ""
    echo "Available tools:"
    echo "  evtest     - Test input events"
    echo "  jstest-gtk - GUI joystick tester"
    echo "  bluetoothctl - Bluetooth pairing"
}

show_troubleshooting() {
    echo "========================================="
    echo "  Controller Troubleshooting"
    echo "========================================="
    echo ""
    echo "Controller not detected:"
    echo "  1. Check USB connection (try different port)"
    echo "  2. Check Bluetooth pairing"
    echo "  3. View kernel messages: dmesg | grep input"
    echo "  4. List input devices: ls /dev/input/event*"
    echo ""
    echo "Controller detected but not working:"
    echo "  1. Check permissions (run 'check permissions' option)"
    echo "  2. Test with evtest: sudo evtest"
    echo "  3. Verify in OpenConsole input configuration"
    echo ""
    echo "Bluetooth controller issues:"
    echo "  1. Ensure Bluetooth is enabled: bluetoothctl power on"
    echo "  2. Re-pair the controller"
    echo "  3. Trust the controller: bluetoothctl trust XX:XX:XX:XX:XX:XX"
    echo "  4. Check Bluetooth service: systemctl status bluetooth"
    echo ""
    echo "Input lag / latency:"
    echo "  1. Use wired connection if possible"
    echo "  2. Reduce wireless interference"
    echo "  3. Update controller firmware (if possible)"
    echo "  4. Check CPU usage during gaming"
    echo ""
}

main_menu() {
    while true; do
        echo ""
        echo "What would you like to do?"
        echo ""
        echo "  1) Detect connected controllers"
        echo "  2) Check permissions"
        echo "  3) Test controller input"
        echo "  4) Pair Bluetooth controller"
        echo "  5) Show controller mappings"
        echo "  6) Install controller utilities (requires sudo)"
        echo "  7) Troubleshooting guide"
        echo "  8) Exit"
        echo ""
        read -p "Select option (1-8): " choice

        case $choice in
            1)
                detect_controllers
                ;;
            2)
                check_permissions
                ;;
            3)
                test_controller_input
                ;;
            4)
                configure_bluetooth_controllers
                ;;
            5)
                show_controller_mapping
                ;;
            6)
                install_utilities
                ;;
            7)
                show_troubleshooting
                ;;
            8)
                echo ""
                log_info "Exiting controller configuration"
                exit 0
                ;;
            *)
                log_error "Invalid option"
                ;;
        esac
    done
}

# Main
print_header
main_menu
