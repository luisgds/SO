#include "io.h"

IOManager::IOManager()
    : scanners_(NUM_SCANNERS),
      printers_(NUM_PRINTERS),
      modems_(NUM_MODEMS),
      sata_(NUM_SATA) 
    {}

bool IOManager::canAllocate(const Process& proc) const {
    if (proc.isRealTime()) return true;

    if (proc.needsScanner && scanners_ < 1) return false;
    if (proc.needsPrinter && printers_ < 1) return false;
    if (proc.needsModem   && modems_   < 1) return false;
    if (proc.needsSATA    && sata_     < 1) return false;

    return true;
}

bool IOManager::allocate(const Process& proc) {
    if (proc.isRealTime()) return true;
    if (!canAllocate(proc)) return false;

    if (proc.needsScanner) --scanners_;
    if (proc.needsPrinter) --printers_;
    if (proc.needsModem)   --modems_;
    if (proc.needsSATA)    --sata_;

    return true;
}

void IOManager::release(const Process& proc)
{
    if (proc.isRealTime()) return;

    if (proc.needsScanner) ++scanners_;
    if (proc.needsPrinter) ++printers_;
    if (proc.needsModem)   ++modems_;
    if (proc.needsSATA)    ++sata_;
}