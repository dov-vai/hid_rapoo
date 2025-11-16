#!/bin/bash

set -e

MODULE_NAME="hid-rapoo"
MODULE_VERSION="1.0.0"

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (use sudo)"
    exit 1
fi

echo "Uninstalling ${MODULE_NAME} ${MODULE_VERSION} from DKMS..."

if lsmod | grep -q hid_rapoo; then
    echo "Unloading module..."
    rmmod hid_rapoo || true
fi

echo "Removing from DKMS..."
dkms remove -m "${MODULE_NAME}" -v "${MODULE_VERSION}" --all || true

echo "Removing source files..."
rm -rf "/usr/src/${MODULE_NAME}-${MODULE_VERSION}"

echo ""
echo "Uninstallation complete!"
