#pragma once

// ===========================================================
// Pseudo-SO - Common Types and Constants
// ===========================================================

// ----- Priority constants -----------------------------------
inline constexpr int PRIORITY_REALTIME = 0;
inline constexpr int PRIORITY_USER_1   = 1;   // Highest user priority
inline constexpr int PRIORITY_USER_2   = 2;
inline constexpr int PRIORITY_USER_3   = 3;   // Lowest user priority
inline constexpr int NUM_USER_QUEUES   = 3;   // Three user priority queues

// ----- Memory constants -------------------------------------
inline constexpr int TOTAL_FRAMES   = 20;
inline constexpr int RT_FRAMES      = 8;       // Frames reserved for real-time
inline constexpr int USER_FRAMES    = 12;      // Frames for user processes
inline constexpr int PRELOAD_PAGES  = 1;       // Pages pre-loaded at start

// ----- Scheduling -------------------------------------------
inline constexpr int QUANTUM_MS     = 1;       // User process quantum (1 ms)
inline constexpr int MAX_PROCESSES  = 1000;    // Global queue capacity
inline constexpr int AGING_THRESHOLD = 15;     // Ticks before queue promotion

// ----- I/O resources ----------------------------------------
inline constexpr int NUM_SCANNERS   = 1;
inline constexpr int NUM_PRINTERS   = 2;
inline constexpr int NUM_MODEMS     = 1;
inline constexpr int NUM_SATA       = 2;

// ============================================================
// Enumerations
// ============================================================

enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

enum class FileOpCode {
    CREATE      = 0,
    DELETE_FILE = 1
};