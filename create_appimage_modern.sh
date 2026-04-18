#!/usr/bin/bash

# =============================================================================
# KONFIGURATION
# =============================================================================
APP_NAME="depdiscover"
PROJECT_DIR=$(pwd)
BUILD_DIR="$PROJECT_DIR/build_appimage"
APP_DIR="$BUILD_DIR/AppDir"
ICON_SOURCE="$PROJECT_DIR/resources/app_icon.png"
DESKTOP_FILE="$PROJECT_DIR/resources/$APP_NAME.desktop"

# Check Voraussetzungen
for tool in patchelf file conan wget cmake; do
    if ! command -v $tool &> /dev/null; then
        echo "FEHLER: '$tool' wurde nicht gefunden. Bitte installieren."
        exit 1
    fi
done

# Check Conan version
CONAN_VER=$(conan --version | grep -oE '[0-9]+\.[0-9]+' | head -1)
if (( $(echo "$CONAN_VER < 2.0" | bc -l) )); then
    echo "FEHLER: Conan Version 2.0+ erforderlich (gefunden: $CONAN_VER)"
    exit 1
fi

echo "--- Starte AppImage Erstellung (Modern Method) ---"

# =============================================================================
# 1. BUILD UMGEBUNG VORBEREITEN
# =============================================================================
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# =============================================================================
# 2. CONAN DEPENDENCIES
# =============================================================================
echo "--- Conan Install ---"
conan install "$PROJECT_DIR" \
    --output-folder=. \
    --build=missing \
    -s build_type=Release \
    -s compiler.cppstd=23

if [ $? -ne 0 ]; then
    echo "FEHLER: Conan install fehlgeschlagen!"
    exit 1
fi

# =============================================================================
# 3. KOMPILIEREN
# =============================================================================
echo "--- CMake Konfiguration ---"
cmake "$PROJECT_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

echo "--- Build ---"
cmake --build . -j$(nproc)

if [ $? -ne 0 ]; then
    echo "FEHLER: Build fehlgeschlagen!"
    exit 1
fi

# =============================================================================
# 4. APPDIR STRUKTUR VORBEREITEN
# =============================================================================
echo "--- Installiere in AppDir ---"
# CMake installiert alles nach AppDir/usr/...
DESTDIR="$APP_DIR" cmake --install .

# Ordnerstruktur sicherstellen
mkdir -p "$APP_DIR/usr/share/applications"
mkdir -p "$APP_DIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$APP_DIR/usr/bin"

# Metadaten & Bin kopieren (falls install nicht alles erwischt hat)
cp "$DESKTOP_FILE" "$APP_DIR/usr/share/applications/"
cp "$ICON_SOURCE" "$APP_DIR/usr/share/icons/hicolor/256x256/apps/$APP_NAME.png"

# Falls cmake --install die Binary nicht in usr/bin abgelegt hat (z.B. fehlendes install-Target)
if [ ! -f "$APP_DIR/usr/bin/$APP_NAME" ]; then
    if [ -f "$APP_NAME" ]; then
        cp "$APP_NAME" "$APP_DIR/usr/bin/"
    fi
fi

# Optional: Bereinigen von Conan-Metadaten im AppDir, falls diese mitinstalliert wurden
find "$APP_DIR" -name "*.cmake" -delete
find "$APP_DIR" -name "*.pc" -delete

# =============================================================================
# 5. LINUXDEPLOY & QT PLUGIN LADEN
# =============================================================================
# Basis-Tool
if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
    echo "--- Lade linuxdeploy herunter ---"
    wget --no-check-certificate -c "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    if [ $? -ne 0 ]; then
        echo "FEHLER: Download von linuxdeploy fehlgeschlagen!"
        exit 1
    fi
    chmod +x linuxdeploy-x86_64.AppImage
fi

# =============================================================================
# 6. APPIMAGE GENERIEREN
# =============================================================================
echo "--- Generiere AppImage ---"

# Environment Variablen setzen
export LD_LIBRARY_PATH="$APP_DIR/usr/lib:$LD_LIBRARY_PATH"

# Das Tool aufrufen
./linuxdeploy-x86_64.AppImage \
    --appdir "$APP_DIR" \
    --executable "$APP_DIR/usr/bin/$APP_NAME" \
    --desktop-file "$DESKTOP_FILE" \
    --icon-file "$ICON_SOURCE" \
    --output appimage

if [ $? -eq 0 ]; then
    echo "------------------------------------------------"
    echo "ERFOLG! AppImage erstellt:"
    ls -lh *.AppImage
    echo "------------------------------------------------"
else
    echo "FEHLER: linuxdeploy ist fehlgeschlagen."
    exit 1
fi
