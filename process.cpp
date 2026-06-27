#include "process.h"

Process::Process(int pid, int priority, int startTime, int cpuTime,
                 int maxWorkingSet,
                 bool needsPrinter, bool needsScanner,
                 bool needsModem, bool needsSATA)
    : pid(pid),
      priority(priority),
      startTime(startTime),
      cpuTime(cpuTime),
      remainingTime(cpuTime),
      maxWorkingSet(maxWorkingSet),
      pageFaults(0),
      pageRefIndex(0),
      needsPrinter(needsPrinter),
      needsScanner(needsScanner),
      needsModem(needsModem),
      needsSATA(needsSATA),
      state(ProcessState::NEW),
      waitingTicks(0),
      quantumUsed(0),
      instructionsDone(0),
      memAllocated(false),
      ioAllocated(false)
{
    queueLevel = priority;
}
