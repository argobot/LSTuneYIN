cmake_minimum_required(VERSION 3.22)

project(LSTune
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Opciones de compilación para C++ y optimización de punto flotante para DSP
add_compile_options(-Wall -Wextra -O3 -ffast-math)

# Definiciones de plataforma heredadas
if(WIN32)
    add_compile_definitions(BUILD_WIN32=1 BUILD_LINUX=0)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(BUILD_WIN32=0 BUILD_LINUX=1)
endif()

# Automatización de herramientas de generación de código de Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Paquetes requeridos de Qt6
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Multimedia
)

# Fuentes del proyecto
set(SOURCES
    AudioIO.cpp
    filter.cc
    LED.cpp
    main.cpp
    noteGuesser.cc
    notes.cc
    sound.cc
    tuner.cpp
    wheel.cpp
)

set(HEADERS
    AudioIO.h
    filter.h
    LED.h
    noteGuesser.h
    notes.h
    sound.h
    tuner.h
    wheel.h
)

set(FORMS
    tunerForm.ui
)

# Declaración del ejecutable
add_executable(lstune
    ${SOURCES}
    ${HEADERS}
    ${FORMS}
)

# Enlace de bibliotecas
target_link_libraries(lstune PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    m
)

target_include_directories(lstune PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)