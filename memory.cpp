#include <algorithm>
#include <stdexcept>

#include "memory.h"

MemoryManager::MemoryManager() : frameOwner_(TOTAL_FRAMES, -1) {}

bool MemoryManager::allocateProcess(Process& proc) {
    bool rt        = proc.isRealTime();
    int  poolStart = rt ? 0 : RT_FRAMES;
    int  poolSize  = rt ? RT_FRAMES  : USER_FRAMES;

    std::vector<int> frames = allocateFromPool(poolStart, poolSize,
                                               proc.maxWorkingSet, proc.pid);
    if (frames.empty()) return false;

    proc.allocatedFrames = std::move(frames);

    // Pre-load the first page (if any) — no fault counted
    if (!proc.pageRefString.empty()) {
        int  page  = proc.pageRefString[0];
        int  frame = proc.allocatedFrames[0];
        proc.pageInFrame[page] = frame;
        proc.lruQueue.push_back(page);   // back = most recently used
        // The pre-loaded page will be a hit when first accessed
    }
    return true;
}

void MemoryManager::releaseProcess(Process& proc) {
    for (int f : proc.allocatedFrames) releaseFrame(f);
    proc.allocatedFrames.clear();
    proc.lruQueue.clear();
    proc.pageInFrame.clear();
}

void MemoryManager::accessPage(Process& proc, int page) {
    auto it = proc.pageInFrame.find(page);

    if (it != proc.pageInFrame.end()) {
        // Hit — move page to back (MRU)
        proc.lruQueue.erase(
            std::remove(proc.lruQueue.begin(), proc.lruQueue.end(), page),
            proc.lruQueue.end());
        proc.lruQueue.push_back(page);
    } else {
        // Fault
        proc.pageFaults++;

        int frame = -1;
        if ((int)proc.lruQueue.size() < proc.maxWorkingSet) {
            // There is a free allocated frame
            frame = findFreeAllocatedFrame(proc);
        } else {
            // All frames occupied — evict LRU
            frame = evictLRU(proc);
        }

        proc.pageInFrame[page] = frame;
        proc.lruQueue.push_back(page);
    }
}

int MemoryManager::freeRTFrames() const {
    int n = 0;
    for (int i = 0; i < RT_FRAMES; ++i)
        if (frameOwner_[i] == -1) ++n;
    return n;
}

int MemoryManager::freeUserFrames() const {
    int n = 0;
    for (int i = RT_FRAMES; i < TOTAL_FRAMES; ++i)
        if (frameOwner_[i] == -1) ++n;
    return n;
}

// ================================================================
// Private helpers
// ================================================================

std::vector<int> MemoryManager::allocateFromPool(int poolStart, int poolSize,
                                                  int count, int pid) {
    std::vector<int> frames;
    for (int i = poolStart; i < poolStart + poolSize && (int)frames.size() < count; ++i) {
        if (frameOwner_[i] == -1) {
            frameOwner_[i] = pid;
            frames.push_back(i);
        }
    }
    return frames;
}

void MemoryManager::releaseFrame(int frame) {
    if (frame >= 0 && frame < TOTAL_FRAMES)
        frameOwner_[frame] = -1;
}

int MemoryManager::evictLRU(Process& proc) {
    // Front of lruQueue is the least recently used
    int lruPage = proc.lruQueue.front();
    proc.lruQueue.pop_front();

    int frame = proc.pageInFrame.at(lruPage);
    proc.pageInFrame.erase(lruPage);
    return frame;
}

int MemoryManager::findFreeAllocatedFrame(const Process& proc) const {
    for (int f : proc.allocatedFrames) {
        // Check if this frame is currently holding any page
        bool used = false;
        for (const auto& [pg, fr] : proc.pageInFrame) {
            if (fr == f) { used = true; break; }
        }
        if (!used) return f;
    }
    return -1;   // Should not happen if lruQueue.size() < maxWorkingSet
}