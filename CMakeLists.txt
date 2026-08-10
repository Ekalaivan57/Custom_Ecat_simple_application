cmake_minimum_required(VERSION 3.10)
project(Custom_Ecat_simple_application C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler Warning Flags
add_compile_options(-Wall -Wextra -O2)

# Include Application Headers (Local include directory)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Application Source Files (Sources in code/ folder)
set(APP_SOURCES
    code/EthercatManager.c
    code/main.c
)

# Executable Target
add_executable(Custom_Ecat_App ${APP_SOURCES})

# Target Link Libraries (Linking custom ecat_api library)
target_link_libraries(Custom_Ecat_App
    PRIVATE
    ecat_api
    pthread
    m
)
