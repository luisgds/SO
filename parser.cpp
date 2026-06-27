#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

#include "parser.h"

// ================================================================
// parseProcesses
// Format (per line):
//   startTime, priority, cpuTime, maxWS, printer, scanner, modem, sata
// ================================================================

std::vector<std::unique_ptr<Process>>
InputParser::parseProcesses(const std::string& path)
{
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open " + path);

    std::vector<std::unique_ptr<Process>> procs;
    std::string line;
    int pid = 0;

    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty()) continue;

        auto p = split(line, ',');
        if (p.size() < 8)
            throw std::runtime_error("Bad process line: " + line);

        int  startTime = std::stoi(trim(p[0]));
        int  priority  = std::stoi(trim(p[1]));
        int  cpuTime   = std::stoi(trim(p[2]));
        int  maxWS     = std::stoi(trim(p[3]));
        bool printer   = (std::stoi(trim(p[4])) != 0);
        bool scanner   = (std::stoi(trim(p[5])) != 0);
        bool modem     = (std::stoi(trim(p[6])) != 0);
        bool sata      = (std::stoi(trim(p[7])) != 0);

        auto proc = std::make_unique<Process>(
            pid++,
            priority,
            startTime,
            cpuTime,
            maxWS,
            printer,
            scanner,
            modem,
            sata
        );

        procs.push_back(std::move(proc));
    }
    return procs;
}

// ================================================================
// parsePageStrings
// ================================================================

void InputParser::parsePageStrings(
    const std::string& path,
    std::vector<std::unique_ptr<Process>>& procs)
{
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open " + path);

    std::string line;
    std::size_t idx = 0;

    while (std::getline(f, line) && idx < procs.size()) {
        line = trim(line);
        if (line.empty()) continue;

        for (const auto& tok : split(line, ','))
            procs[idx]->pageRefString.push_back(std::stoi(trim(tok)));
        ++idx;
    }
}

// ================================================================
// parseFilesystem
// Line 1  : totalBlocks
// Line 2  : N (existing files)
// Lines 3..N+2 : name, firstBlock, blockCount
// Lines N+3..  : pid, opCode, fileName [, blockCount]
// ================================================================

DiskConfig InputParser::parseFilesystem(const std::string& path)
{
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open " + path);

    DiskConfig cfg;
    std::string line;

    std::getline(f, line); cfg.totalBlocks = std::stoi(trim(line));

    int n = 0;
    std::getline(f, line); n = std::stoi(trim(line));

    for (int i = 0; i < n; ++i) {
        std::getline(f, line);
        auto p = split(line, ',');
        if (p.size() < 3) continue;
        cfg.existingFiles.emplace_back(trim(p[0]),
                                       std::stoi(trim(p[1])),
                                       std::stoi(trim(p[2])));
    }

    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty()) continue;
        auto p = split(line, ',');
        if (p.size() < 3) continue;

        FileOperation op;
        op.pid        = std::stoi(trim(p[0]));
        op.code       = static_cast<FileOpCode>(std::stoi(trim(p[1])));
        op.fileName   = trim(p[2]);
        op.blockCount = (p.size() >= 4) ? std::stoi(trim(p[3])) : 0;
        cfg.operations.push_back(op);
    }
    return cfg;
}

// ================================================================
// Private helpers
// ================================================================

std::vector<std::string> InputParser::split(const std::string& s, char d) {
    std::vector<std::string> v;
    std::stringstream ss(s);
    std::string tok;
    while (std::getline(ss, tok, d)) v.push_back(tok);
    return v;
}

std::string InputParser::trim(const std::string& s) {
    auto b = s.find_first_not_of(" \t\r\n");
    auto e = s.find_last_not_of(" \t\r\n");
    return (b == std::string::npos) ? "" : s.substr(b, e - b + 1);
}