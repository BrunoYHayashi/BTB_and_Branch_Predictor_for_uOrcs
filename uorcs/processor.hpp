#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "branch_predictor.hpp"
#include "btb.hpp"
#include "opcode_package.hpp"

class processor_t {
   private:
    opcode_package_t previous_instruction;
    bool has_previous_instructions;

    // ciclos de atraso pelas branches
    uint64_t fetch_stall_cycles;

    btb_t btb;
    branch_predictor_t branch_predictor;

   public:
    processor_t();
    void branch_resolution(opcode_package_t &instruction, bool taken);
    void allocate();
    void clock();
    void statistics();
};

#endif  // PROCESSOR_HPP
