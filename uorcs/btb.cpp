#include "btb.hpp"

#include <cstdio>

#include "simulator.hpp"

// ==============================================================================
btb_entry_t::btb_entry_t() {  // zera as entradas quando iniciado
    this->valid = false;
    this->tag = 0;
    this->ciclo_ultimo_acesso = 0;
};

// ==============================================================================
btb_t::btb_t() {  // zera as estatísticas quando iniciado
    this->stat_accesses = 0;
    this->stat_hits = 0;
    this->stat_misses = 0;
};

// ==============================================================================
// PC = tag x BTB_SETS + set
uint32_t btb_t::get_set_index(uint64_t pc) {
    return pc % BTB_SETS;  // 10 bits baixos do pc = set
};

// ==============================================================================
uint64_t btb_t::get_tag(uint64_t pc) {
    return pc / BTB_SETS;  // quociente do pc = tag
};

// ==============================================================================
bool btb_t::access(uint64_t pc, uint64_t current_cycle) {
    this->stat_accesses++;

    uint32_t set_index = this->get_set_index(pc);
    uint64_t tag = this->get_tag(pc);

    // Verifica se é um hit
    for (uint32_t way = 0; way < BTB_WAYS; way++) {          // Vias
        btb_entry_t &entry = this->entries[set_index][way];  // Conjunto

        if (entry.valid && entry.tag == tag) {
            entry.ciclo_ultimo_acesso =
                current_cycle;  // atualiza o ciclo de acesso
            this->stat_hits++;
            return true;
        }
    }

    // se não é hit, é miss
    uint32_t victim_way = 0;
    uint64_t oldest_cycle = this->entries[set_index][0].ciclo_ultimo_acesso;
    bool found_invalid = false;

    for (uint32_t way = 0; way < BTB_WAYS; way++) {
        btb_entry_t &entry = this->entries[set_index][way];

        // Se não estiver preenchida, coloca aqui mesmo
        if (!entry.valid) {
            victim_way = way;
            found_invalid = true;
            break;
        }

        // Passa por todas as vias e escolhe a mais antiga
        if (entry.ciclo_ultimo_acesso < oldest_cycle) {
            oldest_cycle = entry.ciclo_ultimo_acesso;
            victim_way = way;
        }
    }

    (void)found_invalid;  // apenas para deixar claro que o caminho foi
                          // considerado.

    btb_entry_t &victim = this->entries[set_index][victim_way];
    victim.valid = true;
    victim.tag = tag;
    victim.ciclo_ultimo_acesso = current_cycle;

    this->stat_misses++;
    return false;
};
// ==============================================================================
void btb_t::allocate() {};

// ==============================================================================
// Estatísticas
void btb_t::statistics() {
    ORCS_PRINTF("######################################################\n");
    ORCS_PRINTF("btb_t\n");
    ORCS_PRINTF("btb_accesses:%lu\n", this->stat_accesses);
    ORCS_PRINTF("btb_hits:%lu\n", this->stat_hits);
    ORCS_PRINTF("btb_misses:%lu\n", this->stat_misses);

    double hit_rate = 0.0;
    if (this->stat_accesses > 0) {
        hit_rate =
            (double)this->stat_hits / (double)this->stat_accesses * 100.0;
    }
    ORCS_PRINTF("btb_hit_rate:%.2f%%\n", hit_rate);
};
