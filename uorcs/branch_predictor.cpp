#include "branch_predictor.hpp"

#include <cstdio>

#include "simulator.hpp"

// ==============================================================================
counter_t::counter_t() {
    // O artigo assume que todos os branches anteriores foram "taken" antes
    // então iniciamos no valor mais alto (3)
    this->value = 3;
};

// ==============================================================================
void counter_t::increment() {
    if (this->value < 3) {
        this->value++;
    }
};

// ==============================================================================
void counter_t::decrement() {
    if (this->value > 0) {
        this->value--;
    }
};

// ==============================================================================
bool counter_t::predicts_taken() { return this->value >= 2; };

// ==============================================================================
branch_predictor_t::branch_predictor_t() {
    this->global_history = 0;

    this->last_bimodal_prediction = false;
    this->last_gshare_prediction = false;
    this->last_final_prediction = false;

    this->stat_predictions = 0;
    this->stat_correct = 0;
    this->stat_incorrect = 0;
    this->stat_used_bimodal = 0;
    this->stat_used_gshare = 0;
};

// ==============================================================================
uint32_t branch_predictor_t::get_bimodal_index(uint64_t pc) {
    return pc % PREDICTOR_TABLE_SIZE;  // Limita o index de 0 a 1023
};

// ==============================================================================
uint32_t branch_predictor_t::get_gshare_index(uint64_t pc) {
    uint32_t gh_bits = this->global_history % PREDICTOR_TABLE_SIZE;
    uint32_t pc_bits = pc & PREDICTOR_TABLE_SIZE;

    return pc_bits ^ gh_bits;  // XOR
};

// ==============================================================================
uint32_t branch_predictor_t::get_selector_index(uint64_t pc) {
    return pc % PREDICTOR_TABLE_SIZE;
};

// ==============================================================================
bool branch_predictor_t::predict(uint64_t pc) {
    this->stat_predictions++;  // Incrementa mais uma predição nas estatísticas

    uint32_t bimodal_index = this->get_bimodal_index(pc);
    uint32_t gshare_index = this->get_gshare_index(pc);
    uint32_t selector_index = this->get_selector_index(pc);

    // Verifica a predição do bimodal e do gshare
    this->last_bimodal_prediction =
        this->bimodal_table[bimodal_index].predicts_taken();
    this->last_gshare_prediction =
        this->gshare_table[gshare_index].predicts_taken();

    // Verifica se usa gshare ou bimodal
    bool use_gshare = this->bp_selector[selector_index].predicts_taken();

    // Incrementa estatísticas dos preditores usados e guarda o resultado da
    // predição
    if (use_gshare) {
        this->stat_used_gshare++;
        this->last_final_prediction = this->last_gshare_prediction;
    } else {
        this->stat_used_bimodal++;
        this->last_final_prediction = this->last_bimodal_prediction;
    }

    return this->last_final_prediction;
};

// ==============================================================================
void branch_predictor_t::update(uint64_t pc, bool taken) {
    uint32_t bimodal_index = this->get_bimodal_index(pc);
    uint32_t gshare_index = this->get_gshare_index(pc);
    uint32_t selector_index = this->get_selector_index(pc);

    // Atualiza o bpselector.
    bool bimodal_correct = (this->last_bimodal_prediction == taken);
    bool gshare_correct = (this->last_gshare_prediction == taken);

    if (gshare_correct && !bimodal_correct) {
        this->bp_selector[selector_index].increment();  // reforca gshare.
    } else if (!gshare_correct && bimodal_correct) {
        this->bp_selector[selector_index].decrement();  // reforca bimodal.
    }
    // Se os dois acertaram ou os dois erraram, nao muda nada.

    // Atualiza os contadores dos preditores individuais.
    if (taken) {
        this->bimodal_table[bimodal_index].increment();
        this->gshare_table[gshare_index].increment();
    } else {
        this->bimodal_table[bimodal_index].decrement();
        this->gshare_table[gshare_index].decrement();
    }

    // Atualiza o histórico global.
    this->global_history = (this->global_history << 1) | (taken ? 1 : 0);

    // Estatísticas
    if (this->last_final_prediction == taken) {
        this->stat_correct++;
    } else {
        this->stat_incorrect++;
    }
};

// =============================================================================
void branch_predictor_t::allocate() {};

// =============================================================================
void branch_predictor_t::statistics() {
    ORCS_PRINTF("######################################################\n");
    ORCS_PRINTF("branch_predictor_t\n");
    ORCS_PRINTF("predictor_predictions:%lu\n", this->stat_predictions);
    ORCS_PRINTF("predictor_correct:%lu\n", this->stat_correct);
    ORCS_PRINTF("predictor_incorrect:%lu\n", this->stat_incorrect);

    double accuracy = 0.0;
    if (this->stat_predictions > 0) {
        accuracy =
            (double)this->stat_correct / (double)this->stat_predictions * 100.0;
    }
    ORCS_PRINTF("predictor_accuracy:%.2f%%\n", accuracy);

    ORCS_PRINTF("predictor_used_bimodal:%lu\n", this->stat_used_bimodal);
    ORCS_PRINTF("predictor_used_gshare:%lu\n", this->stat_used_gshare);
};
