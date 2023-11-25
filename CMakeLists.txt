cmake_minimum_required(VERSION 3.5)

project(songsailor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

find_package(Qt6 REQUIRED COMPONENTS Multimedia) ##
find_package(Qt6 REQUIRED COMPONENTS Sql) ##

#TAGLIB
# instalação do taglib
set(TAGLIB_INSTALL_DIR "C:/Lib/taglib")

# adição das pastas de include e lib
include_directories(${TAGLIB_INSTALL_DIR}/include)
link_directories(${TAGLIB_INSTALL_DIR}/lib)

set(PROJECT_SOURCES
        main.cpp
        songsailor.cpp
        songsailor.h
        songsailor.ui

        src/music_downloader.py
        src/refresh_token.py
        src/config.json

        icons.qrc
        TODO.md
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(songsailor
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        README.md
        src/audioinfo.cpp src/audioinfo.h src/buttonhoverwatcher.cpp src/buttonhoverwatcher.h src/musicitem.cpp src/musicitem.h src/musiclistmodel.cpp src/musiclistmodel.h
        config.h config.cpp config.ui
        src/database.h src/database.cpp


    )

    #configuração para o cmake realizar a linkagem corretamente
    target_include_directories(songsailor PRIVATE ${TAGLIB_INSTALL_DIR}/include)
    target_link_libraries(songsailor PRIVATE tag) ##

# Define target properties for Android with Qt 6 as:
#    set_property(TARGET songsailor APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(songsailor SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(songsailor
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(songsailor PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(songsailor PRIVATE Qt6::Multimedia)
target_link_libraries(songsailor PRIVATE Qt6::Sql)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.songsailor)
endif()
set_target_properties(songsailor PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS songsailor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(songsailor)
endif()
