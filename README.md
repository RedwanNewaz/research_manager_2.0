# Research Manager

<p align="center">
  <img src="ResearchManager/images/ResearchManager.png" alt="Research Manager Logo" width="128" height="128">
</p>

<p align="center">
  <strong>A comprehensive desktop application for managing research projects, deadlines, contacts, and files</strong>
</p>

<p align="center">
  <a href="#features">Features</a> •
  <a href="#installation">Installation</a> •
  <a href="#building-from-source">Building</a> •
  <a href="#usage">Usage</a> •
  <a href="#architecture">Architecture</a>
</p>

---

## Overview

**Research Manager** is a cross-platform desktop application built with Qt 6 and QML that helps researchers, academics, and professionals organize their research projects efficiently. It provides a unified workspace for managing multiple research projects across different storage locations (local drives, OneDrive, Google Drive), tracking deadlines, managing contacts, and leveraging AI for productivity.

## Features

### 🗂️ Multi-Workspace Management

- **Multiple Workspace Support**: Manage research projects across different storage locations simultaneously
- **Cloud Storage Integration**: Seamlessly work with projects stored on:
  - Local drives (C:, D:, etc.)
  - Microsoft OneDrive
  - Google Drive
- **Quick Workspace Switching**: Switch between workspaces with a single click
- **Workspace Configuration**: Create, edit, and delete workspaces with custom database paths and icons

### 📁 Project Organization

- **Category-Based Organization**: Group projects by categories (papers, proposals, presentations, etc.)
- **Visual Project Grid**: Browse projects with a clean, visual folder grid interface
- **Project Templates**: Create new projects from predefined templates
- **Hierarchical File Browser**: Navigate project files with a tree-view folder structure
- **Split-View File Explorer**: Side-by-side folder tree and file list for efficient navigation

### 📅 Calendar & Deadline Management

- **Interactive Calendar View**: Visual monthly calendar with deadline indicators
- **Deadline Tracking**: Track important dates, submission deadlines, and milestones
- **Smart Deadline Parser**: Automatically extract and parse deadlines from text
- **Event Notifications**: View deadline details by clicking on calendar dates
- **Cross-Project Deadlines**: See all deadlines across your workspace in one view

### ✅ Task Management

- **Project-Specific Tasks**: Create and manage tasks for each project
- **Task Status Tracking**: Mark tasks as complete/incomplete
- **Quick Task Entry**: Add tasks with a simple text input
- **Persistent Storage**: Tasks are saved to the project database

### 🔗 Link Management

- **URL Bookmarking**: Save important research links per project
- **Automatic Website Detection**: Extracts and displays website names from URLs
- **Quick Link Access**: One-click to open saved links
- **Link Organization**: Manage and delete links as needed

### 👥 Contact Management

- **Research Contact Database**: Store collaborator and contact information
- **Contact Fields**:
  - Name
  - Affiliation/Institution
  - Email
  - Phone
  - Website
  - Zoom link
  - Profile photo
- **Editable Contact Cards**: Update contact information inline
- **Workspace-Scoped Contacts**: Contacts are organized per workspace

### 🤖 AI Integration (Gemini API)

- **AI-Powered PDF Renaming**: Automatically rename PDF files based on their content using Google's Gemini AI
- **Customizable Prompts**: Configure AI prompts for:
  - PDF renaming
  - Calendar event extraction
  - Task generation
- **API Key Management**: Securely store your Gemini API key
- **Test Interface**: Test AI prompts directly from the settings

### 📥 File Downloader

- **Direct URL Downloads**: Download files directly into your project folders
- **Progress Indicator**: Visual feedback during downloads
- **Auto-Path Configuration**: Downloads go to the current project directory
- **Support for Various File Types**: Download PDFs, documents, and other research materials

### 🎨 Modern User Interface

- **Dark Theme**: Easy on the eyes for extended research sessions
- **Responsive Design**: Adapts to different window sizes
- **Intuitive Navigation**: Side menu for quick access to all features
- **Native File Icons**: Display system file icons for easy file type recognition
- **Smooth Animations**: Polished UI transitions and interactions

## Screenshots

*Coming soon*

## Installation

### Pre-built Releases

Download the latest release for your platform from the [Releases](https://github.com/yourusername/ResearchManager/releases) page:

| Platform | Download |
|----------|----------|
| Windows (64-bit) | `ResearchManager-Windows-x64.zip` |
| Linux (64-bit) | `ResearchManager-Linux-x64.AppImage` |
| macOS (64-bit) | `ResearchManager-macOS-x64.dmg` |

### Windows Installation

1. Download `ResearchManager-Windows-x64.zip`
2. Extract to your preferred location
3. Run `appResearchManager.exe`

### Linux Installation

1. Download `ResearchManager-Linux-x64.AppImage`
2. Make it executable:
   ```bash
   chmod +x ResearchManager-Linux-x64.AppImage
   ```
3. Run:
   ```bash
   ./ResearchManager-Linux-x64.AppImage
   ```

### macOS Installation

1. Download `ResearchManager-macOS-x64.dmg`
2. Open the DMG file
3. Drag the app to your Applications folder
4. Run from Applications

## Building from Source

### Prerequisites

- **Qt 6.10.1+** with the following modules:
  - Qt Core
  - Qt Quick (QML)
  - Qt SQL
  - Qt Widgets
  - Qt Network
  - Qt PDF (optional)
- **CMake 3.16+**
- **C++17 compatible compiler**:
  - Windows: MinGW 13+ or MSVC 2019+
  - Linux: GCC 9+ or Clang 10+
  - macOS: Xcode 12+ (Apple Clang)

### Build Steps

```bash
# Clone the repository
git clone https://github.com/yourusername/ResearchManager.git
cd ResearchManager

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/gcc_64

# Build
cmake --build . --parallel

# Run
./appResearchManager
```

### Windows Build with Qt Creator

1. Open `CMakeLists.txt` in Qt Creator
2. Select your Qt 6 kit
3. Configure the project
4. Build and run

### Running Tests

```bash
cmake -B build -DBUILD_TEST=ON
cmake --build build
cd build
ctest --output-on-failure
```

## Usage

### First Launch

1. On first launch, you'll be prompted to configure your database path
2. Choose or create a configuration database (`common_config.db`)
3. The app will create necessary tables automatically

### Creating a Workspace

1. Go to **Settings** → **Workspace Manager**
2. Click **Create Workspace**
3. Fill in:
   - Workspace name (e.g., "Research 2025")
   - Database path
   - Root directory path
   - Select an icon
4. Click **Create**

### Managing Projects

1. Select a workspace from the side menu
2. Browse projects in the category grid
3. Click a project folder to open it
4. Use the file browser to navigate project contents

### Adding Deadlines

1. Open the **Calendar** tab in a project
2. Paste deadline text or manually add dates
3. The deadline parser will extract dates automatically
4. View deadlines highlighted on the calendar

### Using AI Features

1. Go to **Settings** → **AI Config**
2. Enter your Google Gemini API key
3. Customize prompts if needed
4. Test the connection
5. Use **AI Rename** on PDFs from the file context menu

## Architecture

### Technology Stack

| Layer | Technology |
|-------|------------|
| Frontend | Qt Quick (QML) |
| Backend | C++17 |
| Database | SQLite |
| Build System | CMake |
| AI Integration | Google Gemini API |

### Project Structure

```
ResearchManager/
├── Backend/                    # C++ backend classes
│   ├── applicationmanager.*    # App lifecycle management
│   ├── database.h              # Database abstraction
│   ├── workspacemodel.*        # Workspace data model
│   ├── projectpage.*           # Project page logic
│   ├── calendarview.*          # Calendar functionality
│   ├── deadlineparser.*        # Deadline text parsing
│   ├── taskmanger.*            # Task management
│   ├── linkviewer.*            # Link management
│   ├── contactsmodel.*         # Contacts database
│   ├── filedownloader.*        # Network file downloads
│   ├── aiconfig.*              # AI/Gemini integration
│   └── templatemanager.*       # Project templates
├── ResearchManager/            # QML UI files
│   ├── HomePage.qml            # Main dashboard
│   ├── HomeProjects.qml        # Project grid view
│   ├── RecentProjects.qml      # Project detail view
│   ├── Settings.qml            # Settings page
│   ├── ContactList.qml         # Contact management
│   ├── AiConfig.qml            # AI settings
│   ├── ProjectComponents/      # Reusable UI components
│   │   ├── CalendarView.qml
│   │   ├── TaskViewer.qml
│   │   ├── FileViewer.qml
│   │   ├── LinkView.qml
│   │   └── ...
│   └── SettingComponents/      # Settings sub-pages
├── Test/                       # Unit tests
├── CMakeLists.txt              # Build configuration
└── .github/workflows/          # CI/CD pipelines
```

### Database Schema

The application uses two SQLite databases:

**Configuration Database** (`common_config.db`):
- `workspaces` - Workspace definitions
- `templates` - Project templates
- `ai_config` - AI settings and prompts
- `contacts` - Contact information

**Research Database** (per workspace):
- `projects` - Project metadata
- `categories` - Project categories
- `tasks` - Project tasks
- `links` - Saved URLs
- `deadlines` - Deadline entries

## Configuration

### Settings Location

| Platform | Path |
|----------|------|
| Windows | `%APPDATA%/ResearchManager/` |
| Linux | `~/.config/ResearchManager/` |
| macOS | `~/Library/Application Support/ResearchManager/` |

### Environment Variables

| Variable | Description |
|----------|-------------|
| `RESEARCH_MANAGER_CONFIG` | Override config database path |

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

### Development Guidelines

- Follow Qt coding conventions
- Use meaningful commit messages
- Add tests for new features
- Update documentation as needed

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [Qt Framework](https://www.qt.io/)
- AI features powered by [Google Gemini](https://ai.google.dev/)
- Icons from various open-source icon sets

## Support

If you encounter any issues or have questions:

1. Check the [Issues](https://github.com/yourusername/ResearchManager/issues) page
2. Create a new issue with detailed information
3. Include your OS, Qt version, and steps to reproduce

---

<p align="center">
  Made with ❤️ for researchers everywhere
</p>
