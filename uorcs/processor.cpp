#include "processor.hpp"  // allocate, clock, processor_t, statistics

// If you use clangd, it'll give you a warning on this include, but it's used by
// ORCS_PRINTF. Try removing it to see what happens.
// ~ Gabriel
#include <cstdio>

#include "branch_predictor.hpp"  // branch_predictor_t
#include "btb.hpp"               // btb_t
#include "opcode_package.hpp"    // opcode_package_t
#include "orcs_engine.hpp"       // orcs_engine_t
#include "simulator.hpp"         // ORCS_PRINTF
#include "trace_reader.hpp"

// =============================================================================
processor_t::processor_t() {  // Inicializa zerado
    this->has_previous_instructions = false;
    this->fetch_stall_cycles = 0;
};

// =============================================================================
void processor_t::allocate() {  // Alloca a btb e o branch_predictor
    this->btb.allocate();
    this->branch_predictor.allocate();
};

// =============================================================================
void processor_t::clock() {
    // Enquanto houver ciclos stall, ele vai rodando sem incrementar o pc
    if (this->fetch_stall_cycles > 0) {
        this->fetch_stall_cycles--;
        return;
    }

    // Próxima instrução
    // Get the next instruction from the trace.
    opcode_package_t new_instruction;
    if (!orcs_engine.trace_reader->trace_fetch(&new_instruction)) {
        // If EOF.
        orcs_engine.simulator_alive = false;
    }

    // Verifica se a última instrução era um branch
    if (this->has_previous_instructions) {
        bool taken = (this->previous_instruction.opcode_address +
                          this->previous_instruction.opcode_size !=
                      new_instruction.opcode_address);

        // Se realmente era um branch, então chama branch resolution
        if (this->previous_instruction.opcode_operation ==
            INSTRUCTION_OPERATION_BRANCH) {
            this->branch_resolution(this->previous_instruction, taken);
        }
    }

    // Atualiza a última instrução
    this->previous_instruction = new_instruction;
    this->has_previous_instructions = true;
};

// =============================================================================
void processor_t::branch_resolution(opcode_package_t &branch_instruction,
                                    bool taken) {
    bool btb_hit = this->btb.access(branch_instruction.opcode_address,
                                    orcs_engine.get_global_cycle());

    // Ciclos de stall para os casos do enunciado
    switch (branch_instruction.branch_type) {
        // btb miss quando não é condicional
        case BRANCH_UNCOND:
        case BRANCH_CALL:
        case BRANCH_RETURN:
        case BRANCH_SYSCALL:
            if (!btb_hit) {
                this->fetch_stall_cycles = 12;
            }
            break;

        // btb miss e hit com branch condicional e preditor
        case BRANCH_COND:
            bool predict_taken = this->branch_predictor.predict(
                branch_instruction.opcode_address);

            if (!btb_hit) {
                if (taken) {
                    this->fetch_stall_cycles = 512;
                }
            } else {
                if (predict_taken != taken) {
                    this->fetch_stall_cycles = 512;
                }
            }

            this->branch_predictor.update(branch_instruction.opcode_address,
                                          taken);
            break;
    }
}
// =============================================================================
// Estatísticas de BTB e Branch_predictor
void processor_t::statistics() {
    ORCS_PRINTF("######################################################\n");
    ORCS_PRINTF("processor_t\n");
    ORCS_PRINTF("total_simulation_cycles:%lu\n",
                orcs_engine.get_global_cycle());
    this->btb.statistics();
    this->branch_predictor.statistics();
};
