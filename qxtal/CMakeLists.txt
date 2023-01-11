cmake_minimum_required(VERSION 3.5)

project(qxtal VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(ui ../shared/include ../shared/Classes/Util include)

set(PROJECT_SOURCES
        main.cc
        include/instructions.h
        include/notifications.h
        include/commands.h
        ui/mainwindow.cc
        ui/mainwindow.h
        ui/mainwindow.ui
        ui/memorywidget.h
        ui/memorywidget.cc
        ui/statewidget.h
        ui/statewidget.cc
        ui/heatmapwidget.h
        ui/heatmapwidget.cc
        ui/pointswidget.h
        ui/pointswidget.cc
        ui/tracewidget.h
        ui/tracewidget.cc
        ui/asmtextedit.h
        ui/asmtextedit.cc
        ui/vcrwidget.h
        ui/vcrwidget.cc
        sim/simulator.h
        sim/simulator.cc
        sim/mathpack.h
        sim/mathpack.cc
        sim/atarihw.h
        sim/atarihw.cc
        sim/atari.h
        sim/atari.cc
        sim/io.h
        sim/io.cc
        sim/display.h
        sim/display.cc
        sim/worker.h
        sim/worker.cc
        ../shared/Classes/Util/StringUtils.h
        ../shared/Classes/Util/StringUtils.cc
        ../shared/Classes/Util/NotifyCenter.h
        ../shared/Classes/Util/NotifyCenter.cc
        resources.qrc
    )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(qxtal
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET qxtal APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(qxtal SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(qxtal
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(qxtal PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(qxtal PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS qxtal
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(qxtal)
endif()
