/**
 * @file    bignum_init.c
 * @author  git@bayborodov.com
 * @version 1.0.0
 * @date    2026-08-15
 *
 * @brief   Реализация ядра bignum: инициализация.Инициализирует bignum.
 *
 * @details Выполняет in-place (на месте) инициалиацию переменной.
 * @param   x Указатель на структуру; может быть NULL.
 * @return  0 при успехе или BIGNUM_INIT_ERROR_NULL_ARG при NULL.
 * 
 * @history
 *   - rev. 0 (2026-08-15): Первоначальная реализация.
 */

#include <string.h>
#include "bignum_init.h"

/* ------------------------------------------------------------------ */
/*  Инициализация                                                     */
/* ------------------------------------------------------------------ */

int bignum_init(bignum_t *x)
{
    if (x == NULL) {
        return BIGNUM_INIT_ERROR_NULL_ARG;
    }
    memset(x, 0, sizeof(*x));
    return BIGNUM_INIT_SUCCESS;
}

