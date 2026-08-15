/**
 * @file    test_bignum_init_extra.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    2026-08-15
 *
 * @brief   Расширенные тесты для функции bignum_init().
 *
 * @details
 *   Набор покрывает дополнительные граничные и «контрактные» сценарии:
 *   •  len > BIGNUM_CAPACITY (нарушение контракта)
 *   •  NULL‑параметр
 *   •  Уже‑нормализованный ноль остаётся нулём
 *   •  Многократный вызов bignum_init() (идемпотентность)
 *   •  Проверка «непрошивки» памяти за пределами структуры (guard‑слова)
 *   •  Полная очистка при максимально возможных словах
 *
 * @history
 *   - rev. 0 (2026‑08‑15): первая версия расширенных тестов.
 */

#include "bignum_init.h"   /* объявление bignum_init() и статуса */
#include "bignum.h"        /* тип bignum_t и BIGNUM_CAPACITY */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
/*  1. NULL‑параметр                                                    */
/* ------------------------------------------------------------------ */
static void test_null_arg(void)
{
    printf("test_null_arg... ");
    assert(bignum_init(NULL) == BIGNUM_INIT_ERROR_NULL_ARG);
    printf("OK\n");
}

/* ------------------------------------------------------------------ */
/*  2. Нарушение контракта: len > BIGNUM_CAPACITY                       */
/* ------------------------------------------------------------------ */
static void test_len_overflow_contract(void)
{
    printf("test_len_overflow_contract... ");

#ifndef NDEBUG
    /* В debug‑сборке ожидается assert() (пользователь отвечает за контракт). */
    printf("SKIP (debug assert)\n");
#else
    /* В release‑сборке проверяем, что функция не пишет за пределы буфера. */
    bignum_t bad = {0};
    bad.len = BIGNUM_CAPACITY + 1;          /* нарушаем контракт */
    /* words[] оставляем неинициализированными – функция должна игнорировать их */
    int rc = bignum_init(&bad);
    /* Поведение не определено, но функция не должна аварийно падать. */
    assert(rc == BIGNUM_INIT_SUCCESS);
    assert(bignum_is_cleared(&bad));
    printf("OK\n");
#endif
}

/* ------------------------------------------------------------------ */
/*  3. Ноль остаётся нулём (len == 0)                                   */
/* ------------------------------------------------------------------ */
static void test_zero_stays_zero(void)
{
    printf("test_zero_stays_zero... ");

    bignum_t n = {.len = 0};
    /* Заполняем «мусором», чтобы убедиться, что bignum_init действительно стирает. */
    memset(n.words, 0xFF, sizeof(n.words));

    assert(bignum_init(&n) == BIGNUM_INIT_SUCCESS);
    assert(bignum_is_cleared(&n));
    printf("OK\n");
}

/* ------------------------------------------------------------------ */
/*  4. Повторный вызов (идемпотентность)                                 */
/* ------------------------------------------------------------------ */
static void test_repeat_init(void)
{
    printf("test_repeat_init... ");

    bignum_t n;
    /* Сначала заполняем произвольными данными. */
    for (size_t i = 0; i < BIGNUM_CAPACITY; ++i)
        n.words[i] = 0xAAAAAAAAAAAAAAAAULL;
    n.len = BIGNUM_CAPACITY;

    /* Первый вызов */
    assert(bignum_init(&n) == BIGNUM_INIT_SUCCESS);
    assert(bignum_is_cleared(&n));

    /* Второй вызов – должен ничего не сломать */
    assert(bignum_init(&n) == BIGNUM_INIT_SUCCESS);
    assert(bignum_is_cleared(&n));

    printf("OK\n");
}

/* ------------------------------------------------------------------ */
/*  5. Защита границ: guard‑слова до и после структуры                  */
/* ------------------------------------------------------------------ */
static void test_memory_guard_check(void)
{
    printf("test_memory_guard_check... ");

    uint64_t guard_val = 0xDEADBEEFDEADBEEFULL;
    size_t buf_sz = sizeof(bignum_t) + 2 * sizeof(uint64_t);
    char *buf = (char *)malloc(buf_sz);
    assert(buf != NULL);

    uint64_t *guard1 = (uint64_t *)buf;
    bignum_t *num   = (bignum_t *)(buf + sizeof(uint64_t));
    uint64_t *guard2 = (uint64_t *)((char *)num + sizeof(bignum_t));

    *guard1 = guard_val;
    *guard2 = guard_val;

    /* Заполняем структуру произвольными данными, чтобы увидеть, что только она стирается */
    for (size_t i = 0; i < BIGNUM_CAPACITY; ++i)
        num->words[i] = 0xFFFFFFFFFFFFFFFFULL;
    num->len = BIGNUM_CAPACITY;

    assert(bignum_init(num) == BIGNUM_INIT_SUCCESS);
    assert(bignum_is_cleared(num));

    assert(*guard1 == guard_val);
    assert(*guard2 == guard_val);

    free(buf);
    printf("OK\n");
}

/* ------------------------------------------------------------------ */
/*  6. Полная очистка при полном заполнении (len == BIGNUM_CAPACITY)   */
/* ------------------------------------------------------------------ */
static void test_full_capacity_clear(void)
{
    printf("test_full_capacity_clear... ");

    bignum_t n;
    n.len = BIGNUM_CAPACITY;
    for (size_t i = 0; i < BIGNUM_CAPACITY; ++i)
        n.words[i] = 0xFFFFFFFFFFFFFFFFULL;   /* полностью заполнено */

    assert(bignum_init(&n) == BIGNUM_INIT_SUCCESS);
    assert(bignum_is_cleared(&n));
    printf("OK\n");
}

/* ------------------------------------------------------------------ */
/*  Главная функция                                                       */
/* ------------------------------------------------------------------ */
int main(void)
{
    printf("--- Extra tests for bignum_init ---\n");

    test_null_arg();
    test_len_overflow_contract();
    test_zero_stays_zero();
    test_repeat_init();
    test_memory_guard_check();
    test_full_capacity_clear();

    printf("--- All extra bignum_init tests passed ---\n");
    return 0;
}
