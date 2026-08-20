#include <cstdio>
#include "btb.hpp"

// =============================================================================
// Pequeno helper para deixar os testes legíveis.
// =============================================================================
static int total_tests = 0;
static int passed_tests = 0;

static void check(bool condition, const char *description) {
    total_tests++;
    if (condition) {
        passed_tests++;
        printf("[PASS] %s\n", description);
    } else {
        printf("[FAIL] %s\n", description);
    }
}

// =============================================================================
int main() {
    btb_t btb;
    btb.allocate();

    // =========================================================================
    // Teste 1: primeiro acesso a um PC deve ser MISS (tabela vazia).
    // =========================================================================
    bool hit = btb.access(1024, /*current_cycle=*/0);
    check(hit == false, "Primeiro acesso a um PC novo deve ser MISS");

    // =========================================================================
    // Teste 2: acessar o MESMO PC de novo deve ser HIT.
    // =========================================================================
    hit = btb.access(1024, /*current_cycle=*/1);
    check(hit == true, "Segundo acesso ao mesmo PC deve ser HIT");

    // =========================================================================
    // Teste 3: encher um set inteiro (12 vias) e forçar um MISS por falta de espaço.
    // =========================================================================
    // Escolhendo PCs que caem no MESMO set: como set_index = pc % 1024,
    // somar 1024 mantém o mesmo set, mudando a tag.
    uint64_t base_pc = 5000;
    uint64_t pcs_no_mesmo_set[13];
    for (int i = 0; i < 13; i++) {
        pcs_no_mesmo_set[i] = base_pc + (uint64_t)i * BTB_SETS;
    }

    // Preenche as 12 vias do set (ciclos 100 a 111).
    for (int i = 0; i < 12; i++) {
        bool h = btb.access(pcs_no_mesmo_set[i], /*current_cycle=*/100 + i);
        check(h == false, "Preenchendo set: cada PC novo deve ser MISS");
    }

    // O 13o PC no mesmo set: set já está cheio, deve forçar EVICÇÃO (LRU).
    bool miss_13 = btb.access(pcs_no_mesmo_set[12], /*current_cycle=*/200);
    check(miss_13 == false, "13o PC no set cheio deve ser MISS (forca eviccao)");

// =========================================================================
    // Teste 4: verificar o segundo PC mais antigo ANTES do teste
    // que verifica o mais antigo, pra não causar eviccao em cadeia.
    // =========================================================================
    bool should_be_hit = btb.access(pcs_no_mesmo_set[1], /*current_cycle=*/300);
    check(should_be_hit == true,
          "Segundo PC mais antigo NAO deveria ter sido expulso -> HIT");

    // =========================================================================
    // Teste 5: o PC mais ANTIGO (pcs_no_mesmo_set[0]) deve ser
    // expulso pelo LRU. Reacessá-lo agora deve ser MISS.
    // =========================================================================
    bool should_be_miss = btb.access(pcs_no_mesmo_set[0], /*current_cycle=*/301);
    check(should_be_miss == false,
          "PC mais antigo (LRU) deveria ter sido expulso -> MISS ao reacessar");

    // =========================================================================
    // Teste 6: estresse com MAIS PCs distintos do que a capacidade total da
    // BTB (12288 entradas). Espalhamos os PCs por todos os 1024 sets, com
    // 13 PCs por set (13 * 1024 = 13312 PCs distintos, ultrapassando as 12
    // vias por set).
    // =========================================================================
    {
        btb_t stress_btb;
        stress_btb.allocate();

        const uint32_t pcs_per_set = 13;  // 1 a mais que as 12 vias.
        const uint32_t total_distinct_pcs = pcs_per_set * BTB_SETS;

        uint64_t hits = 0;
        uint64_t misses = 0;
        uint64_t cycle = 0;

        for (uint32_t set = 0; set < BTB_SETS; set++) {
            for (uint32_t i = 0; i < pcs_per_set; i++) {
                uint64_t pc = set + (uint64_t)i * BTB_SETS;
                bool hit = stress_btb.access(pc, cycle);
                if (hit) hits++; else misses++;
                cycle++;
            }
        }

        double first_pass_hit_rate = (double)hits / (double)(hits + misses);

        check(misses >= BTB_SETS,
              "Estresse (13 PCs/set): deve haver ao menos 1024 misses estruturais");

        check(first_pass_hit_rate < 0.10,
              "Estresse (13 PCs/set): hit rate da 1a passada deve ser baixa (~0%, tudo novo)");

        printf("[INFO] Estresse: %lu acessos, %lu hits, %lu misses, hit_rate=%.2f%%\n",
               (unsigned long)total_distinct_pcs, (unsigned long)hits,
               (unsigned long)misses, first_pass_hit_rate * 100.0);

        stress_btb.statistics();
    }

    // =========================================================================
    // Resultado final.
    // =========================================================================
    btb.statistics();
    printf("\n=== RESULTADO: %d/%d testes passaram ===\n", passed_tests, total_tests);

    return (passed_tests == total_tests) ? 0 : 1;
}