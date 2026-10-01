cmake_minimum_required(VERSION 3.21)

project(PLC_Modbus
    VERSION 1.0
    LANGUAGES CXX
)

# ============================================================
# C++ SETTINGS
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# ============================================================
# SLINT
# ============================================================

find_package(Slint REQUIRED)


# ============================================================
# PKG CONFIG
# ============================================================

find_package(PkgConfig REQUIRED)


# ============================================================
# LIBMODBUS
# ============================================================

pkg_check_modules(MODBUS REQUIRED libmodbus)


# ============================================================
# APPLICATION
# ============================================================

add_executable(PLC_Modbus
    main.cpp
)


# ============================================================
# SLINT SOURCE
# ============================================================

slint_target_sources(
    PLC_Modbus
    main.slint
)


# ============================================================
# LIBMODBUS INCLUDE DIRECTORIES
# ============================================================

target_include_directories(
    PLC_Modbus
    PRIVATE
    ${MODBUS_INCLUDE_DIRS}
)


# ============================================================
# LIBMODBUS COMPILE OPTIONS
# ============================================================

target_compile_options(
    PLC_Modbus
    PRIVATE
    ${MODBUS_CFLAGS_OTHER}
)


# ============================================================
# LINK LIBRARIES
# ============================================================

target_link_libraries(
    PLC_Modbus
    PRIVATE
    Slint::Slint
    ${MODBUS_LIBRARIES}
)


# ============================================================
# COPY LIBMODBUS DLL
# ============================================================

add_custom_command(
    TARGET PLC_Modbus
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_if_different

        "C:/Users/MicroApt/vcpkg/installed/x64-windows/bin/modbus-5.dll"

        "$<TARGET_FILE_DIR:PLC_Modbus>/modbus-5.dll"
)


# ============================================================
# COPY SLINT DLL
# ============================================================

add_custom_command(
    TARGET PLC_Modbus
    POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_if_different

        "C:/Program Files/Slint-cpp 1.18.1/lib/slint_cpp.dll"

        "$<TARGET_FILE_DIR:PLC_Modbus>/slint_cpp.dll"
)
