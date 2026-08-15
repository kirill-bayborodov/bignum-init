/**
 * @file    bignum_init.h
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    2026-08-15
 *
 * @brief   Публичный API для инициализации bignum_t.
 *
 * @details
 *   Выполняет in‑place (на месте) инициализацию переменной.
 *   Функция потокобезопасна, если каждый поток работает с
 *   независимыми объектами `bignum_t`.
 *
 *   **Алгоритм:**<br>
 *   1. Проверка аргумента на `NULL`.
 *   2. Обнуление массива слов (`words`).
 *   3. Обнуление поля `len` (число становится 0).
 *
 * @see     bignum.h
 * @since   1.0.0
 *
 * @history
 *   - rev. 0 (2026-08-15): Первоначальное создание API.
 */

#pragma once

#ifndef BIGNUM_INIT_H
#define BIGNUM_INIT_H

#include "bignum.h"   /* определяет bignum_t и BIGNUM_CAPACITY */

/* ------------------------------------------------------------------ */
/*  Коды состояния функции bignum_init                               */
/* ------------------------------------------------------------------ */
typedef enum {
    BIGNUM_INIT_SUCCESS        = 0,  /**< Успех. Переменная инициализирована. */
    BIGNUM_INIT_ERROR_NULL_ARG = -1, /**< Указатель `x` равен NULL. */
} bignum_init_status_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Инициализирует объект `bignum_t` (обнуляет `words` и `len`).
 *
 * @param[in,out] x  Указатель на структуру, которую следует инициализировать.
 *
 * @return
 *   - `BIGNUM_INIT_SUCCESS`        – инициализация прошла успешно.<br>
 *   - `BIGNUM_INIT_ERROR_NULL_ARG` – передан `NULL`.
 */
bignum_init_status_t bignum_init(bignum_t *restrict x);

#ifdef __cplusplus
}
#endif

#endif /* BIGNUM_INIT_H */
