
cmake_minimum_required(VERSION 3.16)
project(WavePaint LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic processing of moc, uic, rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(WavePaint
    src/main.cpp
    src/MainWindow.cpp
    src/WaveView.cpp
    src/SignalModel.cpp
    src/io/WaveVcdImporter.cpp
    src/io/WaveJsonIO.cpp
)

target_include_directories(WavePaint PRIVATE src src/io)

target_link_libraries(WavePaint PRIVATE Qt6::Widgets)
