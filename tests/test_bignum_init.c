/**
 * @file    test_bignum_init.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    2026-08-15
 *
 * @brief   Детерминированные тесты для функции bignum_init().
 *
 * @details
 *   Набор покрывает все граничные и типичные ситуации:
 *   1.  Проверка NULL‑аргумента.
 *   2.  Инициализация уже‑незанятой структуры (len > 0, произвольные слова).
 *   3.  Инициализация структуры с len == 0.
 *   4.  Проверка, что после инициализации массив `words` полностью обнулён
 *       независимо от исходных данных.
 *
 * @history
 *   - rev. 0 (2026-08-15): Первая версия тестов.
 */

#include "bignum_init.h"      /* объявление bignum_init() и типы */
#include "bignum.h"           /* определение bignum_t и BIGNUM_CAPACITY */
#include <stdio.h>
#include <string.h>
#include <assert.h>

/* ------------------------------------------------------------------ */
/*  Вспомогательная проверка: все слова == 0 и len == 0               */
/* ------------------------------------------------------------------ */
static int bignum_is_cleared(const bignum_t *x)
{
    if (x->len != 0) return 0;
    /* words[] может содержать «мусор», поэтому проверяем полностью */
    for (size_t i = 0; i < BIGNUM_CAPACITY; ++i)
        if (x->words[i] != 0) return 0;
    return 1;
}

/* ------------------------------------------------------------------ */
/*  Тест 1 – NULL‑аргумент                                               */
/* ------------------------------------------------------------------ */
static void test_null_arg(void)
{
    printf("Running %s … ", __func__);
    assert(bignum_init(NULL) == BIGNUM_INIT_ERROR_NULL_ARG);
    printf("PASSED\n");
}

/* ------------------------------------------------------------------ */
/*  Тест 2 – уже‑незанятая структура (len > 0, произвольные слова)      */
/* ------------------------------------------------------------------ */
static void test_nonzero_input(void)
{
    printf("Running %s … ", __func__);

    bignum_t n = {
        .words = { 0x1122334455667788ULL, 0xFFEEDDCCBBAA9988ULL,
                   0x1234567890ABCDEFULL, 0x0ULL },
        .len   = 3
    };
    bignum_t expected = { .len = 0 };
    memset(expected.words, 0, sizeof(expected.words));

    assert(bignum_init(&n) == BIGNUM_INIT_SUCCESS);
    assert(bignum_is_cleared(&n));
    printf("PASSED\n");
}

/* ------------------------------------------------------------------ */
/*  Тест 3 – len == 0 (пустое число)                                    */
/* ------------------------------------------------------------------ */
static void test_empty_len_zero(void)
{
    printf("Running %s … ", __func__);

    bignum_t n = { .len = 0 };
    memset(n.words, 0xFF, sizeof(n.words));   /* заполним «мусором» */

    assert(bignum_init(&n) == BIGNUM_INIT_SUCCESS);
    assert(bignum_is_cleared(&n));
    printf("PASSED\n");
}

/* ------------------------------------------------------------------ */
/*  Тест 4 – проверка, что len и words действительно обнуляются      */
/* ------------------------------------------------------------------ */
static void test_full_clear(void)
{
    printf("Running %s … ", __func__);

    bignum_t n;
    /* Заполняем полностью произвольными данными */
    for (size_t i = 0; i < BIGNUM_CAPACITY; ++i)
        n.words[i] = 0xFFFFFFFFFFFFFFFFULL;
    n.len = BIGNUM_CAPACITY;

    assert(bignum_init(&n) == BIGNUM_INIT_SUCCESS);
    assert(bignum_is_cleared(&n));
    printf("PASSED\n");
}

/* ------------------------------------------------------------------ */
/*  Главная функция                                                       */
/* ------------------------------------------------------------------ */
int main(void)
{
    printf("--- Starting bignum_init tests ---\n");

    test_null_arg();
    test_nonzero_input();
    test_empty_len_zero();
    test_full_clear();

    printf("--- All bignum_init tests passed ---\n");
    return 0;
}
