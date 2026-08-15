/**
 * @file    test_bignum_init_mt.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    2026-08-15
 *
 * @brief   Многопоточный тест для bignum_init().
 *
 * @details
 *   Каждый поток получает собственный объект `bignum_t` со случайным
 *   содержимым (включая «мусор» в словах и произвольное значение `len`).
 *   Поток многократно вызывает `bignum_init()` и проверяет, что структура
 *   остаётся полностью обнулённой.  После завершения всех потоков основной
 *   поток проверяет, что каждое `bignum_t` действительно очищено.
 *
 *   Тест доказывает, что `bignum_init()` не использует глобальное/статическое
 *   состояние и корректно работает в условии реентрантного доступа.
 *
 * @history
 *   - rev. 1 (2026‑08‑15): первая версия MT‑теста.
 */

#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bignum_init.h"

#define NUM_THREADS      8
#define NUM_ITERATIONS  1000   /* количество повторных инициализаций в каждом потоке */

/* ------------------------------------------------------------------ */
/*  Вспомогательная проверка: полностью обнулённая структура            */
/* ------------------------------------------------------------------ */
static int bignum_is_cleared(const bignum_t *x)
{
    if (x->len != 0) return 0;
    for (size_t i = 0; i < BIGNUM_CAPACITY; ++i)
        if (x->words[i] != 0) return 0;
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Структура данных, передаваемая в поток                         */
/* ------------------------------------------------------------------ */
typedef struct {
    bignum_t num;          /* объект, который будет инициализироваться */
    int      thread_id;
} thread_data_t;

/* ------------------------------------------------------------------ */
/*  Тело потока                                                     */
/* ------------------------------------------------------------------ */
static void *worker_thread(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        /* каждый вызов должен вернуть BIGNUM_INIT_SUCCESS и полностью очистить */
        int rc = bignum_init(&data->num);
        assert(rc == BIGNUM_INIT_SUCCESS);
        assert(bignum_is_cleared(&data->num));
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Главная функция                                                 */
/* ------------------------------------------------------------------ */
int main(void)
{
    printf("--- Starting MT test for bignum_init ---\n");

    pthread_t       threads[NUM_THREADS];
    thread_data_t   thread_data[NUM_THREADS];
    unsigned int    seed = (unsigned int)time(NULL);

    for (int i = 0; i < NUM_THREADS; ++i) {
        thread_data[i].thread_id = i;

        /* Заполняем структуру «мусором», чтобы убедиться, что init действительно стирает */
        thread_data[i].num.len = (size_t)(rand_r(&seed) % (BIGNUM_CAPACITY + 5)); /* иногда превышаем capacity → проверка контракта */
        for (size_t w = 0; w < BIGNUM_CAPACITY; ++w)
            thread_data[i].num.words[w] = ((uint64_t)rand_r(&seed) << 32) | rand_r(&seed);

        /* Запускаем поток */
        int rc = pthread_create(&threads[i], NULL, worker_thread, &thread_data[i]);
        assert(rc == 0);
    }

    /* Ожидаем завершения всех потоков */
    for (int i = 0; i < NUM_THREADS; ++i) {
        int rc = pthread_join(threads[i], NULL);
        assert(rc == 0);
    }

    /* Финальная проверка – каждый объект всё ещё должен быть чистым */
    for (int i = 0; i < NUM_THREADS; ++i) {
        assert(bignum_is_cleared(&thread_data[i].num));
    }

    printf("--- MT test for bignum_init passed ---\n");
    return 0;
}
