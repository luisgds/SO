#include "scheduler.h"
#include <algorithm>

// ================================================================
// Scheduler — Implementation
// ================================================================

bool compareArrival(Process* a, Process* b) {
    return a->startTime < b->startTime;
}

void Scheduler::addToArrivalQueue(Process* proc) {
    arrivalBuffer_.push_back(proc);
    std::stable_sort(arrivalBuffer_.begin(), arrivalBuffer_.end(), compareArrival);
}

void Scheduler::dispatchArrivals(int time) {
    auto it = arrivalBuffer_.begin();
    while (it != arrivalBuffer_.end() && (*it)->startTime <= time) {
        Process* p = *it;
        p->state = ProcessState::READY;
        admit(p);
        it = arrivalBuffer_.erase(it);
    }
}

int Scheduler::nextArrivalTime() const {
    if (arrivalBuffer_.empty()) return -1;
    return arrivalBuffer_.front()->startTime;
}

bool Scheduler::hasPendingArrivals() const {
    return !arrivalBuffer_.empty();
}

Process* Scheduler::selectNext() {
    // Real-time queue has absolute priority over all user queues
    if (!realTimeQueue_.empty()) {
        Process* p = realTimeQueue_.front();
        realTimeQueue_.pop();
        return p;
    }
    // User queues from highest (index 0 = priority 1) to lowest
    for (auto& q : userQueues_) {
        if (!q.empty()) {
            Process* p = q.front();
            q.pop();
            return p;
        }
    }
    return nullptr;
}

void Scheduler::admit(Process* proc) {
    proc->state = ProcessState::READY;
    if (proc->isRealTime()) {
        realTimeQueue_.push(proc);
    } else {
        // queueLevel 1/2/3 → index 0/1/2
        int idx = proc->queueLevel - 1;
        if (idx < 0) idx = 0;
        if (idx >= NUM_USER_QUEUES) idx = NUM_USER_QUEUES - 1;
        userQueues_[idx].push(proc);
    }
}

void Scheduler::demoteAfterQuantum(Process* proc) {
    // Move to the next lower queue (max PRIORITY_USER_3)
    
    if (proc->queueLevel < PRIORITY_USER_3) {
        proc->queueLevel++;
    }
    proc->quantumUsed  = 0;
    proc->waitingTicks = 0;
    admit(proc);
}

void Scheduler::applyAging() {
    // Process user queues from lowest priority up
    // (promotion goes toward index 0 = priority 1)
    for (int lvl = PRIORITY_USER_3; lvl > PRIORITY_USER_1; --lvl) {
        int idx = lvl - 1;
        std::queue<Process*> stay, promote;

        while (!userQueues_[idx].empty()) {
            Process* p = userQueues_[idx].front();
            userQueues_[idx].pop();
            p->waitingTicks++;
            if (p->waitingTicks >= AGING_THRESHOLD) {
                p->queueLevel  = lvl - 1;  // promote one level
                p->waitingTicks = 0;
                promote.push(p);
            } else {
                stay.push(p);
            }
        }
        userQueues_[idx] = std::move(stay);
        while (!promote.empty()) {
            userQueues_[idx - 1].push(promote.front());
            promote.pop();
        }
    }

    // Tick waiting counters in priority-1 queue (can't promote further)
    std::queue<Process*> tmp;
    while (!userQueues_[0].empty()) {
        Process* p = userQueues_[0].front();
        userQueues_[0].pop();
        p->waitingTicks++;
        tmp.push(p);
    }
    userQueues_[0] = std::move(tmp);
}

bool Scheduler::allEmpty() const
{
    if (!arrivalBuffer_.empty())  return false;
    if (!realTimeQueue_.empty())  return false;
    for (const auto& q : userQueues_) {
        if (!q.empty()) return false;
    }
    return true;
}