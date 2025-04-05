message("PATH_TO_ELF: ${PATH_TO_ELF}")
file(TO_CMAKE_PATH "${PATH_TO_ELF}" PATH_TO_ELF_CMAKE)
message("Resolved ELF path: ${PATH_TO_ELF_CMAKE}")

if(NOT EXISTS "${PATH_TO_ELF_CMAKE}")
    message(WARNING "🚨 ELF file does not exist, skipping memory report.")
    return()
endif()

# Run objdump
execute_process(
    COMMAND ${CMAKE_OBJDUMP} -h ${PATH_TO_ELF}
    OUTPUT_VARIABLE OBJDUMP_OUTPUT
    RESULT_VARIABLE OBJDUMP_RESULT
)

if (NOT OBJDUMP_RESULT EQUAL 0)
    message(WARNING "🚨 Failed to run objdump on ELF file.")
    return()
endif()

# Debug print raw output
message("========== objdump -h output ==========")
message("${OBJDUMP_OUTPUT}")
message("=======================================")

# Match and extract sizes
set(DATA_SIZE 0)
string(REGEX MATCH "[0-9]+[ ]+\\.data[ ]+([0-9a-fA-F]+)" _data_match "${OBJDUMP_OUTPUT}")
if (_data_match)
    set(DATA_HEX "${CMAKE_MATCH_1}")
    math(EXPR DATA_SIZE "0x${DATA_HEX}")
    message("Matched .data: ${DATA_HEX} (${DATA_SIZE} bytes)")
else()
    message("No match for .data")
endif()

set(BSS_SIZE 0)
string(REGEX MATCH "[0-9]+[ ]+\\.bss[ ]+([0-9a-fA-F]+)" _bss_match "${OBJDUMP_OUTPUT}")
if (_bss_match)
    set(BSS_HEX "${CMAKE_MATCH_1}")
    math(EXPR BSS_SIZE "0x${BSS_HEX}")
    message("Matched .bss : ${BSS_HEX} (${BSS_SIZE} bytes)")
else()
    message("No match for .bss")
endif()

math(EXPR TOTAL "${DATA_SIZE} + ${BSS_SIZE}")

message("")
message("📦 Framework RAM usage breakdown (static only)")
message("------------------------------------------------")
message("  .data: ${DATA_SIZE} bytes")
message("  .bss : ${BSS_SIZE} bytes")
message("------------------------------------------------")
message("  Framework RAM usage: ${TOTAL} bytes")
