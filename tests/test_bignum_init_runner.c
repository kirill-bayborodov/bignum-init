/**
 * @file    test_bignum_init_runner.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    2026-08-15
 *
 * @brief   Интеграционный тест‑раннер для библиотеки libbignum_init.a.
 * @details Проверяет, что заголовочный файл `bignum_init.h` объявляет
 *          функцию `bignum_init` с правильной сигнатурой и что
 *          статическая библиотека `libbignum_init.a` линкуется без ошибок.
 *
 * @history
 *   - rev. 1 (2026‑08‑15): первая версия тест‑раннера.
 */

#include "bignum_init.h"   /* объявление bignum_init() и типы статуса */
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    printf("Running test: test_bignum_init_runner... ");

    /* Создаём объект с «мусором», чтобы убедиться, что инициализация действительно стирает */
    bignum_t num;
    num.len = BIGNUM_CAPACITY;                     /* произвольное (может быть >0) */
    memset(num.words, 0xFF, sizeof(num.words));   /* заполнить «мусором» */

    /* Вызываем функцию из статической библиотеки */
    int rc = bignum_init(&num);

    /* Ожидаем успешный код возврата и полностью нулевое состояние */
    assert(rc == BIGNUM_INIT_SUCCESS);
    assert(num.len == 0);
    for (size_t i = 0; i < BIGNUM_CAPACITY; ++i)
        assert(num.words[i] == 0);

    printf("PASSED\n");
    return 0;
}
