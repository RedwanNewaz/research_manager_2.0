#!/bin/bash

# --- Configuration ---
APP_NAME="ResearchManager"
APPIMAGE_PATH="ResearchManager-x86_64.AppImage" # Update this to your actual file path
REPO_OWNER="RedwanNewaz"   # Change this
REPO_NAME="research_manager_2.0"

# --- 1. Determine Version (Logic from your template) ---
# Check if a tag is provided as an argument, otherwise generate a rolling version
if [ -n "$1" ]; then
    VERSION="$1"
else
    VERSION="linux-0.5.1-$(date +%Y%m%d-%H%M%S)"
fi

echo "🚀 Preparing release for version: $VERSION"

# --- 2. Check if AppImage exists ---
if [ ! -f "$APPIMAGE_PATH" ]; then
    echo "❌ Error: AppImage not found at $APPIMAGE_PATH"
    exit 1
fi

# --- 3. Create Release Body (Logic from your template) ---
RELEASE_BODY="## Research Manager $VERSION

### What's New
#### Platform Improvements
- ✅ **Linux**: Fixed file manager integration (Nemo, Nautilus, Dolphin, etc.)
- ✅ **All Platforms**: Fixed \"Open File\" to use default applications

#### Build System
- Updated CMakeLists.txt for better Qt 6.10.1 compatibility
- Improved AppImage packaging for Linux

---
### Installation (Linux)
1. Download \`ResearchManager-Linux-x86_64.AppImage\`
2. Make it executable: \`chmod +x ResearchManager-Linux-x86_64.AppImage\`
3. Run: \`./ResearchManager-Linux-x86_64.AppImage\`"

# --- 4. Push to GitHub ---
echo "📦 Uploading to GitHub..."

# Create the release. If it exists, it will fail, so we use 'gh release view' to check or 'gh release upload'
gh release create "$VERSION" "$APPIMAGE_PATH" \
    --title "Research Manager $VERSION" \
    --notes "$RELEASE_BODY" \
    --repo "$REPO_OWNER/$REPO_NAME"

if [ $? -eq 0 ]; then
    echo "✅ Success! Release $VERSION is live."
else
    echo "⚠️ Release might already exist. Attempting to upload asset only..."
    gh release upload "$VERSION" "$APPIMAGE_PATH" --clobber --repo "$REPO_OWNER/$REPO_NAME"
fi