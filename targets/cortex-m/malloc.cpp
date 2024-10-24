/*
 * Copyright (c) 2018 triaxis s.r.o.
 * Licensed under the MIT license. See LICENSE.txt file in the repository root
 * for full license information.
 *
 * malloc.cpp
 *
 * Simple but efficient malloc implementation.
 *
 * Also provides malloc_once() for zero-overhead allocations of memory that will be never freed and
 * mtrim() for trimming overallocated memory in-place.
 */

#include <base/base.h>

#include <ld_symbols.h>

#define DIAG_ALLOC      1
#define DIAG_HEAP       2
#define DIAG_FREECHAIN  4

//#define MALLOC_DIAG     DIAG_ALLOC

#define MYDBG(...)      DBGCL("malloc", __VA_ARGS__)

ALWAYS_INLINE static uint32_t __LR()
{
    uint32_t res;
    __asm volatile ("mov %0,lr" : "=r" (res));
    return res;
}

#if TRACE || MINITRACE
#define CAPTURE_LR() UNUSED auto __lr = __LR() & ~1;
#else
#define CAPTURE_LR()
#endif

#if MALLOC_DIAG
#define MYDIAG(flag, ...)	if ((MALLOC_DIAG) & (flag)) { MYDBG(__VA_ARGS__); }
#else
#define MYDIAG(...)
#endif

static void* _malloc_impl(size_t size, bool clear);

OPTIMIZE void* calloc(size_t size, size_t count)
{
    return _malloc_impl(size * count, true);
}

struct free_list
{
    struct free_list* next;
    size_t size;
};

static struct
{
    free_list* free;
    size_t fragments;
    void* top = &__heap_start;
    void* limit = &__heap_end;
} __heap;

void* _malloc_r(_reent* _, size_t size) { return _malloc_impl(size, false); }
void _free_r(_reent* _, void* ptr) { free(ptr); }

#if (MALLOC_DIAG) & DIAG_FREECHAIN
void dump_free_chain()
{
    DBGC("malloc", "FREECHAIN(%4d):", __heap.fragments);
    for (free_list* p = __heap.free; p; p = p->next)
        _DBG(" %p+%d=%p", p, p->size, (size_t)p + p->size);
    _DBG(" (%p+%d=%p)\n", __heap.top, (size_t)__heap.limit - (size_t)__heap.top, __heap.limit);
}
#else
#define dump_free_chain()
#endif

#define SMALLEST_BLOCK	(sizeof(intptr_t) * 2)
#define BLOCK_ALIGNMENT 16
#define REQUIRED_BLOCK(n)	(((n) + sizeof(size_t) + BLOCK_ALIGNMENT - 1) & ~(BLOCK_ALIGNMENT - 1))
#define MEM_SIZE(ptr)	(((size_t*)(ptr))[-1])
#define BLOCK_ADDR(ptr)	((size_t*)(ptr) - 1)

void* mtrim(void* ptr, size_t size)
{
    if (size)
    {
        CAPTURE_LR();
        size = REQUIRED_BLOCK(size);
        size_t cur = MEM_SIZE(ptr);
        if (cur > size + SMALLEST_BLOCK)
        {
            // there is space to be trimmed - we divide the allocated block in two and free the second part
            MEM_SIZE(ptr) = size;	// new size of the first block
            void* p2 = (uint8_t*)ptr + size;
            MEM_SIZE(p2) = cur - size; 	// remainder to be freed
            MYDIAG(DIAG_ALLOC, "-[%p] %p-%d=%d +%p=%d", __lr, BLOCK_ADDR(ptr), cur - size, size, BLOCK_ADDR(p2), cur - size);
            free(p2);
        }
    }

    return ptr;
}

void* realloc(void* ptr, size_t size)
{
    size_t curSize = MEM_SIZE(ptr);
    if (REQUIRED_BLOCK(size) <= curSize)
        return mtrim(ptr, size);

    // TODO: expand in case there is enough free space after the current block

    // fallback: allocate a whole new block
    void* pNew = _malloc_impl(size, false);
    memcpy(pNew, ptr, curSize);
    free(ptr);
    return pNew;
}

OPTIMIZE void* malloc(size_t size)
{
    return _malloc_impl(size, false);
}

void* _malloc_impl(size_t size, bool clear)
{
    if (!size)
        return NULL;

    CAPTURE_LR();
    size = REQUIRED_BLOCK(size);
    free_list** pp = &__heap.free;
    void* res = NULL;

    for (free_list* p = *pp; p; pp = &p->next, p = p->next)
    {
        if (p->size == size)
        {
            // a free block matches our requirement perfectly
            *pp = p->next;
            res = p;
            MYDIAG(DIAG_ALLOC, "+[%p] %p=%d", __lr, p, size);
            break;
        }

        if (p->size >= size + SMALLEST_BLOCK)
        {
            // a big enough block to slice a part of
            res = p;
            *pp = (free_list*)((uint8_t*)p + size);
            (*pp)->next = p->next;
            (*pp)->size = p->size - size;
            MYDIAG(DIAG_ALLOC, "+[%p] %p<%d +%p=%d", __lr, p, size, *pp, (*pp)->size);
            break;
        }
    }

    if (!res)
    {
        // not enough space, let's grow the heap
        void* brkptr = (uint8_t*)__heap.top + size;
        if (brkptr > __heap.limit)
        {
            MYDBG("failed, not enough memory left");
            return NULL;
        }
        else
        {
            MYDIAG(DIAG_ALLOC, "+[%p] %p+%d=%p", __lr, __heap.top, size, brkptr);
            MYDIAG(DIAG_HEAP, "HEAP: %p+%d=%p", __heap.top, size, brkptr);
        }

        res = __heap.top;
        __heap.top = brkptr;
    }
    else
    {
        __heap.fragments -= size;
    }

    dump_free_chain();

    // res is actually start of block, first word is size, the rest is to be returned
    *(size_t*)res = size;
    if (clear)
    {
        for (size_t i = sizeof(size_t); i < size; i += sizeof(size_t))
        {
            *(size_t*)((intptr_t)res + i) = 0;
        }
    }
    return (size_t*)res + 1;
}

int __dbglines;

void* malloc_once(size_t size)
{
    CAPTURE_LR();
    void* ptr = (uint8_t*)__heap.limit - size;
    if (ptr < __heap.top)
    {
        DBGCL("malloc_once", "[%p] %p-%u=%p<%p", __lr, __heap.limit, size, ptr, __heap.top);
        return NULL;
    }
#if (MALLOC_DIAG) & DIAG_ALLOC
    DBGCL("malloc_once", "[%p] %p-%u=%p", __lr, __heap.limit, size, ptr);
#endif
    __heap.limit = ptr;
    return ptr;
}

void free(void* ptr)
{
    if (!ptr)
        return;

    CAPTURE_LR();
    free_list** pp = &__heap.free;
    ptr = (size_t*)ptr - 1;
    size_t size = *(size_t*)ptr;
    void* end = (uint8_t*)ptr + size;
    free_list* cur = (free_list*)ptr;

    for (free_list* p = *pp; p && p <= end; pp = &p->next, p = p->next)
    {
        if (p == end)
        {
            // the block being freed is immediately before another free block
            cur->next = p->next;
            cur->size = p->size + size;
            MYDIAG(DIAG_ALLOC, "-[%p] %p+%p=%d", __lr, cur, p, cur->size);
            goto done;
        }

        if ((uint8_t*)p + p->size == ptr)
        {
            // the block being freed is immediately after another free block
            if (p->next == end)
            {
                // ...and is immediately followed by yet another free block
                p->size += size + p->next->size;
                MYDIAG(DIAG_ALLOC, "-[%p] %p+%p+%p=%d", __lr, cur, p, p->next, p->size);
                p->next = p->next->next;
            }
            else
            {
                p->size += size;
                MYDIAG(DIAG_ALLOC, "-[%p] %p+%p=%d", __lr, cur, p, p->size);
            }
            cur = p;
            goto done;
        }
    }

    // we've created a new free block
    cur->next = *pp;
    cur->size = size;
    MYDIAG(DIAG_ALLOC, "-[%p] %p=%d", __lr, cur, size);

done:
    __heap.fragments += size;
    if ((uint8_t*)cur + cur->size == __heap.top)
    {
        // let's shrink the heap
        ASSERT(!cur->next);
        MYDIAG(DIAG_HEAP, "HEAP: %p-%d=%p", __heap.top, cur->size, cur);
        __heap.top = cur;
        __heap.fragments -= cur->size;
        *pp = NULL;
    }
    else
    {
        *pp = cur;
    }

    dump_free_chain();
}

void* operator new(size_t size) __attribute__((leaf, nothrow, alias("malloc")));
void* operator new[](size_t size) __attribute__((leaf, nothrow, alias("malloc")));
void operator delete(void* ptr) __attribute__((leaf, nothrow, alias("free")));
void operator delete[](void* ptr) __attribute__((leaf, nothrow, alias("free")));
