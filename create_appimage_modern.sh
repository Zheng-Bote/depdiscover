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

# QMake (wird für das Plugin benötigt, um Qt-Pfade zu finden)
#export QMAKE="qmake6"
#export QMAKE="${QMAKE:-qmake6}"

# Check Voraussetzungen
for tool in patchelf file; do
    if ! command -v $tool &> /dev/null; then
        echo "FEHLER: '$tool' wurde nicht gefunden. Bitte installieren."
        exit 1
    fi
done

echo "--- Starte AppImage Erstellung (Modern Method) ---"

# =============================================================================
# 1. BUILD UMGEBUNG VORBEREITEN
# =============================================================================
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# =============================================================================
# 2. KOMPILIEREN
# =============================================================================
echo "--- CMake Konfiguration ---"
cmake "$PROJECT_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr

echo "--- Build ---"
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "FEHLER: Build fehlgeschlagen!"
    exit 1
fi

# =============================================================================
# 3. APPDIR STRUKTUR VORBEREITEN
# =============================================================================
echo "--- Installiere in AppDir ---"
make install DESTDIR="$APP_DIR"

# Ordnerstruktur für Desktop/Icons sicherstellen
mkdir -p "$APP_DIR/usr/share/applications"
mkdir -p "$APP_DIR/usr/share/icons/hicolor/256x256/apps"

# Metadaten kopieren
cp "$DESKTOP_FILE" "$APP_DIR/usr/share/applications/"
cp "$ICON_SOURCE" "$APP_DIR/usr/share/icons/hicolor/256x256/apps/$APP_NAME.png"

# =============================================================================
# 4. LINUXDEPLOY & QT PLUGIN LADEN
# =============================================================================
# Basis-Tool
if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
    wget -q "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    chmod +x linuxdeploy-x86_64.AppImage
fi

# Qt-Plugin
#if [ ! -f "linuxdeploy-plugin-qt-x86_64.AppImage" ]; then
#    wget -q "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
#    chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
#fi

# =============================================================================
# 5. APPIMAGE GENERIEREN
# =============================================================================
echo "--- Generiere AppImage ---"

# Environment Variablen für das Qt Plugin setzen
export LD_LIBRARY_PATH="$APP_DIR/usr/lib:$LD_LIBRARY_PATH"

# Das Tool aufrufen
# --appdir: Wo liegt die App?
# --output appimage: Erstelle am Ende die fertige Datei
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