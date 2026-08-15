/**
 * @file    bench_bignum_init.c
 * @brief   Микробенчмарк для измерения затрат функции bignum_init().
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    2026-08-15
 *
 * @details
 *   Генерируются заранее случайные структуры `bignum_t` (включая «мусор» в
 *   словах и произвольное значение `len`).  Основной измеряемый цикл лишь
 *   копирует одну из предсозданных структур и вызывает `bignum_init()`.  Это
 *   позволяет отрезать накладные расходы на генерацию случайных чисел и
 *   сосредоточиться на реальном времени работы функции и её влиянии на кэш.
 *
 *   Сборка (пример):
 *       gcc -O2 -march=native -Iinclude -Wall -Wextra \
 *           benchmarks/bench_bignum_init.c src/bignum_init.c \
 *           -o bin/bench_bignum_init
 *
 *   Запуск под `perf` (пример):
 *       perf record -F 9999 -g -o benchmarks/reports/report_bignum_init \
 *           -- bin/bench_bignum_init
 *
 *   Затем:
 *       perf report -i benchmarks/reports/report_bignum_init \
 *           --symbol-filter=bignum_init
 *
 * @history
 *   - rev. 1 (2026-08-15): первая версия бенчмарка для bignum_init.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "bignum.h"
#include "bignum_init.h"

/* ------------------------------------------------------------------ */
/*  Константы, согласованные с реализацией (см. bignum_init.asm)        */
/* ------------------------------------------------------------------ */
#define BIGNUM_CAPACITY 32               /* количество 64‑битных слов */
#define ITERATIONS      (100000000u * 20)/* общее число измеряемых вызовов */
#define PREGEN_DATA_COUNT 8192           /* размер предгенерированного пула */

/* ------------------------------------------------------------------ */
/*  Заполняет структуру «мусором», чтобы гарантировать полное стирание */
/* ------------------------------------------------------------------ */
static void fill_random_bignum(bignum_t *x)
{
    /* Случайный len, иногда превышающий capacity – проверяем, что init
       не пишет за пределы и не падает. */
    x->len = (size_t)(rand() % (BIGNUM_CAPACITY + 5));

    for (size_t i = 0; i < BIGNUM_CAPACITY; ++i) {
        x->words[i] = ((uint64_t)rand() << 32) | (uint64_t)rand();
    }
}

/* ------------------------------------------------------------------ */
/*  Главная функция                                                   */
/* ------------------------------------------------------------------ */
int main(void)
{
    printf("Pre‑generating %u random bignum structures…\n", PREGEN_DATA_COUNT);

    bignum_t *pool = malloc(sizeof(bignum_t) * PREGEN_DATA_COUNT);
    if (!pool) {
        perror("malloc");
        return 1;
    }

    srand((unsigned)time(NULL));
    for (unsigned i = 0; i < PREGEN_DATA_COUNT; ++i) {
        fill_random_bignum(&pool[i]);
    }

    printf("Starting benchmark: %u iterations.\n", ITERATIONS);

    for (uint32_t i = 0; i < ITERATIONS; ++i) {
        /* Берём очередной элемент из пула (циклически) */
        const bignum_t *src = &pool[i % PREGEN_DATA_COUNT];

        /* Копируем, чтобы избежать изменения оригинального «мусора» */
        bignum_t tmp = *src;

        /* Тот единственный код, который измеряется */
        int rc = bignum_init(&tmp);
        (void)rc;                /* подавляем предупреждение о неиспользуемой переменной */
    }

    free(pool);
    printf("Benchmark finished.\n");
    return 0;
}
