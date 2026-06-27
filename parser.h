#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <memory>

#include "process.h"
#include "filesystem.h"


struct DiskConfig {
    int  totalBlocks = 0;
    std::vector<std::tuple<std::string, int, int>> existingFiles;
    std::vector<FileOperation> operations;
};


class InputParser {
public:
    /// Parse processes.txt; assigns PIDs 0..N-1.
    static std::vector<std::unique_ptr<Process>>
        parseProcesses(const std::string& path);

    /// Parse string.txt; sets pageRefString for each process.
    static void parsePageStrings(
        const std::string& path,
        std::vector<std::unique_ptr<Process>>& procs);

    /// Parse files.txt into a DiskConfig.
    static DiskConfig parseFilesystem(const std::string& path);

private:
    static std::vector<std::string> split(const std::string& s, char delim);
    static std::string              trim(const std::string& s);
};