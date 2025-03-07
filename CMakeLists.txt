cmake_minimum_required(VERSION 3.24...3.26)

set(CMAKE_C_COMPILER "cl")
set(CMAKE_CXX_COMPILER "cl")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT "Use Qt functionality" ON)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_qt(COMPONENTS Widgets Core Svg Network Xml)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt::Core Qt::Widgets Qt::Svg Qt::Network Qt::Xml)
  target_compile_options(
    ${CMAKE_PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header
                                  -Wno-comma>)
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON
               AUTOUIC ON
               AUTORCC ON)
endif()

target_sources(${CMAKE_PROJECT_NAME} PRIVATE 
  src/plugin-main.cpp
  src/booking-handler.cpp
  src/text-handler.cpp
  src/mail-handler.cpp
  src/storage-handler.cpp
  src/json-parser.cpp
  src/animated-dialog.cpp
  src/fullscreen-dialog.cpp
  src/message-dialog.cpp
  src/booking-editor.cpp
  src/booking-manager.cpp
  src/quick-record.cpp
  src/login.cpp
  src/login-dialog.cpp
  src/mode-select.cpp
  src/time-observer.cpp
  src/source-record.c
  src/login.ui
  src/login-dialog.ui
  src/mode-select.ui
  src/booking-editor.ui
  src/booking-manager.ui
  src/quick-record.ui
  src/ptz-controls.ui
  src/settings.ui
  src/login-dialog.qrc
  src/ptz-controls.qrc
  src/ptz-controls.cpp
  src/ptz-device.cpp
  src/settings.cpp
  src/ptz-onvif.cpp
  src/ptz-visca.cpp
  src/ptz-visca-udp.cpp
  src/ptz-visca-tcp.cpp
  src/protocol-helpers.cpp
  src/ptz-action-source.c
  src/circularlistview.cpp
  src/touch-control.cpp
  src/widgets.cpp
  src/imported/qt-wrappers.cpp
  src/imported/obs-app.cpp
  src/imported/properties-view.cpp
  src/imported/vertical-scroll-area.cpp
  src/imported/double-slider.cpp
  src/imported/slider-ignorewheel.cpp
  src/imported/spinbox-ignorewheel.cpp
  src/booking-handler.hpp
  src/text-handler.hpp
  src/mail-handler.hpp
  src/storage-handler.hpp
  src/login.hpp
  src/login-dialog.hpp
  src/mode-select.hpp
  src/time-observer.hpp
  src/animated-dialog.hpp
  src/fullscreen-dialog.hpp
  src/message-dialog.hpp
  src/booking-editor.hpp
  src/booking-manager.hpp
  src/quick-record.hpp
  src/json-parser.hpp
  src/ptz-controls.hpp
  src/ptz-device.hpp
  src/settings.hpp
  src/ptz-onvif.hpp
  src/ptz-visca.hpp
  src/ptz-visca-udp.hpp
  src/ptz-visca-tcp.hpp
  src/protocol-helpers.hpp
  src/circularlistview.hpp
  src/touch-control.hpp
  src/widgets.hpp
  src/styles.hpp
  src/imported/qt-wrappers.hpp
  src/imported/obs-app.hpp
  src/imported/properties-view.hpp
  src/imported/properties-view.moc.hpp
  src/imported/vertical-scroll-area.hpp
  src/imported/double-slider.hpp
  src/imported/slider-ignorewheel.hpp
  src/imported/spinbox-ignorewheel.hpp)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
