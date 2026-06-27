#pragma once
#include <string>
#include <vector>
#include <deque>
#include <unordered_map>

#include "types.h"


struct Process {
    // --- Identity --------------------------------------------
    int pid;
    int priority;           ///< 0=real-time, 1/2/3=user level
    int startTime;          ///< Arrival time (from processes.txt)
    int cpuTime;            ///< Total CPU burst required
    int remainingTime;      ///< CPU time still to execute

    // --- Memory ----------------------------------------------
    int maxWorkingSet;      ///< Max frames this process may use
    std::vector<int> pageRefString;     ///< Sequence of page accesses
    std::vector<int> allocatedFrames;   ///< Frames currently held
    std::deque<int>  lruQueue;          ///< LRU order (front=oldest)
    std::unordered_map<int, int> pageInFrame; ///< page# → frame#
    int pageFaults;
    int pageRefIndex;       ///< Next index in pageRefString

    // --- I/O Resources (user processes only) -----------------
    bool needsPrinter;      ///< Requests a printer
    bool needsScanner;      ///< Requests the scanner
    bool needsModem;        ///< Requests the modem
    bool needsSATA;         ///< Whether the process needs SATA devices (0/1/2)

    // --- Scheduling ------------------------------------------
    ProcessState state;
    int queueLevel;         ///< Current user queue (1/2/3) or 0 for real-time
    int waitingTicks;       ///< Ticks spent waiting (for aging)
    int quantumUsed;        ///< Quantum consumed in current slice
    int instructionsDone;   ///< Instructions printed so far

    // ------------- Flags de alocação (admissão) ---------------------
    bool memAllocated;  // Já reservou seus frames na memória?
    bool ioAllocated;   // Já reservou seus dispositivos de E/S?

    // ---------------------------------------------------------
    Process(int pid, int priority, int startTime, int cpuTime,
            int maxWorkingSet,
            bool needsPrinter, bool needsScanner,
            bool needsModem, bool needsSATA);

    /// True if this is a real-time process (priority 0).
    bool isRealTime() const noexcept { return priority == PRIORITY_REALTIME; }

    /// True if the process has finished executing.
    bool isDone() const noexcept { return remainingTime <= 0; }

    /// True if the process has been terminated.
    bool isTerminated() const noexcept { return state == ProcessState::TERMINATED; }
};

