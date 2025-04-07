cmake_minimum_required(VERSION 3.14)

project(EdgeDetection VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

if (NOT Qt6_FOUND)
    message(FATAL_ERROR "Qt6 not found. Please install Qt6.")
endif()

# Find MPI
find_package(MPI REQUIRED)

if (NOT MPI_FOUND)
    message(FATAL_ERROR "MPI not found. Please install MPI.")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    main.cpp
    algorithms.cpp
)

set(HEADERS
    algorithms.h
)


# Add executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_FILES})

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets Qt6::Gui)

# Link MPI libraries
target_link_libraries(${PROJECT_NAME} MPI::MPI_CXX)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.EdgeDetection"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})