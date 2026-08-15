/**
 * @file    bench_bignum_init_mt.c
 * @brief   Многопоточный микробенчмарк для профилирования bignum_init().
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    2026-08-15
 *
 * @details
 *   Предгенерируется пул `bignum_t` со случайным «мусором» (в том числе
 *   иногда `len` > BIGNUM_CAPACITY, чтобы проверить отсутствие выхода за
 *   границы).  Каждый из N потоков многократно копирует один элемент из
 *   пула и вызывает `bignum_init()` над копией.  Всё, что измеряется,
 *   – это стоимость самой функции и связанные с ней операции копирования.
 *
 *   Сборка (пример):
 *       gcc -O2 -march=native -pthread -Iinclude \
 *           benchmarks/bench_bignum_init_mt.c src/bignum_init.c \
 *           -o bin/bench_bignum_init_mt
 *
 *   Запуск под perf (пример):
 *       perf record -F 9999 -g -o benchmarks/reports/report_bignum_init_mt \
 *           -- bin/bench_bignum_init_mt
 *
 *   Затем:
 *       perf report -i benchmarks/reports/report_bignum_init_mt \
 *           --symbol-filter=bignum_init
 *
 * @history
 *   - rev. 1 (2026-08-15): первая версия многопоточного бенчмарка.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "bignum.h"
#include "bignum_init.h"

/* ------------------------------------------------------------------ */
/*  Константы, согласованные с реализацией (см. bignum_init.asm)        */
/* ------------------------------------------------------------------ */
#define BIGNUM_CAPACITY 32               /* количество 64‑битных слов */
#ifndef THREAD_COUNT
#   define THREAD_COUNT 8               /* количество одновременно работающих потоков */
#endif
#ifndef ITER_PER_THREAD
#   define ITER_PER_THREAD (20000000u * 20) /* количество вызовов на поток */
#endif
#define PREGEN_DATA_COUNT 8192           /* размер пула предгенерированных чисел */

/* ------------------------------------------------------------------ */
/*  Заполняет структуру «мусором», иногда превышая capacity            */
/* ------------------------------------------------------------------ */
static void fill_random_bignum(bignum_t *x)
{
    /* len может быть в диапазоне [0, BIGNUM_CAPACITY+4] */
    x->len = (size_t)(rand() % (BIGNUM_CAPACITY + 5));

    for (size_t i = 0; i < BIGNUM_CAPACITY; ++i) {
        x->words[i] = ((uint64_t)rand() << 32) | (uint64_t)rand();
    }
}

/* ------------------------------------------------------------------ */
/*  Аргументы, передаваемые каждому потоку                               */
/* ------------------------------------------------------------------ */
typedef struct {
    unsigned   thread_id;
    unsigned   iters;
    const bignum_t *pool;   /* общий пул «мусорных» чисел */
    unsigned   pool_sz;
} thread_arg_t;

/* ------------------------------------------------------------------ */
/*  Тело потока – копирует элемент из пула и инициализирует его          */
/* ------------------------------------------------------------------ */
static void *thread_func(void *arg)
{
    const thread_arg_t *t = (const thread_arg_t *)arg;

    for (unsigned i = 0; i < t->iters; ++i) {
        /* Выбираем элемент из пула (циклически, с небольшим сдвигом) */
        unsigned idx = (i + t->thread_id) % t->pool_sz;

        /* Копируем, чтобы не повредить исходный «мусор» */
        bignum_t tmp = t->pool[idx];

        /* Единственная измеряемая операция */
        int rc = bignum_init(&tmp);
        (void)rc;   /* подавляем предупреждение о неиспользуемой переменной */
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Главная функция                                                   */
/* ------------------------------------------------------------------ */
int main(void)
{
    printf("Pre‑generating %u bignum structures for %u threads…\n",
           PREGEN_DATA_COUNT, THREAD_COUNT);

    bignum_t *pool = malloc(sizeof(bignum_t) * PREGEN_DATA_COUNT);
    if (!pool) {
        perror("malloc");
        return 1;
    }

    srand((unsigned)time(NULL));
    for (unsigned i = 0; i < PREGEN_DATA_COUNT; ++i) {
        fill_random_bignum(&pool[i]);
    }

    printf("Starting benchmark: %u threads, %u iterations each.\n",
           THREAD_COUNT, ITER_PER_THREAD);

    pthread_t       threads[THREAD_COUNT];
    thread_arg_t    args[THREAD_COUNT];

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        args[i].thread_id = i;
        args[i].iters     = ITER_PER_THREAD;
        args[i].pool      = pool;
        args[i].pool_sz   = PREGEN_DATA_COUNT;

        int rc = pthread_create(&threads[i], NULL, thread_func, &args[i]);
        if (rc != 0) {
            perror("pthread_create");
            free(pool);
            return 1;
        }
    }

    for (unsigned i = 0; i < THREAD_COUNT; ++i) {
        void *res;
        pthread_join(threads[i], &res);
        if (res != NULL) {
            fprintf(stderr, "Thread %u returned error.\n", i);
        }
    }

    free(pool);
    printf("Benchmark finished.\n");
    return 0;
}
