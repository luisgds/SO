#pragma once
#include <string>
#include <vector>
#include <deque>

#include "types.h"

enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};



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
    int pageFaults;
    int pageRefIndex;       ///< Next index in pageRefString

    // --- I/O Resources (user processes only) -----------------
    bool needsPrinter;      ///< Requests a printer
    bool needsScanner;      ///< Requests the scanner
    bool needsModem;        ///< Requests the modem
    int  needsSATA;         ///< Number of SATA devices needed (0/1/2)

    // --- Scheduling ------------------------------------------
    ProcessState state;
    int queueLevel;         ///< Current user queue (1/2/3) or 0 for real-time
    int waitingTicks;       ///< Ticks spent waiting (for aging)
    int quantumUsed;        ///< Quantum consumed in current slice
    int instructionsDone;   ///< Instructions printed so far

    // ---------------------------------------------------------
    Process(int pid, int priority, int startTime, int cpuTime,
            int maxWorkingSet,
            bool needsPrinter, bool needsScanner,
            bool needsModem, int needsSATA);

    /// True if this is a real-time process (priority 0).
    bool isRealTime() const noexcept { return priority == PRIORITY_REALTIME; }

    /// True if the process has finished executing.
    bool isDone() const noexcept { return remainingTime <= 0; }
};