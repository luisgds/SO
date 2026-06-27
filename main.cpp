#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <memory>
#include <string>

#include "types.h"
#include "process.h"
#include "scheduler.h"
#include "memory.h"
#include "io.h"
#include "filesystem.h"
#include "parser.h"

// Completo : g++ -std=c++17 -Wall -Wextra -Wpedantic *.cpp -o dispatcher
// Simples: g++ -std=c++17 *.cpp -o dispatcher

static void printDispatcherHeader(const Process& p) {
    std::cout << "dispatcher =>\n"
              << " PID: "      << p.pid              << '\n'
              << " frames: "   << p.maxWorkingSet     << '\n'
              << " priority: " << p.priority          << '\n'
              << " time: "     << p.cpuTime           << '\n'
              << " printers: " << (p.needsPrinter ? 1 : 0) << '\n'
              << " scanners: " << (p.needsScanner ? 1 : 0) << '\n'
              << " modems: "   << (p.needsModem   ? 1 : 0) << '\n'
              << " drives: "   << (p.needsSATA    ? 1 : 0) << '\n';
}

static void executeTick(Process& proc, MemoryManager& mem) {
    if (proc.pageRefIndex < (int)proc.pageRefString.size()) {
        int page = proc.pageRefString[proc.pageRefIndex++];
        mem.accessPage(proc, page);
    }
    std::cout << "P" << proc.pid
              << " instruction " << ++proc.instructionsDone << '\n';
    --proc.remainingTime;
    ++proc.quantumUsed;
}

// Try to formally admit a newly-selected process.
// Returns the process if successfully admitted, nullptr if blocked.
static Process* admitProcess(
    Process* proc,
    MemoryManager& mem,
    IOManager& io,
    std::vector<Process*>& blockedIO,
    std::unordered_set<int>& admitted)
{
    // Allocate memory (only once)
    if (!proc->memAllocated) {
        if (!mem.allocateProcess(*proc)) {
            std::cerr << "P" << proc->pid << ": memória insuficiente\n";
            proc->state = ProcessState::TERMINATED;
            return nullptr;
        }
        proc->memAllocated = true;
    }

    // Allocate I/O (only once, user processes only)
    if (!proc->isRealTime() && !proc->ioAllocated) {
        if (!io.canAllocate(*proc)) {
            proc->state = ProcessState::BLOCKED;
            blockedIO.push_back(proc);
            return nullptr;
        }
        io.allocate(*proc);
        proc->ioAllocated = true;
    }

    // Print dispatcher header on first actual admission
    if (admitted.find(proc->pid) == admitted.end()) {
        printDispatcherHeader(*proc);
        std::cout << "process " << proc->pid << " =>\n";
        std::cout << "P" << proc->pid << " STARTED\n";
        admitted.insert(proc->pid);
    }

    proc->state = ProcessState::RUNNING;
    return proc;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Uso: " << argv[0]
                  << " processes.txt files.txt string.txt\n";
        return 1;
    }

    // ---------- 1. Leitura dos arquivos de entrada ------------------
    std::vector<std::unique_ptr<Process>> procs;
    DiskConfig diskCfg;

    try {
        procs   = InputParser::parseProcesses(argv[1]);
        InputParser::parsePageStrings(argv[3], procs);
        diskCfg = InputParser::parseFilesystem(argv[2]);
    } catch (const std::exception& ex) {
        std::cerr << "Erro de leitura: " << ex.what() << '\n';
        return 1;
    }

    if (procs.size() > static_cast<std::size_t>(MAX_PROCESSES)) {
        std::cerr << "Erro: número de processos excede o limite de "
                  << MAX_PROCESSES << ".\n";
        return 1;
    }

    // ---------- 2. Listas de PID para o sistema de arquivos ----------
    std::vector<int> validPids, rtPids;
    for (const auto& p : procs) {
        validPids.push_back(p->pid);
        if (p->isRealTime()) rtPids.push_back(p->pid);
    }

    // ---------- 3. Inicialização dos gerenciadores --------------------
    Scheduler     sched;
    MemoryManager mem;
    IOManager     io;
    FileSystem    fs(diskCfg.totalBlocks);

    for (const auto& [name, start, cnt] : diskCfg.existingFiles)
        fs.addExistingFile(name, start, cnt);

    for (auto& p : procs)
        sched.addToArrivalQueue(p.get());

    // ---------- 4. Laço principal da simulação --------------------------
    int time = 0;
    Process* running = nullptr;          // Processo de posse da CPU agora
    std::vector<Process*>   blockedIO;   // Esperando recurso de E/S
    std::unordered_set<int> admitted;    // PIDs já "despachados" uma vez

    // Se não há nenhum processo já pronto para rodar, pula direto para
    // o tick em que o primeiro processo efetivamente chega.
    // (sched.allEmpty() é false enquanto houver alguém no arrivalBuffer,
    // por isso usamos schedulingQueuesEmpty() aqui.)
    if (sched.schedulingQueuesEmpty() && sched.hasPendingArrivals())
        time = sched.nextArrivalTime();

    while (true) {
        // a) Processos cujo startTime já chegou entram nas filas.
        sched.dispatchArrivals(time);

        // b) Processos bloqueados em E/S: tenta liberar quem já pode.
        for (auto it = blockedIO.begin(); it != blockedIO.end(); ) {
            Process* p = *it;
            if (io.canAllocate(*p)) {
                io.allocate(*p);
                p->ioAllocated = true;
                sched.admit(p);
                it = blockedIO.erase(it);
            } else {
                ++it;
            }
        }

        // c) Tempo real tem prioridade absoluta: se um processo de
        //    usuário está rodando e um de tempo real chegou, o de
        //    usuário é preemptado e volta para sua fila.
        if (running && !running->isRealTime() && sched.realTimeSize() > 0) {
            sched.admit(running);
            running = nullptr;
        }

        // d) Se a CPU está livre, seleciona o próximo processo.
        if (!running) {
            Process* candidate = sched.selectNext();
            if (candidate)
                running = admitProcess(candidate, mem, io, blockedIO, admitted);
        }

        // e) Executa um tick do processo em CPU (se houver um).
        if (running) {
            executeTick(*running, mem);

            if (running->isDone()) {
                std::cout << "P" << running->pid << " return SIGINT\n";
                io.release(*running);
                mem.releaseProcess(*running);
                running->state = ProcessState::TERMINATED;
                running = nullptr;
            } else if (!running->isRealTime() &&
                       running->quantumUsed >= QUANTUM_MS) {
                // Quantum de 1ms esgotado: rebaixa de fila (seção 1.1).
                sched.demoteAfterQuantum(running);
                running = nullptr;
            }

            // f) Envelhecimento: evita starvation nas filas de usuário.
            sched.applyAging();
        }

        // g) Critério de parada: nada rodando, nada pronto, ninguém
        //    esperando E/S → a simulação de processos acabou.
        if (!running && sched.allEmpty() && blockedIO.empty()) break;

        // h) Avança o relógio. Se a CPU está ociosa mas há processos
        //    esperando para chegar, pula diretamente para o próximo
        //    instante de chegada (evita iterações vazias).
        if (!running && sched.schedulingQueuesEmpty()
            && blockedIO.empty() && sched.hasPendingArrivals()) {
            time = sched.nextArrivalTime();
        } else {
            ++time;
        }
    }

    // ---------- 5. Operações do sistema de arquivos -----------------------
    std::cout << "\nSistema de arquivos =>\n";
    int opNum = 1;
    for (const auto& op : diskCfg.operations) {
        std::cout << "Operação " << opNum++ << " => ";
        std::cout << fs.execute(op, validPids, rtPids) << '\n';
    }

    // ---------- 6. Mapa final de ocupação do disco -------------------------
    std::cout << '\n';
    fs.printDiskMap();

    // ---------- 7. Faltas de página por processo ----------------------------
    std::cout << "\nNúmero de Faltas de Páginas por processo:\n";
    for (const auto& p : procs) {
        std::cout << "P" << p->pid << " = " << p->pageFaults
                  << " faltas de páginas\n";
    }

    return 0;
}
