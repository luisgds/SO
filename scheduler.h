#pragma once
#include <vector>
#include <queue>
#include <array>
#include "types.h"
#include "process.h"

// ================================================================
// Scheduler — manages all scheduling queues
//
// Structure:
//   arrivalBuffer_: processes sorted by startTime
//   realTimeQueue_: FIFO (priority 0), no preemption among RT
//   userQueues_[0]: user priority 1 (highest)
//   userQueues_[1]: user priority 2
//   userQueues_[2]: user priority 3 (lowest)
//
// Policy:
//   - Real-time processes always run before user processes
//   - User processes use multilevel feedback with aging
//   - Quantum = QUANTUM_MS ticks; on expiry the process is demoted
//   - Aging: if a user process waits AGING_THRESHOLD ticks it is promoted
// ================================================================

class Scheduler {
public:
    Scheduler() = default;

    // --- Arrival management -------------------------------------

    /// Add process to the arrival buffer (sorted by startTime).
    void addToArrivalQueue(Process* proc);

    /// Move processes with startTime <= time into scheduling queues.
    void dispatchArrivals(int time);

    /// Time of the next pending arrival; -1 if no pending arrivals.
    int nextArrivalTime() const;

    /// True if there are processes in the arrival buffer.
    bool hasPendingArrivals() const;

    // --- Queue operations ---------------------------------------

    /// Pick and remove the next process to run.
    /// Returns nullptr if all queues are empty.
    Process* selectNext();

    /// Put a process (back) into the queue matching its queueLevel.
    /// Used for initial admission and re-admission after preemption.
    void admit(Process* proc);

    /// Called when a user process exhausts its quantum.
    /// Demotes the process one level (clamped at PRIORITY_USER_3)
    /// and re-enqueues it.
    void demoteAfterQuantum(Process* proc);

    // --- Aging --------------------------------------------------

    /// Increment waiting ticks and promote processes that have
    /// waited >= AGING_THRESHOLD.  Call once per simulation tick.
    void applyAging();

    // --- Status -------------------------------------------------

    /// True if arrival buffer AND all scheduling queues are empty.
    bool allEmpty() const;

    std::size_t realTimeSize() const { return realTimeQueue_.size(); }
    std::size_t userQueueSize(int level) const {
        return userQueues_[level - 1].size();
    }

private:
    std::vector<Process*>                          arrivalBuffer_;
    std::queue<Process*>                           realTimeQueue_;
    std::array<std::queue<Process*>, NUM_USER_QUEUES> userQueues_;
};