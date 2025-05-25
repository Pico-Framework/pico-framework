# framework_application.cmake
# Shared CMake logic for all framework-based applications

# ----------------------------------------------------------------------------------
# Precondition
# ----------------------------------------------------------------------------------

if(NOT DEFINED APP_SOURCES)
    message(FATAL_ERROR "APP_SOURCES must be defined before including framework_application.cmake")
endif()

# ----------------------------------------------------------------------------------
# Basic setup
# ----------------------------------------------------------------------------------

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ----------------------------------------------------------------------------------
# Required FreeRTOS
# ----------------------------------------------------------------------------------

if(NOT FREERTOS_KERNEL_PATH AND NOT DEFINED ENV{FREERTOS_KERNEL_PATH})
    message(FATAL_ERROR "FREERTOS_KERNEL_PATH is not set.")
endif()

include(${CMAKE_SOURCE_DIR}/../../cmake/FreeRTOS_Kernel_import.cmake)

# ----------------------------------------------------------------------------------
# Add framework first so its dependencies are resolved before app is built
# ----------------------------------------------------------------------------------

set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_SOURCE_DIR}/../../framework/include/port)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../framework pico_framework)

# ----------------------------------------------------------------------------------
# Declare the executable from APP_SOURCES (after framework is fully defined)
# ----------------------------------------------------------------------------------

add_executable(${APP_NAME} ${APP_SOURCES})

# ----------------------------------------------------------------------------------
# LittleFS offset and linker fragment (generated if feature is enabled)
# ----------------------------------------------------------------------------------

set(HAS_VALID_LFS_CONFIG FALSE)

if(PICO_HTTP_ENABLE_LITTLEFS)

    # Define flash size per board
    if(${PICO_BOARD} STREQUAL "pico2_w")
        set(FLASH_TOTAL_SIZE 0x400000)  # 4MB
        set(PICO_TARGET_CFG_FILE "rp2350.cfg")
    elseif(${PICO_BOARD} STREQUAL "pico_w")
        set(FLASH_TOTAL_SIZE 0x200000)  # 2MB
        set(PICO_TARGET_CFG_FILE "rp2040.cfg")
    else()
        message(FATAL_ERROR "Unsupported board: ${PICO_BOARD}")
    endif()

    # Compute absolute flash region
    math(EXPR LFS_FLASH_END "0x10000000 + ${FLASH_TOTAL_SIZE}")
    math(EXPR LFS_FLASH_START "${LFS_FLASH_END} - ${LFS_PARTITION_SIZE}")

    # Convert to proper hex strings
    execute_process(
        COMMAND printf "0x%08X" ${LFS_FLASH_START}
        OUTPUT_VARIABLE LFS_FLASH_START_HEX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND printf "0x%X" ${LFS_PARTITION_SIZE}
        OUTPUT_VARIABLE LFS_PARTITION_SIZE_HEX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND printf "0x%08X" ${LFS_FLASH_END}
        OUTPUT_VARIABLE LFS_FLASH_END_HEX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Generate the linker fragment
    set(GENERATED_LD "${CMAKE_BINARY_DIR}/generated_lfs_fragment.ld")
    file(WRITE ${GENERATED_LD}
        "MEMORY {\n"
        "  LFS (r) : ORIGIN = ${LFS_FLASH_START_HEX}, LENGTH = ${LFS_PARTITION_SIZE_HEX}\n"
        "}\n"
        "__flash_lfs_start = ORIGIN(LFS);\n"
        "__flash_lfs_end = ORIGIN(LFS) + LENGTH(LFS);\n"
    )

    message(STATUS "Using generated linker fragment: ${GENERATED_LD}")
    message(STATUS "Using LittleFS start: ${LFS_FLASH_START_HEX}")
    message(STATUS "Using LittleFS end:   ${LFS_FLASH_END_HEX}")

    set(MEMMAP_FRAGMENT ${GENERATED_LD})
    set(LFS_FLASH_OFFSET ${LFS_FLASH_START_HEX})
    set(HAS_VALID_LFS_CONFIG TRUE)

    # Optionally build the image if html/ exists
    set(MKLITTLEFS_TOOL "${CMAKE_SOURCE_DIR}/../../tools/mklittlefs/mklittlefs")
    set(LFS_IMAGE "${CMAKE_BINARY_DIR}/littlefs.img")
    set(LFS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/html")

    if(EXISTS ${LFS_SOURCE_DIR})
        file(GLOB_RECURSE HTML_FILES ${LFS_SOURCE_DIR}/*)

        add_custom_command(
            OUTPUT ${LFS_IMAGE}
            COMMAND ${CMAKE_COMMAND} -E echo "Building LittleFS image..."
            COMMAND ${MKLITTLEFS_TOOL}
                    -c ${LFS_SOURCE_DIR}
                    -b 4096 -p 256 -s ${LFS_PARTITION_SIZE}
                    ${LFS_IMAGE}
            DEPENDS ${HTML_FILES}
            COMMENT "Generating ${LFS_IMAGE} from ${LFS_SOURCE_DIR}"
        )

        add_custom_target(build_lfs_image ALL DEPENDS ${LFS_IMAGE})
    else()
        message(STATUS "[framework] html/ not found, skipping LFS image build but keeping linker symbols.")
    endif()
endif()

# ----------------------------------------------------------------------------------
# Includes
# ----------------------------------------------------------------------------------

target_include_directories(${APP_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/../../framework/include
    ${CMAKE_SOURCE_DIR}/../../framework/include/port
)

if(${PICO_BOARD} STREQUAL "pico2_w")
    target_include_directories(${APP_NAME} PRIVATE ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2350_ARM_NTZ/non_secure/include)
else()
    target_include_directories(${APP_NAME} PRIVATE ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include)
endif()

# ----------------------------------------------------------------------------------
# Linking and linker fragment
# ----------------------------------------------------------------------------------

target_link_libraries(${APP_NAME}
    pico_framework
)

if(PICO_HTTP_ENABLE_LITTLEFS)
    message(STATUS "Linking with MEMMAP_FRAGMENT: ${MEMMAP_FRAGMENT}")
    if(DEFINED MEMMAP_FRAGMENT)
        target_link_options(${APP_NAME} PRIVATE "-T${MEMMAP_FRAGMENT}")
    endif()
endif()

# ----------------------------------------------------------------------------------
# Compiler definitions
# ----------------------------------------------------------------------------------

target_compile_definitions(${APP_NAME} PUBLIC
    WIFI_SSID=\"${WIFI_SSID}\"
    WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
    JWT_SECRET=\"${JWT_SECRET_VALUE}\"
    CYW43_HOST_NAME="Pico-Framework"
    ALTCP_MBEDTLS_AUTHMODE=MBEDTLS_SSL_VERIFY_REQUIRED
    NO_SYS=0
)

target_compile_options(${APP_NAME} PRIVATE
    -Wno-unused-result
    -Wno-psabi
)

# ----------------------------------------------------------------------------------
# Flash Target
# ----------------------------------------------------------------------------------

if(NOT TARGET flash_all)
    if(HAS_VALID_LFS_CONFIG)
        add_custom_target(flash_all DEPENDS ${APP_NAME} build_lfs_image)
    else()
        add_custom_target(flash_all DEPENDS ${APP_NAME})
    endif()

    if(${FLASH_METHOD} STREQUAL "usb")
        add_custom_command(TARGET flash_all POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Flashing via USB..."
            COMMAND picotool load ${CMAKE_BINARY_DIR}/${APP_NAME}.uf2
            COMMAND ${CMAKE_COMMAND} -E echo "Flashing LittleFS image..."
            COMMAND ${CMAKE_COMMAND} -E copy ${LFS_IMAGE} ${CMAKE_BINARY_DIR}/littlefs.bin
            COMMAND picotool load ${CMAKE_BINARY_DIR}/littlefs.bin --offset ${LFS_FLASH_OFFSET}
        )
    else()
        set(PICO_OPENOCD_EXECUTABLE "$ENV{HOME}/.pico-sdk/openocd/openocd")
        set(PICO_OPENOCD_SCRIPTS "$ENV{HOME}/.pico-sdk/openocd/scripts")

        add_custom_command(TARGET flash_all POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Flashing via OpenOCD..."
            COMMAND ${PICO_OPENOCD_EXECUTABLE}
                    -f ${PICO_OPENOCD_SCRIPTS}/interface/cmsis-dap.cfg
                    -f ${PICO_OPENOCD_SCRIPTS}/target/${PICO_TARGET_CFG_FILE}
                    -c "adapter speed 5000"
                    -c "init"
                    -c "reset init"
                    -c "program ${CMAKE_BINARY_DIR}/${APP_NAME}.elf verify"
                    -c "flash write_image erase ${LFS_IMAGE} ${LFS_FLASH_OFFSET} bin"
                    -c "verify_image ${LFS_IMAGE} ${LFS_FLASH_OFFSET}"
                    -c "resume"
                    -c "exit"
        )
    endif()
endif()

# ----------------------------------------------------------------------------------
# Clean Targets
# ----------------------------------------------------------------------------------

add_custom_target(clean_fs
    COMMAND ${CMAKE_COMMAND} -E remove -f ${LFS_IMAGE}
    COMMENT "Cleaning LittleFS image"
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E echo "Cleaning all build outputs..."
    COMMAND ${CMAKE_COMMAND} -E remove -f
        ${CMAKE_BINARY_DIR}/${APP_NAME}.uf2
        ${CMAKE_BINARY_DIR}/${APP_NAME}.elf
        ${CMAKE_BINARY_DIR}/${APP_NAME}.bin
        ${CMAKE_BINARY_DIR}/${APP_NAME}.map
        ${LFS_IMAGE}
        ${CMAKE_BINARY_DIR}/littlefs.bin
    COMMENT "Removed app and filesystem images"
)
