#!/bin/bash

set -e

MODULE_NAME="hid-rapoo"
MODULE_VERSION="1.0.0"
DKMS_DIR="/usr/src/${MODULE_NAME}-${MODULE_VERSION}"

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (use sudo)"
    exit 1
fi

if ! command -v dkms &> /dev/null; then
    echo "DKMS is not installed. Please install it first:"
    echo "  Fedora/RHEL: sudo dnf install dkms"
    echo "  Ubuntu/Debian: sudo apt install dkms"
    echo "  Arch: sudo pacman -S dkms"
    exit 1
fi

echo "Installing ${MODULE_NAME} ${MODULE_VERSION} with DKMS..."

echo "Copying source files to ${DKMS_DIR}..."
mkdir -p "${DKMS_DIR}"
cp -r hid_rapoo.c Makefile dkms.conf "${DKMS_DIR}/"

echo "Adding module to DKMS..."
dkms add -m "${MODULE_NAME}" -v "${MODULE_VERSION}" || true

echo "Building module..."
dkms build -m "${MODULE_NAME}" -v "${MODULE_VERSION}"

echo "Installing module..."
dkms install -m "${MODULE_NAME}" -v "${MODULE_VERSION}"

echo "Loading module..."
modprobe hid_rapoo

echo ""
echo "Installation complete!"
echo "The module will automatically load on boot and after kernel updates."
echo ""
echo "To check module status: lsmod | grep hid_rapoo"
echo "To view module info: modinfo hid_rapoo"
