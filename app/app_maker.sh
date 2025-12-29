#!/usr/bin/env bash
set -e

# ----------------------------
# CONFIG
# ----------------------------
APPNAME=${1:-appResearchManager}           # The actual binary name
BUILD_DIR=../build/Desktop_Qt_6_10_1-Debug
APPDIR=AppDir

# ----------------------------
# 1. Detect build system
# ----------------------------
if [ -f "../CMakeLists.txt" ]; then
    BUILD_SYSTEM="cmake"
elif ls *.pro >/dev/null 2>&1; then
    BUILD_SYSTEM="qmake"
else
    echo "Error: No CMakeLists.txt or .pro file found."
    exit 1
fi

echo "==> Using build system: $BUILD_SYSTEM"

# ----------------------------
# 2. Build the application
# ----------------------------
# Use existing build directory
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory $BUILD_DIR does not exist"
    echo "Please build the project first using Qt Creator or CMake"
    exit 1
fi

echo "==> Using existing build from $BUILD_DIR"

# The built binary should be in build/
if [ ! -f "$BUILD_DIR/$APPNAME" ]; then
    echo "Error: Cannot find built binary: $BUILD_DIR/$APPNAME"
    echo "Rename your binary or update APPNAME variable."
    exit 1
fi

# ----------------------------
# 3. Prepare AppDir
# ----------------------------
echo "==> Creating AppDir structure"

rm -rf $APPDIR
mkdir -p $APPDIR/usr/bin

cp $BUILD_DIR/$APPNAME $APPDIR/usr/bin/

# Desktop entry and icon
ICON_SRC="../ResearchManager/images/ResearchManager.png"
ICON_FILE="$APPNAME.png"

if [ -f "$ICON_SRC" ]; then
    cp "$ICON_SRC" "$ICON_FILE"
else
    echo "[app_maker] Warning: $ICON_SRC not found; generating fallback icon" >&2
    python - <<'PY'
import base64, sys
png_b64 = (
    "iVBORw0KGgoAAAANSUhEUgAAAIwAAACMCAIAAAAj5C91AAAACXBIWXMAAAsTAAALEwEAmpwYAAA"
    "AGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAB59JREFUeNrsnQ1sHFcWxz+z3dp3"
    "27bt2rZt27ZtnYUKgqCwW0gD0ooFVqiQ0gPQSg0yBk0KEL6CgFCihVW1jNUEKqFJQ0hKwsn6dzbbv"
    "3szO7L3fW+fOnfPu++d3ZnZ2Z3ZnZt7zne/7nP+d3znfO+d3znV2w9tggDAYDBgMBgMGn4HcZ4Pr"
    "4cE3bbrfr08I5P40EZvUfq7rD2K4EcOa6EEeN0YHnCjO8n30L0U9e0O0WWbqGHKEDuHd7Hcl4Pmt"
    "vB6Dd3oHt+2p7rH5sDHmP0Q52LTP8sJYF9kTftn4QhwLg3HkB0LPe1Xd3pT0A+uPYXx8i/xpt3iXO"
    "CwBcVjljxqg7u5zEvQ2E8EwMRwXzLvCvBvAfCvk+0xg4V0eY48NwK6D6PJPL5WyD2FDjP1xhi7W1"
    "F7B1lQbhn1Fec1yFd0pBRBzT1xgYlv+3nqZb3wxJfW6vVgT0fJr5qzIHMH9lFvBrm6ap4rgXF5uN"
    "gx8yX3YhXDPB22ATs2iX7Ud6FwtoHRt1YLP8NKM+BnSR7Y8yB3A1y3lJwq+5SOyXOc/PlAj4WswK"
    "F3DC8u1AYQ80AXEWcTjFJvA+Hm4j4vE2dQD/A1y/94B+NZYp+wqzxdJX8B8dFYG/aV5u0kAMzXrc"
    "dF+Mf7Yk9WkqRCKRSCQSiUQikUgkEolEIpFIJBIJRL4HdAHxZfq5po4AAAAASUVORK5CYII="
)
with open("$ICON_FILE", "wb") as f:
    f.write(base64.b64decode(png_b64))
PY
fi

cat > $APPDIR/$APPNAME.desktop <<EOF
[Desktop Entry]
Type=Application
Name=ResearchManager
Exec=$APPNAME
Icon=$APPNAME
Categories=Utility;
EOF

# Resize icon to 256x256 (valid AppImage size) if ImageMagick is available
if command -v convert >/dev/null 2>&1; then
    convert "$ICON_FILE" -resize 256x256 $APPDIR/$APPNAME.png
else
    # Fallback: copy as-is
    cp "$ICON_FILE" $APPDIR/$APPNAME.png
fi

# ----------------------------
# 4. Download linuxdeploy + Qt plugin
# ----------------------------
echo "==> Downloading linuxdeploy tools"


if [ ! -f linuxdeploy-x86_64.AppImage ]; then
    wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

if [ ! -f linuxdeploy-plugin-qt-x86_64.AppImage ]; then
    wget -q https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
    chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
fi

# ----------------------------
# 5. Temporarily hide problematic SQL drivers
# ----------------------------
echo "==> Backing up problematic SQL drivers"
if [ -x /home/airlab/Qt/6.10.1/gcc_64/bin/qmake ]; then
    export QMAKE=/home/airlab/Qt/6.10.1/gcc_64/bin/qmake
    QT_PLUGINS_DIR=/home/airlab/Qt/6.10.1/gcc_64/plugins
elif [ -x /home/airlab/Qt/6.10.1/clang_64/bin/qmake ]; then
    export QMAKE=/home/airlab/Qt/6.10.1/clang_64/bin/qmake
    QT_PLUGINS_DIR=/home/airlab/Qt/6.10.1/clang_64/plugins
else
    echo "ERROR: qmake not found for Qt 6.10.1" >&2
    exit 1
fi

# Backup problematic drivers temporarily
BACKUP_DIR=$(mktemp -d)
echo "==> Temporarily moving problematic drivers to $BACKUP_DIR"
if [ -d "$QT_PLUGINS_DIR/sqldrivers" ]; then
    # Move all drivers except sqlite to backup
    for driver in "$QT_PLUGINS_DIR/sqldrivers"/libqsql*.so; do
        if [[ ! "$driver" =~ "sqlite" ]]; then
            mv "$driver" "$BACKUP_DIR/" 2>/dev/null || true
        fi
    done
fi

# ----------------------------
# 6. Run linuxdeploy + Qt plugin
# ----------------------------
echo "==> Running linuxdeploy"
export QT_QPA_PLATFORM=offscreen
export QML_SOURCES_PATHS=../ResearchManager/

./linuxdeploy-x86_64.AppImage \
    --appdir $APPDIR \
    --desktop-file $APPDIR/$APPNAME.desktop \
    --icon-file $APPDIR/$APPNAME.png \
    --plugin qt \
    --output appimage

# Restore backed up drivers
echo "==> Restoring SQL drivers"
if [ -d "$BACKUP_DIR" ] && [ "$(ls -A $BACKUP_DIR)" ]; then
    mv "$BACKUP_DIR"/*.so "$QT_PLUGINS_DIR/sqldrivers/" 2>/dev/null || true
fi
rm -rf "$BACKUP_DIR"

echo "==> AppImage created successfully!"
ls -lh ./*.AppImage
