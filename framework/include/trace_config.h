#pragma once

// === Per-module enable flags ===
#define TRACE_NET     1
#define TRACE_HTTP    0
#define TRACE_AUTH    1
#define TRACE_APP     1

// === Global minimum log level (for all modules) ===
// Options: 0 = INFO, 1 = WARN, 2 = ERROR
#define TRACE_LEVEL_MIN TRACE_LVL_INFO

