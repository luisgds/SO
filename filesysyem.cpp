#include <sstream>
#include <iostream>
#include <algorithm>

#include "filesystem.h"

FileSystem::FileSystem(int totalBlocks)
    : diskMap_(totalBlocks, "")
{}

void FileSystem::addExistingFile(const std::string& name,
                                  int startBlock, int blockCount) {
    FileEntry e { name, startBlock, blockCount, -1 };
    writeFile(e);
    files_[name] = e;
}

std::string FileSystem::execute(const FileOperation& op,
                                 const std::vector<int>& validPids,
                                 const std::vector<int>& realTimePids)
{
    std::ostringstream out;

    if (!processExists(op.pid, validPids)) {
        out << "Falha\nO processo " << op.pid << " não existe.";
        return out.str();
    }

    bool rt = isRealTime(op.pid, realTimePids);

    if (op.code == FileOpCode::CREATE) {
        int start = firstFit(op.blockCount);
        if (start == -1) {
            out << "Falha\nO processo " << op.pid
                << " não pode criar o arquivo " << op.fileName
                << " (falta de espaço).";
            return out.str();
        }
        FileEntry e { op.fileName, start, op.blockCount, op.pid };
        writeFile(e);
        files_[op.fileName] = e;

        out << "Sucesso\nO processo " << op.pid
            << " criou o arquivo " << op.fileName
            << " (blocos " << formatBlockList(start, op.blockCount) << ").";

    } else {
        // DELETE
        auto it = files_.find(op.fileName);
        if (it == files_.end()) {
            out << "Falha\nO processo " << op.pid
                << " não pode deletar o arquivo " << op.fileName
                << " porque ele não existe.";
            return out.str();
        }
        const FileEntry& e = it->second;
        if (!rt && e.ownerPid != op.pid) {
            out << "Falha\nO processo " << op.pid
                << " não pode deletar o arquivo " << op.fileName
                << " (não é o proprietário).";
            return out.str();
        }
        eraseFile(op.fileName);
        files_.erase(it);
        out << "Sucesso\nO processo " << op.pid
            << " deletou o arquivo " << op.fileName << ".";
    }

    return out.str();
}

void FileSystem::printDiskMap() const
{
    std::cout << "Mapa de ocupação do disco:\n";
    for (std::size_t i = 0; i < diskMap_.size(); ++i) {
        if (i > 0) std::cout << ' ';
        std::cout << (diskMap_[i].empty() ? "0" : diskMap_[i]);
    }
    std::cout << '\n';
}

// ================================================================
// Private helpers
// ================================================================

int FileSystem::firstFit(int count) const
{
    int consecutive = 0, start = -1;
    for (int i = 0; i < (int)diskMap_.size(); ++i) {
        if (diskMap_[i].empty()) {
            if (consecutive == 0) start = i;
            if (++consecutive == count) return start;
        } else {
            consecutive = 0;
            start = -1;
        }
    }
    return -1;
}

void FileSystem::writeFile(const FileEntry& e)
{
    for (int i = 0; i < e.blockCount; ++i)
        diskMap_[e.startBlock + i] = e.name;
}

void FileSystem::eraseFile(const std::string& name) {
    for (auto& b : diskMap_)
        if (b == name) b = "";
}

bool FileSystem::processExists(int pid,
                                const std::vector<int>& valid) const {
    return std::find(valid.begin(), valid.end(), pid) != valid.end();
}

bool FileSystem::isRealTime(int pid,
                              const std::vector<int>& rtPids) const {
    return std::find(rtPids.begin(), rtPids.end(), pid) != rtPids.end();
}

std::string FileSystem::formatBlockList(int start, int count) {
    if (count == 1) return std::to_string(start);
    std::ostringstream oss;
    for (int i = 0; i < count; ++i) {
        if (i == count - 1) oss << " e ";
        else if (i > 0)     oss << ", ";
        oss << (start + i);
    }
    return oss.str();
}