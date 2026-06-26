#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "types.h"

// ================================================================
// File system structures
// ================================================================

struct FileEntry {
    std::string name;
    int startBlock;
    int blockCount;
    int ownerPid;      ///< -1 = system (pre-existing)
};

struct FileOperation {
    int         pid;
    FileOpCode  code;
    std::string fileName;
    int         blockCount;   ///< Meaningful only for CREATE
};

// ================================================================
// FileSystem — contiguous allocation, first-fit
//
// Permissions:
//   Real-time processes → create (if space) and delete any file
//   User processes      → create (if space); delete only own files
// ================================================================

class FileSystem {
public:
    explicit FileSystem(int totalBlocks);

    /// Load a pre-existing file from the initial disk state.
    void addExistingFile(const std::string& name, int startBlock,
                         int blockCount);

    /// Execute one file operation.
    /// validPids    : set of existing process IDs
    /// realTimePids : subset of validPids that are real-time
    /// Returns a formatted result string for output.
    std::string execute(const FileOperation& op,
                        const std::vector<int>& validPids,
                        const std::vector<int>& realTimePids);

    /// Print the disk map to stdout.
    void printDiskMap() const;

private:
    std::vector<std::string> diskMap_;  ///< per-block label
    std::unordered_map<std::string, FileEntry> files_;

    int  firstFit(int count) const;
    void writeFile(const FileEntry& entry);
    void eraseFile(const std::string& name);
    bool processExists(int pid, const std::vector<int>& valid) const;
    bool isRealTime(int pid, const std::vector<int>& rtPids) const;
    static std::string formatBlockList(int start, int count);
};