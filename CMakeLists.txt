cmake_minimum_required(VERSION 3.16)

project(Ermis VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Qt6Concurrent Qt6Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Concurrent Multimedia)

find_package(OpenSSL REQUIRED)
# Add the custom QCA installation to the prefix path
#set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/home/dharma/ssd/cpp/qca-qt6-custom")

# Find QCA
#find_package(Qca-qt6 REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)

    qt_add_executable(Ermis
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        passphrasedialog.h passphrasedialog.cpp
        stegengine.h stegengine.cpp
        constants.h constants.cpp
        audiostegengine.h audiostegengine.cpp
        audioplayer.h audioplayer.cpp
        ffmpeghandler.h ffmpeghandler.cpp
        resources.qrc
        helpmenudialog.cpp helpmenudialog.h
        donationdialog.cpp donationdialog.h
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Ermis APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Ermis SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Ermis
            ${PROJECT_SOURCES}
        )
    endif()
endif()

#target_link_libraries(Ermis PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
#target_link_libraries(Ermis PRIVATE Qt${QT_VERSION_MAJOR}::Widgets qca-qt6 OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(Ermis PRIVATE Qt${QT_VERSION_MAJOR}::Widgets OpenSSL::SSL OpenSSL::Crypto Qt6::Concurrent Qt6::Multimedia)
target_link_libraries(Ermis PRIVATE Qt6::Core)
target_link_libraries(Ermis PRIVATE Qt6::Core)
target_link_libraries(Ermis PRIVATE Qt6::Core)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Ermis)
endif()
set_target_properties(Ermis PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS Ermis
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Ermis)
endif()
