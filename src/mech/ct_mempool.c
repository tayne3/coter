/**
 * @file ct_mempool.c
 * @brief 内存池实现
 * @author tayne3@dingtalk.com
 * @date 2023.12.03
 */

#include "ct_mempool.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// -------------------------[STATIC DECLARATION]-------------------------

#define STR_CURRTITLE "[ct_mempool]"

// -------------------------[GLOBAL DEFINITION]-------------------------

void *ct_mempool_malloc(ct_mempool_buf_t mempool, size_t size)
{
	return malloc(size);
	ct_unused(mempool);
}

void *ct_mempool_calloc(ct_mempool_buf_t mempool, size_t nmemb, size_t size)
{
	return calloc(nmemb, size);
	ct_unused(mempool);
}

void ct_mempool_free(ct_mempool_buf_t mempool, void *ptr)
{
	free(ptr);
	return;
	ct_unused(mempool);
}

// -------------------------[STATIC DEFINITION]-------------------------
