/*!
    \file    ibl_heap.c
    \brief   IBL heap for GD32 SDK

    \version 2024-06-30, V1.0.0, demo for GD32
*/

/*
    Copyright (c) 2024, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors
       may be used to endorse or promote products derived from this software without
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "ibl_includes.h"

/**
 * @brief Byte alignment requirements.
 */
#define IBL_HEAP_ALIGNMENT            8
#define IBL_HEAP_ALIGNMENT_MASK        0x7

/* Block sizes must not get too small. */
#define MINIMUM_BLOCK_SIZE    ((size_t)(heap_struct_sz << 1))

/**
 * @brief The linked list structure.
 *
 * This is used to link free blocks in order of their memory address.
 */
typedef struct block_link
{
    struct block_link *pnext;    /**< The next free block in the list. */
    size_t blk_sz;                        /**< The size of the free block. */
} block_link_t;


/* Allocate the memory for the heap. */
static uint8_t ibl_heap[IBL_HEAP_SIZE];


/**
 * @brief The size of the structure placed at the beginning of each allocated
 * memory block must by correctly byte aligned.
 */
static const size_t heap_struct_sz = (sizeof(block_link_t) + ((size_t)(IBL_HEAP_ALIGNMENT - 1))) & (~((size_t)IBL_HEAP_ALIGNMENT_MASK));

/**
 * @brief Create a couple of list links to mark the start and end of the list.
 */
static block_link_t xstart, *pxend;

/**
 * @brief Keeps track of the number of free bytes remaining, but says nothing
 * about fragmentation.
 */
static size_t free_bytes_remaining;
//static size_t xMinimumEverFreeBytesRemaining = 0U;

/**
 * @brief Gets set to the top bit of an size_t type.
 *
 * When this bit in the blk_sz member of an block_link_t structure is set
 * then the block belongs to the application. When the bit is free the block is
 * still part of the free heap space.
 */
static size_t block_alloc_bit;


/*-----------------------------------------------------------*/
static void ibl_heap_init(void)
{
    block_link_t *pfirst;
    uint8_t *palign_heap;
    size_t uxaddr;
    size_t tot_sz = IBL_HEAP_SIZE;

    memset(ibl_heap, 0, sizeof(ibl_heap));
    pxend = NULL;
    free_bytes_remaining = 0U;
    block_alloc_bit = 0;

    /* Ensure the heap starts on a correctly aligned boundary. */
    uxaddr = (size_t)ibl_heap;

    if ((uxaddr & IBL_HEAP_ALIGNMENT_MASK) != 0) {
        uxaddr += ((size_t)IBL_HEAP_ALIGNMENT - 1);
        uxaddr &= ~((size_t)IBL_HEAP_ALIGNMENT_MASK);
        tot_sz -= uxaddr - (size_t)ibl_heap;
    }

    palign_heap = (uint8_t *)uxaddr;

    /* xstart is used to hold a pointer to the first item in the list of free
     * blocks.  The void cast is used to prevent compiler warnings. */
    xstart.pnext = (void *)palign_heap;
    xstart.blk_sz = (size_t)0;

    /* pxend is used to mark the end of the list of free blocks and is inserted
     * at the end of the heap space. */
    uxaddr = ((size_t)palign_heap) + tot_sz;
    uxaddr -= heap_struct_sz;
    uxaddr &= ~((size_t)IBL_HEAP_ALIGNMENT_MASK);
    pxend = (void *) uxaddr;
    pxend->blk_sz = 0;
    pxend->pnext = NULL;

    /* To start with there is a single free block that is sized to take up the
     * entire heap space, minus the space taken by pxend. */
    pfirst = (void *)palign_heap;
    pfirst->blk_sz = uxaddr - (size_t)pfirst;
    pfirst->pnext = pxend;

    /* Only one block exists - and it covers the entire usable heap space. */
    //xMinimumEverFreeBytesRemaining = pfirst->blk_sz;
    free_bytes_remaining = pfirst->blk_sz;

    /* Work out the position of the top bit in a size_t variable. */
    block_alloc_bit = ((size_t)1) << ((sizeof(size_t) * BITS_PER_BYTE) - 1);
}

static void insert_block_to_free(block_link_t *pblk_to_insert)
{
    block_link_t *piter;
    uint8_t *puc;

    /* Iterate through the list until a block is found that has a higher address
     * than the block being inserted. */
    for (piter = &xstart; piter->pnext < pblk_to_insert; piter = piter->pnext) {
        /* Nothing to do here, just iterate to the right position. */
    }

    /* Do the block being inserted, and the block it is being inserted after
     * make a contiguous block of memory? */
    puc = (uint8_t *)piter;
    if ((puc + piter->blk_sz) == (uint8_t *)pblk_to_insert) {
        piter->blk_sz += pblk_to_insert->blk_sz;
        pblk_to_insert = piter;
    }

    /* Do the block being inserted, and the block it is being inserted before
     * make a contiguous block of memory? */
    puc = (uint8_t *)pblk_to_insert;
    if ((puc + pblk_to_insert->blk_sz) == (uint8_t *)piter->pnext) {
        if (piter->pnext != pxend) {
            /* Form one big block from the two blocks. */
            pblk_to_insert->blk_sz += piter->pnext->blk_sz;
            pblk_to_insert->pnext = piter->pnext->pnext;
        } else {
            pblk_to_insert->pnext = pxend;
        }
    } else {
        pblk_to_insert->pnext = piter->pnext;
    }

    /* If the block being inserted plugged a gab, so was merged with the block
     * before and the block after, then it's pnext pointer will have
     * already been set, and should not be set here as that would make it point
     * to itself. */
    if (piter != pblk_to_insert) {
        piter->pnext = pblk_to_insert;
    }
}

void *ibl_malloc(uint32_t xsize)
{
    block_link_t *pblk, *pprev_blk, *pnext_blk;
    void *pvReturn = NULL;

    /* If this is the first call to malloc then the heap will require
     * initialisation to setup the list of free blocks. */
    if (pxend == NULL) {
        ibl_heap_init();
    }

    /* Check the requested block size is not so large that the top bit is set.
     * The top bit of the block size member of the block_link_t structure is used
     * to determine who owns the block - the application or the kernel, so it
     * must be free. */
    if ((xsize & block_alloc_bit) == 0) {
        /* The wanted size is increased so it can contain a block_link_t
         * structure in addition to the requested amount of bytes. */
        if (xsize > 0) {
            xsize += heap_struct_sz;

            /* Ensure that blocks are always aligned to the required number of
             * bytes. */
            if ((xsize & IBL_HEAP_ALIGNMENT_MASK) != 0x00) {
                /* Byte alignment required. */
                xsize += (IBL_HEAP_ALIGNMENT - (xsize & IBL_HEAP_ALIGNMENT_MASK));
                IBL_ASSERT((xsize & IBL_HEAP_ALIGNMENT_MASK ) == 0);
            }
        }

        if ((xsize > 0) && (xsize <= free_bytes_remaining)) {
            /* Traverse the list from the start (lowest address) block until
             * one of adequate size is found. */
            pprev_blk = &xstart;
            pblk = xstart.pnext;
            while ((pblk->blk_sz < xsize) && (pblk->pnext != NULL)) {
                pprev_blk = pblk;
                pblk = pblk->pnext;
            }

            /* If the end marker was reached then a block of adequate size was
             * not found. */
            if (pblk != pxend) {
                /* Return the memory space pointed to - jumping over the
                 * block_link_t structure at its start. */
                pvReturn = (void *)(((uint8_t *)pprev_blk->pnext) + heap_struct_sz);

                /* This block is being returned for use so must be taken out
                 * of the list of free blocks. */
                pprev_blk->pnext = pblk->pnext;

                /* If the block is larger than required it can be split into
                 * two. */
                if ((pblk->blk_sz - xsize) > MINIMUM_BLOCK_SIZE) {
                    /* This block is to be split into two.  Create a new
                     * block following the number of bytes requested. The void
                     * cast is used to prevent byte alignment warnings from the
                     * compiler. */
                    pnext_blk = (void *)(((uint8_t *)pblk) + xsize);
                    IBL_ASSERT((((size_t)pnext_blk) & IBL_HEAP_ALIGNMENT_MASK) == 0);

                    /* Calculate the sizes of two blocks split from the single
                     * block. */
                    pnext_blk->blk_sz = pblk->blk_sz - xsize;
                    pblk->blk_sz = xsize;

                    /* Insert the new block into the list of free blocks. */
                    insert_block_to_free(pnext_blk);
                }

                free_bytes_remaining -= pblk->blk_sz;

                /*if (free_bytes_remaining < xMinimumEverFreeBytesRemaining) {
                    xMinimumEverFreeBytesRemaining = free_bytes_remaining;
                }*/

                /* The block is being returned - it is allocated and owned by
                 * the application and has no "next" block. */
                pblk->blk_sz |= block_alloc_bit;
                pblk->pnext = NULL;
            }
        }
    }
    IBL_ASSERT((((size_t)pvReturn) & (size_t)IBL_HEAP_ALIGNMENT_MASK) == 0);
    return pvReturn;
}

void * ibl_calloc(size_t count, size_t size)
{
    void *mem_ptr;

    mem_ptr = ibl_malloc(count * size);
    if (mem_ptr)
        memset(mem_ptr, 0, (count*size));

    return mem_ptr;
}

void ibl_free(void *pv)
{
    uint8_t *puc = (uint8_t *)pv;
    block_link_t *plink;

    if (pv != NULL) {
        /* The memory being freed will have an block_link_t structure immediately
         * before it. */
        puc -= heap_struct_sz;

        /* This casting is to keep the compiler from issuing warnings. */
        plink = (void *) puc;

        /* Check the block is actually allocated. */
        IBL_ASSERT((plink->blk_sz & block_alloc_bit) != 0);
        IBL_ASSERT(plink->pnext == NULL);

        if ((plink->blk_sz & block_alloc_bit) != 0) {
            if (plink->pnext == NULL) {
                /* The block is being returned to the heap - it is no longer
                 * allocated. */
                plink->blk_sz &= ~block_alloc_bit;

                /* Add this block to the list of free blocks. */
                free_bytes_remaining += plink->blk_sz;
                insert_block_to_free(((block_link_t *)plink));
            }
        }
    }
}

uint32_t ibl_free_heap_size(void)
{
    return free_bytes_remaining;
}

/*-----------------------------------------------------------*/

