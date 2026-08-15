; -----------------------------------------------------------------------------
; @file    bignum_init.asm
; @author  git@bayborodov.com
; @version 1.0.0
; @date    15.08.2026
;
; @brief   Реализация функций ядра больших целых чисел (bignum) на YASM x86_64.
;
; @details
;   Эталонная ассемблерная реализация для Yasm x86-64 (System V ABI).
;   Модуль предоставляет функцию bignum_init для инициализации больших чисел,
;   определённых типом bignum_t. Реализованы проверки корректности входных указателей.
;
;   Структура для представления большого беззнакового целого числа.   
;   typedef struct {
;       /** Массив 64-битных "слов" для хранения числа. */
;       uint64_t words[BIGNUM_CAPACITY];
;       /**
;        * Количество используемых слов.
;        * Для числа 0 значение len равно 0.
;        * Для всех остальных чисел гарантируется, что words[len-1] != 0
;        * (число нормализовано).
;        */
;       size_t   len;
;   } bignum_t;
;
; @history
;   - rev. 0 (15.08.2026): Первоначальная реализация на ассемблере.
; -----------------------------------------------------------------------------
; SPDX‑License‑Identifier: MIT
; -----------------------------------------------------------------------------

default rel

section .text
    align 16
    global bignum_init

; --------------------------------------------------------------
; Константы
BIGNUM_CAPACITY         equ 32
BIGNUM_WORD_SIZE        equ 8
BIGNUM_OFFSET_WORDS     equ 0
BIGNUM_OFFSET_LEN       equ BIGNUM_CAPACITY * BIGNUM_WORD_SIZE   ; 256

BIGNUM_SUCCESS          equ 0
BIGNUM_ERROR_NULL_ARG   equ -1
BIGNUM_ERROR_OVERFLOW   equ -2

BIGNUM_INIT_SUCCESS          equ 0
BIGNUM_INIT_ERROR_NULL_ARG   equ -1

BUF_QWORDS              equ BIGNUM_CAPACITY + 1   ; 32 words + len
; --------------------------------------------------------------
; void bignum_init(bignum_t *x)
; rdi = x
bignum_init:
    ; проверка аргумента
    test    rdi, rdi
    jz      .ret_error          ; rdi == NULL → ошибка

    ; очистить всю структуру (33 qwords)
    mov     rcx, BUF_QWORDS
    xor     rax, rax
    rep     stosq

    ; успех
    xor     eax, eax            ; BIGNUM_INIT_SUCCESS (0)
    ret

.ret_error:
    mov     eax, BIGNUM_INIT_ERROR_NULL_ARG
    ret

section .note.GNU-stack noalloc noexec nowrite progbits