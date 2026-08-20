#ifndef BTB_HPP
#define BTB_HPP

#include <cstdint>

#define BTB_SETS 1024
#define BTB_WAYS 12

// ==============================================================================
class btb_entry_t {
   public:
    bool valid;
    uint64_t tag;
    uint64_t ciclo_ultimo_acesso;

    btb_entry_t();  // Construtor
};

// ==============================================================================
class btb_t {
   private:
    btb_entry_t entries[BTB_SETS][BTB_WAYS];  // 1024 * 12 = 12288

    // Estatísticas
    uint64_t stat_accesses;
    uint64_t stat_hits;
    uint64_t stat_misses;

    uint32_t get_set_index(uint64_t pc);
    uint64_t get_tag(uint64_t pc);

   public:
    btb_t();
    bool access(uint64_t pc, uint64_t current_cycle);
    void allocate();
    void statistics();
};

#endif  // BTB_HPP
