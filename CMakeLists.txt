cmake_minimum_required(VERSION 3.16)
project(WavePaint LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Genera compile_commands.json para VS Code / clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable Qt's automatic processing of moc, uic, rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Buscar Qt6 Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Incluir carpetas de encabezados
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/io
    ${CMAKE_SOURCE_DIR}/src/ui
   # ${CMAKE_SOURCE_DIR}/src/utils
)

# Recolectar automáticamente todos los archivos fuente (.cpp, .h)
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

# Crear el ejecutable
add_executable(WavePaint ${SRC_FILES})

# Vincular con Qt6 Widgets
target_link_libraries(WavePaint PRIVATE Qt6::Widgets)
