#pragma once

#include "types.h"
#include "process.h"

// ================================================================
// IOManager — exclusive allocation of I/O devices
// ================================================================

class IOManager {
public:
    IOManager();

    /// True if all resources requested by proc are currently free.
    bool canAllocate(const Process& proc) const;

    /// Allocate resources; returns false if unavailable.
    bool allocate(const Process& proc);

    /// Release all resources held by proc.
    void release(const Process& proc);

    int availableScanners() const { return scanners_; }
    int availablePrinters() const { return printers_; }
    int availableModems()   const { return modems_;   }
    int availableSATA()     const { return sata_;     }

private:
    int scanners_;
    int printers_;
    int modems_;
    int sata_;
};