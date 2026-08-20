#ifndef BRANCH_PREDICTOR_HPP
#define BRANCH_PREDICTOR_HPP

#include <cstdint>
#define PREDICTOR_TABLE_SIZE 1024  // Dito no artigo
#define GLOBAL_HISTORY_BITS 10     // lg PREDICTOR_TABLE_SIZE

// ==============================================================================
// Contador de 2 bits (0..3), usado nas 3 tabelas
// (bimodal, gshare e bpselector)
class counter_t {
   private:
    uint8_t value;  // 0 a 3

   public:
    counter_t();

    void increment();
    void decrement();
    bool predicts_taken();  // true se value >= 2 (bit mais significativo 1)
};

// ==============================================================================
class branch_predictor_t {
   private:
    counter_t bimodal_table[PREDICTOR_TABLE_SIZE];
    counter_t gshare_table[PREDICTOR_TABLE_SIZE];
    counter_t bp_selector[PREDICTOR_TABLE_SIZE];

    uint32_t global_history;  // Registrador de histórico global

    // Guarda a última predição de cada sub-preditor entre o predict() e o
    // update() seguinte, para sabermos quem acertou/errou na hora de atualizar
    // o seletor
    bool last_bimodal_prediction;
    bool last_gshare_prediction;
    bool last_final_prediction;  // A previsão que foi retornada por predict()

    // Gera o index dos preditores e do seletor
    uint32_t get_bimodal_index(uint64_t pc);
    uint32_t get_gshare_index(uint64_t pc);
    uint32_t get_selector_index(uint64_t pc);

    // Estatísticas.
    uint64_t stat_predictions;
    uint64_t stat_correct;
    uint64_t stat_incorrect;
    uint64_t stat_used_bimodal;
    uint64_t stat_used_gshare;

   public:
    branch_predictor_t();
    bool predict(uint64_t pc);
    void update(uint64_t pc, bool taken);
    void allocate();
    void statistics();
};

#endif  // BRANCH_PREDICTOR_HPP
