#pragma once

#include <vector>

#include "types.h"
#include "process.h"

// MemoryManager — virtual memory with LRU page replacement

class MemoryManager {
public:
    MemoryManager();

    /// Allocate up to proc.maxWorkingSet frames from the correct pool
    /// and pre-load the first page.  Returns false if the pool is full.
    bool allocateProcess(Process& proc);

    /// Release all frames owned by proc.
    void releaseProcess(Process& proc);

    /// Handle one page reference: update LRU, count fault if miss.
    void accessPage(Process& proc, int page);

    int freeRTFrames()   const;
    int freeUserFrames() const;

private:
    // frameOwner_[i] = -1 (free) or pid of owner process
    std::vector<int> frameOwner_;

    // --- helpers ------------------------------------------------
    std::vector<int> allocateFromPool(int poolStart, int poolSize,
                                      int count, int pid);
    void releaseFrame(int frame);
    int  evictLRU(Process& proc);      ///< Evict LRU page, return freed frame
    int  findFreeAllocatedFrame(const Process& proc) const;
};