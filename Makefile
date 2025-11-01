# ESP32 APRS Tracker Makefile
# Convenient wrapper around PlatformIO commands

.PHONY: help init build upload monitor clean test format check all

# Default target
help:
	@echo "ESP32 APRS Tracker - Makefile Commands"
	@echo "========================================"
	@echo ""
	@echo "Setup:"
	@echo "  make init          - Initialize PlatformIO project and install dependencies"
	@echo ""
	@echo "Build & Upload:"
	@echo "  make build         - Build the project"
	@echo "  make upload        - Upload firmware to device"
	@echo "  make all           - Build and upload"
	@echo ""
	@echo "Monitor & Debug:"
	@echo "  make monitor       - Start serial monitor"
	@echo "  make run           - Upload and start monitor"
	@echo ""
	@echo "Maintenance:"
	@echo "  make clean         - Clean build files"
	@echo "  make clean-all     - Deep clean (removes .pio directory)"
	@echo "  make update        - Update library dependencies"
	@echo ""
	@echo "Code Quality:"
	@echo "  make check         - Check for common issues"
	@echo "  make size          - Show compiled binary size"
	@echo ""
	@echo "Configuration:"
	@echo "  make config        - Show current configuration"
	@echo ""

# Initialize PlatformIO project
init:
	@echo "Initializing PlatformIO project..."
	@if ! command -v pio &> /dev/null; then \
		echo "Error: PlatformIO CLI not found!"; \
		echo "Install with: pip install platformio"; \
		exit 1; \
	fi
	pio project init --ide vscode
	@echo "Installing dependencies..."
	pio lib install
	@echo ""
	@echo "✓ Project initialized successfully!"
	@echo ""
	@echo "⚠ IMPORTANT: Edit src/main.cpp and change the callsign!"
	@echo "   Line 103: aprsConfig.callsign = \"YOUR_CALL\";"
	@echo ""

# Build the project
build:
	@echo "Building project..."
	pio run

# Upload to device
upload:
	@echo "Uploading to device..."
	pio run -t upload

# Build and upload
all: build upload

# Start serial monitor
monitor:
	@echo "Starting serial monitor (Ctrl+C to exit)..."
	pio device monitor

# Upload and monitor
run: upload
	@sleep 2
	@$(MAKE) monitor

# Clean build files
clean:
	@echo "Cleaning build files..."
	pio run -t clean

# Deep clean (removes .pio directory)
clean-all:
	@echo "Deep cleaning project..."
	rm -rf .pio .vscode

# Update dependencies
update:
	@echo "Updating dependencies..."
	pio lib update
	pio platform update

# Check for common issues
check:
	@echo "Checking project configuration..."
	@echo ""
	@echo "1. Checking callsign in main.cpp..."
	@if grep -q 'callsign = "VA7RCV"' src/main.cpp; then \
		echo "   ⚠ WARNING: Using example callsign VA7RCV"; \
		echo "   Please change to your callsign in src/main.cpp line 103"; \
	elif grep -q 'callsign = "NOCALL"' src/main.cpp; then \
		echo "   ⚠ WARNING: Using NOCALL callsign"; \
		echo "   Please change to your callsign in src/main.cpp line 103"; \
	else \
		echo "   ✓ Callsign appears to be customized"; \
	fi
	@echo ""
	@echo "2. Checking PlatformIO installation..."
	@pio --version
	@echo ""
	@echo "3. Checking for connected devices..."
	@pio device list || echo "   ⚠ No devices found"
	@echo ""

# Show compiled binary size
size:
	@echo "Checking binary size..."
	@if [ -f .pio/build/nodemcu-32s/firmware.bin ]; then \
		ls -lh .pio/build/nodemcu-32s/firmware.bin; \
		du -h .pio/build/nodemcu-32s/firmware.elf; \
	else \
		echo "No compiled binary found. Run 'make build' first."; \
	fi

# Show current configuration
config:
	@echo "Project Configuration"
	@echo "====================="
	@echo ""
	@echo "Hardware Configuration:"
	@grep "^#define.*PIN" include/hardware_config.h | head -20
	@echo ""
	@echo "APRS Configuration:"
	@grep "^#define DEFAULT" include/hardware_config.h
	@echo ""
	@echo "PlatformIO Environment:"
	@cat platformio.ini
	@echo ""

# Quick flash without build (if binary exists)
flash:
	@echo "Quick flashing existing binary..."
	pio run -t nobuild -t upload

# Erase flash
erase:
	@echo "Erasing ESP32 flash..."
	pio run -t erase

# Show memory usage
mem:
	@echo "Memory usage analysis..."
	pio run -t size

# Verify configuration
verify:
	@echo "Verifying project setup..."
	@echo ""
	@$(MAKE) check
	@echo ""
	@echo "Verifying library dependencies..."
	@pio lib list
	@echo ""
	@echo "✓ Verification complete"
