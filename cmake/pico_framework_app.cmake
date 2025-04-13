function(pico_framework_app)
    cmake_parse_arguments(APP "" "NAME" "SOURCES" ${ARGN})

    if(NOT APP_NAME)
        message(FATAL_ERROR "You must provide a NAME to pico_framework_app()")
    endif()

    # Validate board
    if(NOT DEFINED PICO_BOARD)
        set(PICO_BOARD pico_w)
    endif()
    if(NOT PICO_BOARD STREQUAL "pico_w" AND NOT PICO_BOARD STREQUAL "pico_w_2")
        message(FATAL_ERROR "Unsupported board: ${PICO_BOARD}")
    endif()

    # Add framework
    add_subdirectory(${CMAKE_SOURCE_DIR}/framework)

    # Create executable
    add_executable(${APP_NAME} ${APP_SOURCES})
    target_link_libraries(${APP_NAME} pico_framework)

    # Compile definitions from options
    foreach(opt IN ITEMS
        PICO_HTTP_ENABLE_LITTLEFS
        PICO_HTTP_ENABLE_HTTP_CLIENT
        PICO_HTTP_CLIENT_ENABLE_TLS
        PICO_HTTP_ENABLE_JWT
        PICO_HTTP_TLS_VERIFY
    )
        if(${opt})
            target_compile_definitions(${APP_NAME} PRIVATE ${opt}=1)
        endif()
    endforeach()

    # Linker script for LittleFS
    if(PICO_HTTP_ENABLE_LITTLEFS)
        target_link_options(${APP_NAME} PRIVATE
            "-Wl,-T${CMAKE_SOURCE_DIR}/memmap_lfs_fragment.ld"
        )
    endif()

    pico_add_extra_outputs(${APP_NAME})

    # Memory diagnostics (optional)
    if(PICO_PRINT_MEM_USAGE)
        find_program(CMAKE_SIZE arm-none-eabi-size)
        find_program(CMAKE_OBJDUMP arm-none-eabi-objdump)
        if(CMAKE_SIZE AND CMAKE_OBJDUMP)
            add_custom_target(report_memory_${APP_NAME} ALL
                DEPENDS ${APP_NAME}
                COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${APP_NAME}>
                COMMAND ${CMAKE_OBJDUMP} -h $<TARGET_FILE:${APP_NAME}>
                COMMAND ${CMAKE_COMMAND}
                    -DPATH_TO_ELF=$<TARGET_FILE:${APP_NAME}>
                    -DCMAKE_OBJDUMP=${CMAKE_OBJDUMP}
                    -P ${CMAKE_SOURCE_DIR}/tools/report_memory.cmake
                COMMENT "📊 Running memory usage report for ${APP_NAME}..."
            )
        else()
            message(WARNING "Memory tools not found. Skipping diagnostics.")
        endif()
    endif()
endfunction()
