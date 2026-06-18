#pragma once
#include <string>
#include <vector>

enum class ProcessType {
    REAL_TIME = 0,
    USER      = 1
};

enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

struct Process {
    int pid;
    int priority;        // 0=real-time, 1,2,3=user priorities
    int startTime;       // Initialization time
    int cpuTime;         // Total CPU time needed
    int remainingTime;   // Remaining CPU time
    int maxWorkingSet;   // Max frames for this process
    
    // Resource requests
    bool requestsPrinter;
    bool requestsScanner;
    bool requestsModem;
    int  requestsSATA;   // 0, 1, or 2
    
    // Memory
    std::vector<int> pageReferenceString;
    std::vector<int> frames;            // Currently allocated frames
    std::vector<int> pageLoadOrder;     // For LRU tracking
    int pageFaults;
    // Aging (for user processes anti-starvation)
    int waitingTime;
    int agingCounter;
    
    ProcessType type;
    ProcessState state;
    
    // For multilevel feedback queue
    int currentQueueLevel; // 1, 2, or 3 for user processes
    int quantumUsed;       // How much quantum has been used
    
    Process() = default;
    Process(int pid, int priority, int startTime, int cpuTime,
            int maxWorkingSet, bool printer, bool scanner,
            bool modem, int sata);
            
    bool isRealTime() const { return priority == 0; }
};