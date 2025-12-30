# ==============================================================================
# Project Configuration
# ==============================================================================
cmake_minimum_required(VERSION 3.16)
project(ResearchManager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==============================================================================
# Dependencies
# ==============================================================================
option(BUILD_TEST, "Build tests" OFF)

# Qt6 Components - Core Requirements
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Quick
    Sql
    Widgets
    Network
    Gui
)

# Qt6 Optional Components
find_package(Qt6 COMPONENTS Pdf QUIET)
find_package(Qt6 COMPONENTS Core5Compat QUIET)
find_package(Qt6 COMPONENTS ShaderTools QUIET)
find_package(Qt6 COMPONENTS Charts QUIET)
find_package(Qt6 COMPONENTS Multimedia QUIET)

# Qt6 Test component (only for tests)
if(BUILD_TEST)
    find_package(Qt6 REQUIRED COMPONENTS Test)
endif()

# ==============================================================================
# Qt Setup
# ==============================================================================
qt_standard_project_setup()
# Force SQLite only for QtSql
set(QT_FEATURE_sql_mysql OFF)
set(QT_FEATURE_sql_psql OFF)
set(QT_FEATURE_sql_mimer OFF)
# ==============================================================================
# Main Application Executable
# ==============================================================================

# Platform-specific source files
if(WIN32)
    set(APP_ICON_RESOURCE appicon.rc)
else()
    set(APP_ICON_RESOURCE "")
endif()

qt_add_executable(appResearchManager
    main.cpp
    Backend/applicationmanager.h
    Backend/applicationmanager.cpp
    Backend/homepage.h
    Backend/homepage.cpp
    Backend/database.h
    Backend/filedownloader.h
    Backend/filedownloader.cpp
    Backend/contactsmodel.h
    Backend/contactsmodel.cpp
    Backend/aiconfig.h
    Backend/aiconfig.cpp
    ${APP_ICON_RESOURCE}
)

# ==============================================================================
# QML Module Configuration
# ==============================================================================
qt_add_qml_module(appResearchManager
    URI ResearchManager

    # Main QML Files
    QML_FILES

        App.qml
        Workspace.qml

    # Project Components
    QML_FILES
        ResearchManager/ProjectComponents/CalendarView.qml
        ResearchManager/ProjectComponents/CitationView.qml
        ResearchManager/ProjectComponents/ContactsViewer.qml
        ResearchManager/ProjectComponents/DeadlineView.qml
        ResearchManager/ProjectComponents/FileViewer.qml
        ResearchManager/ProjectComponents/FolderViewer.qml
        ResearchManager/ProjectComponents/HeaderTile.qml
        ResearchManager/ProjectComponents/LinkView.qml
        ResearchManager/ProjectComponents/NavBar.qml
        ResearchManager/ProjectComponents/TabView.qml
        ResearchManager/ProjectComponents/TaskViewer.qml

    # UI Views
    QML_FILES
        ResearchManager/AiConfig.qml
        ResearchManager/ContactList.qml
        ResearchManager/FilesPanel.qml
        ResearchManager/HomePage.qml
        ResearchManager/HomeProjects.qml
        ResearchManager/RecentProjects.qml
        ResearchManager/Settings.qml
        ResearchManager/SideMenu.qml
        ResearchManager/SettingComponents/WorkspaceManager.qml
        ResearchManager/SettingComponents/CreateWorkspace.qml
        ResearchManager/SettingComponents/TemplateManager.qml

    # Resources - Images
    RESOURCES
        ResearchManager/images/ai-svgrepo-com.svg
        ResearchManager/images/back.svg
        ResearchManager/images/contact-card-svgrepo-com.svg
        ResearchManager/images/folder_generic.png
        ResearchManager/images/go_previous.png
        ResearchManager/images/go_previous_off.png
        ResearchManager/images/home-svgrepo-com.svg
        ResearchManager/images/images.txt
        ResearchManager/images/old-contact-card-svgrepo-com.svg
        ResearchManager/images/recent-svgrepo-com.svg
        ResearchManager/images/refresh.png
        ResearchManager/images/search.png
        ResearchManager/images/search.svg
        ResearchManager/images/settings-svgrepo-com.svg
        ResearchManager/images/ResearchManager.ico
        ResearchManager/images/ResearchManager.png
        ResearchManager/images/local-folder.svg
        ResearchManager/images/icons8-google-drive-240.svg
        ResearchManager/images/icons8-onedrive-240.svg
        ResearchManager/images/go_next.png
        ResearchManager/images/send.svg
        ResearchManager/images/delete.svg


    # QML Module Definitions
    RESOURCES
        ResearchManager/ProjectComponents/qmldir
        ResearchManager/qmldir

    # Source Files
    SOURCES
        Backend/projectpage.h Backend/projectpage.cpp
        Backend/taskmanger.h Backend/taskmanger.cpp
        Backend/fileexplorer.h Backend/fileexplorer.cpp
        Backend/filelistviewer.h Backend/filelistviewer.cpp
        Backend/linkviewer.h Backend/linkviewer.cpp
        Backend/calendarview.h Backend/calendarview.cpp
        Backend/backend.h
        Backend/deadlinemodel.h Backend/deadlinemodel.cpp
        Backend/deadlineparser.h Backend/deadlineparser.cpp
        Backend/createproject.h Backend/createproject.cpp
        Backend/workspacemodel.h Backend/workspacemodel.cpp
        Backend/templatemanager.h Backend/templatemanager.cpp
        QML_FILES ResearchManager/ProjectComponents/NoteViewer.qml
        RESOURCES ResearchManager/images/icons8-dropbox-64.png ResearchManager/images/icons8-google-drive-64.png


)

# ==============================================================================
# Application Target Properties
# ==============================================================================
set_target_properties(appResearchManager PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.airlabuno.ResearchManager
    MACOSX_BUNDLE_BUNDLE_NAME "Research Manager"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_EXECUTABLE_NAME appResearchManager
    MACOSX_BUNDLE_INFO_STRING "Research Manager - Research and project management tool"
    MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_COPYRIGHT "Copyright © 2025 AiRLab-UNO"
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Set application icon for Linux
if(UNIX AND NOT APPLE)
    install(FILES ${CMAKE_SOURCE_DIR}/ResearchManager/images/ResearchManager.png
            DESTINATION share/icons/hicolor/256x256/apps
            RENAME ResearchManager.png)
endif()

# ==============================================================================
# Application Linking
# ==============================================================================
target_link_libraries(appResearchManager
    PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::Sql
        Qt6::Widgets
        Qt6::Network
        Qt6::Gui
)

# Link optional Qt components if available
if(TARGET Qt6::Pdf)
    target_link_libraries(appResearchManager PRIVATE Qt6::Pdf)
    target_compile_definitions(appResearchManager PRIVATE HAS_QT_PDF)
    message(STATUS "Qt6::Pdf found - PDF support enabled")
else()
    message(STATUS "Qt6::Pdf not found - PDF support disabled")
endif()

if(TARGET Qt6::Core5Compat)
    target_link_libraries(appResearchManager PRIVATE Qt6::Core5Compat)
    message(STATUS "Qt6::Core5Compat found - Qt5 compatibility enabled")
else()
    message(STATUS "Qt6::Core5Compat not found - continuing without Qt5 compatibility")
endif()

if(TARGET Qt6::ShaderTools)
    target_link_libraries(appResearchManager PRIVATE Qt6::ShaderTools)
    message(STATUS "Qt6::ShaderTools found")
endif()

if(TARGET Qt6::Charts)
    target_link_libraries(appResearchManager PRIVATE Qt6::Charts)
    message(STATUS "Qt6::Charts found - chart widgets enabled")
endif()

if(TARGET Qt6::Multimedia)
    target_link_libraries(appResearchManager PRIVATE Qt6::Multimedia)
    message(STATUS "Qt6::Multimedia found - multimedia support enabled")
endif()

# Copy Qt DLLs to test executable directory on Windows
if (WIN32)
    add_custom_command(TARGET appResearchManager POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
                --qmldir "${CMAKE_CURRENT_SOURCE_DIR}"
                "$<TARGET_FILE:appResearchManager>"
    )
endif()


# ==============================================================================
# Test Executable
# ==============================================================================
if(BUILD_TEST)
    # Google Test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    # For Windows: avoid overriding parent compiler flags
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    macro(add_qt_gtest_executable TARGET_NAME)
        # The macro takes the target name as the first argument,
        # followed by all source files passed as extra arguments (${ARGN})

        add_executable(${TARGET_NAME} ${ARGN})

        target_include_directories(${TARGET_NAME}
            PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/Backend
        )

        target_link_libraries(${TARGET_NAME}
            PRIVATE
                Qt6::Core
                Qt6::Sql
                Qt6::Test
                Qt6::Network
                GTest::gtest
                GTest::gmock
        )
    endmacro()

    set(DATABASE_TEST_SRC Test/tst_database.cpp Backend/database.h)
    set(NETWORK_TEST_SRC Test/test_NetwrokManager.cpp)
    set(DEADLINE_TEST_SRC Test/test_DeadlineParser.cpp Backend/deadlineparser.cpp)

    # Use the macro to create the executables
    add_qt_gtest_executable(DatabaseManagerTest ${DATABASE_TEST_SRC})
    add_qt_gtest_executable(NetworkManagerTest ${NETWORK_TEST_SRC})
    add_qt_gtest_executable(DeadlineParserTest ${DEADLINE_TEST_SRC})

    # Copy Qt DLLs to test executable directory on Windows
    if(WIN32)
        add_custom_command(TARGET DatabaseManagerTest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:Qt6::Core>
                $<TARGET_FILE:Qt6::Sql>
                $<TARGET_FILE:Qt6::Test>
                $<TARGET_FILE:Qt6::Network>
                $<TARGET_FILE_DIR:DatabaseManagerTest>
        )
    endif()

    # ==============================================================================
    # Installation
    # ==============================================================================
    include(GNUInstallDirs)

    install(TARGETS appResearchManager
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # ==============================================================================
    # Testing
    # ==============================================================================
    enable_testing()
    add_test(NAME DatabaseManager COMMAND DatabaseManagerTest)
    add_test(NAME NetworkManager COMMAND NetworkManagerTest)
    add_test(NAME DeadlineParser COMMAND DeadlineParserTest)
endif()
