cmake_minimum_required(VERSION 3.21)
project(PRICOL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Поиск Qt6 ---
# CMAKE_PREFIX_PATH = C:/Qt/6.11.2/mingw_64
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# --- Исходники ---
add_executable(PRICOL
        main.cpp
        Globals.cpp
        MainWindow.cpp
        Settings.cpp
        PowerMonitor.cpp
        Notifications.cpp
        Reports.cpp
        SettingsDialog.cpp
        Globals.h
        Globals.cpp
        Notifications.h
        Notifications.cpp
        MainWindow.h
        MainWindow.cpp
        MainWindow.cpp
        Settings.h
        Settings.cpp
        PowerMonitor.h
        PowerMonitor.cpp
        Reports.h
        Reports.cpp
)

# --- Заголовки ---
target_sources(PRICOL PRIVATE
        Globals.h
        MainWindow.h
        Settings.h
        PowerMonitor.h
        Notifications.h
        Reports.h
        SettingsDialog.h
)

# --- Линковка: WinAPI + Qt ---
target_link_libraries(PRICOL PRIVATE
        shell32
        user32
        gdi32
        advapi32
        powrprof
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

# --- WIN32: точка входа wWinMain ---
set_target_properties(PRICOL PROPERTIES WIN32_EXECUTABLE TRUE)

if (MINGW)
    target_link_options(PRICOL PRIVATE -municode)
endif ()

# --- Копирование Qt DLL рядом с .exe ---
add_custom_command(TARGET PRICOL POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:PRICOL>
        $<TARGET_FILE_DIR:PRICOL>
        COMMAND_EXPAND_LISTS
)

# --- Плагин qwindows.dll через windeployqt ---
if (WIN32)
    add_custom_command(TARGET PRICOL POST_BUILD
            COMMAND "C:/Qt/6.11.2/mingw_64/bin/windeployqt.exe"
            --no-translations
            --no-compiler-runtime
            $<TARGET_FILE:PRICOL>
            COMMENT "Running windeployqt..."
    )
endif ()