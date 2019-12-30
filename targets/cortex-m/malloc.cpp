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

//#define MALLOC_DEBUG

#ifdef MALLOC_DEBUG
#define MYDBG(fmt, ...)	DBGC("malloc", fmt, ## __VA_ARGS__)
#define _MYDBG(...)	_DBG(__VA_ARGS__)

ALWAYS_INLINE uint32_t __LR()
{
    uint32_t res;
    __asm volatile ("mov %0,lr" : "=r" (res));
    return res;
}

#else
#define MYDBG(...)
#define _MYDBG(...)
#endif

void* calloc(size_t size, size_t count)
{
    size *= count;
    void* ptr = malloc(size);
    if (ptr)
        memset(ptr, 0, size);
    return ptr;
}

struct free_list
{
    struct free_list* next;
    size_t size;
};


static free_list* __free;

void* __heap_top = &__heap_start;
void* __heap_limit = &__heap_end;
size_t __free_fragments;

#ifdef CORTEX_TRACK_STACK_USAGE
size_t stack_max_used()
{
    size_t* p = (size_t*)&__stack_start;
    size_t* e = (size_t*)&__stack_end;

    while (p < e && *p == STACK_MAGIC)
        p++;

    return (e - p) * sizeof(size_t);
}
#endif

void* _malloc_r(_reent* _, size_t size) { return malloc(size); }
void _free_r(_reent* _, void* ptr) { free(ptr); }

#ifdef MALLOC_DEBUG
void dump_free_chain()
{
    MYDBG("freeblock chain, %d bytes total:", __free_fragments);
    for (free_list* p = __free; p; p = p->next)
        _MYDBG("  %d @ %p-%p", p->size, p, (size_t)p + p->size);
    _MYDBG("  top @ %p + %d unused = %p\n", __heap_top, (size_t)__heap_limit - (size_t)__heap_top, __heap_limit);
}
#else
#define dump_free_chain()
#endif

#define SMALLEST_BLOCK	8
#define REQUIRED_BLOCK(n)	(((n) + SMALLEST_BLOCK - 1) & ~3)
#define MEM_SIZE(ptr)	(((size_t*)(ptr))[-1])
#define BLOCK_ADDR(ptr)	((size_t*)(ptr) - 1)

void* mtrim(void* ptr, size_t size)
{
    if (size)
    {
        size = REQUIRED_BLOCK(size);
        size_t cur = MEM_SIZE(ptr);
        if (cur > size + SMALLEST_BLOCK)
        {
            // there is space to be trimmed - we divide the allocated block in two and free the second part
            MYDBG("trimming %d bytes off %d byte block @ %p\n", cur - size, cur, BLOCK_ADDR(ptr));
            MEM_SIZE(ptr) = size;	// new size of the first block
            void* p2 = (uint8_t*)ptr + size;
            MEM_SIZE(p2) = cur - size; 	// remainder to be freed
            free(p2);
        }
    }

    return ptr;
}

void* realloc(void* ptr, size_t size)
{
    size_t curSize = MEM_SIZE(ptr);
    if (size <= curSize)
        return mtrim(ptr, size);

    // TODO: expand in case there is enough free space after the current block

    // fallback: allocate a whole new block
    void* pNew = malloc(size);
    memcpy(pNew, ptr, curSize);
    free(ptr);
    return pNew;
}

void* malloc(size_t size)
{
    if (!size)
        return NULL;

#ifdef MALLOC_DEBUG
    auto lr = __LR();
#endif
    size = REQUIRED_BLOCK(size);
    free_list** pp = &__free;
    void* res = NULL;

    for (free_list* p = *pp; p; pp = &p->next, p = p->next)
    {
        if (p->size == size)
        {
            // a free block matches our requirement perfectly
            *pp = p->next;
            res = p;
            MYDBG("exact free block match @ %p\n", p);
            break;
        }

        if (p->size >= size + SMALLEST_BLOCK)
        {
            // a big enough block to slice a part of
            res = p;
            *pp = (free_list*)((uint8_t*)p + size);
            (*pp)->next = p->next;
            (*pp)->size = p->size - size;
            MYDBG("partial free block match @ %p\n", p);
            break;
        }
    }

    if (!res)
    {
        // not enough space, let's grow the heap
        void* brkptr = (uint8_t*)__heap_top + size;
        if (brkptr > __heap_limit)
        {
            DBGC("malloc", "failed, not enough memory left\n");
            return NULL;
        }
        else
        {
            MYDBG("heap size increased by %d bytes\n", size);
        }

        res = __heap_top;
        __heap_top = brkptr;
    }
    else
    {
        __free_fragments -= size;
    }

    MYDBG("allocated %d bytes @ %p - %p from %p\n", size, res, (uint8_t*)res + size, lr);

    dump_free_chain();

    // res is actually start of block, first word is size, the rest is to be returned
    *(size_t*)res = size;
    return (size_t*)res + 1;
}

void* malloc_once(size_t size)
{
    void* ptr = (uint8_t*)__heap_limit - size;
    if (ptr < __heap_top)
    {
        DBGC("malloc_once", "failed, not enough memory left (%p - %u = %p < %p)\n", __heap_limit, size, ptr, __heap_top);
        return NULL;
    }
    __heap_limit = ptr;
#ifdef CORTEX_TRACK_STACK_USAGE
    MYDBG("allocated %d bytes @ %p - %p at the top of the heap, max stack used so far: %d\n", size, ptr, (uint8_t*)ptr + size, stack_max_used());
#else
    MYDBG("allocated %d bytes @ %p - %p at the top of the heap\n", size, ptr, (uint8_t*)ptr + size);
#endif
    return ptr;
}

void free(void* ptr)
{
    if (!ptr)
        return;

    free_list** pp = &__free;
    ptr = (size_t*)ptr - 1;
    size_t size = *(size_t*)ptr;
    void* end = (uint8_t*)ptr + size;
    free_list* cur = (free_list*)ptr;

    MYDBG("freeing %d bytes @ %p - %p\n", size, ptr, end);

    for (free_list* p = *pp; p && p <= end; pp = &p->next, p = p->next)
    {
        if (p == end)
        {
            // the block being freed is immediately before another free block
            cur->next = p->next;
            cur->size = p->size + size;
            MYDBG("prepending to free block @ %p, resulting block is %d bytes @ %p - %p\n", p, cur->size, ptr, (uint8_t*)p + cur->size);
            goto done;
        }

        if ((uint8_t*)p + p->size == ptr)
        {
            // the block being freed is immediately after another free block
            cur = p;
            if (p->next == end)
            {
                // ...and is immediately followed by yet another free block
                p->size += size + p->next->size;
                MYDBG("joining free blocks @ %p and %p, resulting block is %d bytes @ %p - %p\n", p, p->next, p->size, p, (uint8_t*)p + p->size);
                p->next = p->next->next;
            }
            else
            {
                p->size += size;
                MYDBG("appending to free block @ %p, resulting block is %d bytes @ %p - %p\n", p, p->size, p, (uint8_t*)p + p->size);
            }
            goto done;
        }
    }

    // we've created a new free block
    cur->next = *pp;
    cur->size = size;

done:
    __free_fragments += size;
    if ((uint8_t*)cur + cur->size == __heap_top)
    {
        // let's shrink the heap
        ASSERT(!cur->next);
        MYDBG("heap size decreased by %d bytes\n", cur->size);
        __heap_top = cur;
        __free_fragments -= cur->size;
        *pp = NULL;
    }
    else
    {
        *pp = cur;
    }

    dump_free_chain();
}

void* operator new(size_t size) __attribute__((alias("malloc")));
void* operator new[](size_t size) __attribute__((alias("malloc")));
void operator delete(void* ptr) __attribute__((alias("free")));
void operator delete[](void* ptr) __attribute__((alias("free")));
