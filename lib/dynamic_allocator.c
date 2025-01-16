/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

//=====================================================
// 1) GET BLOCK SIZE (including size of its meta data):
//=====================================================
__inline__ uint32 get_block_size(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (*curBlkMetaData) & ~(0x1);
}

//===========================
// 2) GET BLOCK STATUS:
//===========================
__inline__ int8 is_free_block(void* va)
{
	uint32 *curBlkMetaData = ((uint32 *)va - 1) ;
	return (~(*curBlkMetaData) & 0x1) ;
}

//===========================
// 3) ALLOCATE BLOCK:
//===========================

void *alloc_block(uint32 size, int ALLOC_STRATEGY)
{
	void *va = NULL;
	switch (ALLOC_STRATEGY)
	{
	case DA_FF:
		va = alloc_block_FF(size);
		break;
	case DA_NF:
		va = alloc_block_NF(size);
		break;
	case DA_BF:
		va = alloc_block_BF(size);
		break;
	case DA_WF:
		va = alloc_block_WF(size);
		break;
	default:
		cprintf("Invalid allocation strategy\n");
		break;
	}
	return va;
}

//===========================
// 4) PRINT BLOCKS LIST:
//===========================

void print_blocks_list(struct MemBlock_LIST list)
{
	cprintf("=========================================\n");
	struct BlockElement* blk ;
	cprintf("\nDynAlloc Blocks List:\n");
	LIST_FOREACH(blk, &list)
	{
		cprintf("(size: %d, isFree: %d)\n", get_block_size(blk), is_free_block(blk)) ;
	}
	cprintf("=========================================\n");

}
//
////********************************************************************************//
////********************************************************************************//

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

bool is_initialized = 0;
//==================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//==================================
void initialize_dynamic_allocator(uint32 daStart, uint32 initSizeOfAllocatedSpace)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (initSizeOfAllocatedSpace % 2 != 0) initSizeOfAllocatedSpace++; //ensure it's multiple of 2
		if (initSizeOfAllocatedSpace == 0)
			return ;
		is_initialized = 1;
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #04] [3] DYNAMIC ALLOCATOR - initialize_dynamic_allocator
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("initialize_dynamic_allocator is not implemented yet");
	//Your Code is Here...
	uint32 *BEG_Block =(uint32*)daStart ;
	*BEG_Block = 0|1;  //size | isAllocated  0x01
	uint32 *END_Block =(uint32*)(daStart + initSizeOfAllocatedSpace - sizeof(int));
	*END_Block = 0|1;  //size | isAllocated

	uint32 Blockaddress = daStart + 2*sizeof(int);
	struct BlockElement* firstBlock = (struct BlockElement*)Blockaddress;
	uint32* header = (uint32*) ((uint32)firstBlock - 4);
	uint32* footer = (uint32*) ((uint32)firstBlock + initSizeOfAllocatedSpace - 4*sizeof(int));
	*header = (initSizeOfAllocatedSpace-8) | 0;
	*footer = (initSizeOfAllocatedSpace-8) | 0;

	LIST_INIT(&freeBlocksList);
	LIST_INSERT_TAIL(&freeBlocksList,firstBlock);

}
//==================================
// [2] SET BLOCK HEADER & FOOTER:
//==================================
void set_block_data(void* va, uint32 totalSize, bool isAllocated)
{
	//TODO: [PROJECT'24.MS1 - #05] [3] DYNAMIC ALLOCATOR - set_block_data
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("set_block_data is not implemented yet");
	//Your Code is Here...
	    // Calculate the address of the header
	    uint32 *header = (uint32 *)((uint32)va - sizeof(uint32));
	    *header = totalSize | isAllocated;
	    //cprintf("*header = %d\n",*header);
	    // Calculate the address of the footer
	    uint32 *footer = (uint32 *)((uint32)va + totalSize - 2 * sizeof(uint32));
	    *footer = totalSize | isAllocated;
	    //cprintf("*footer = %d\n",*footer);
}
//=========================================
// [3] ALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *alloc_block_FF(uint32 size)
{
	//==================================================================================
	//DON'T CHANGE THESE LINES==========================================================
	//==================================================================================
	{
		if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
		if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
			size = DYN_ALLOC_MIN_BLOCK_SIZE ;
		if (!is_initialized)
		{
			uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
			uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
			uint32 da_break = (uint32)sbrk(0);
			initialize_dynamic_allocator(da_start, da_break - da_start);
		}
	}
	//==================================================================================
	//==================================================================================

	//TODO: [PROJECT'24.MS1 - #06] [3] DYNAMIC ALLOCATOR - alloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_FF is not implemented yet");
	//Your Code is Here...
	if(size==0)
		return NULL;
    struct BlockElement *block;
    uint32 requiredSize;
    requiredSize=size + 8;

	LIST_FOREACH(block,&(freeBlocksList)){
		    uint32 currentblockSize = get_block_size((void*) block);
		    requiredSize=size + 8;
		    if(currentblockSize == requiredSize){
		    	LIST_REMOVE(&freeBlocksList,block);
		    	set_block_data((void*)block,requiredSize,1);
		    	return (struct BlockElement *)(block);

		    }
		    else if(currentblockSize >= requiredSize){
				if(currentblockSize - 16 >= ( requiredSize )){
					struct BlockElement *new_block;
					uint32 newAddres;
					newAddres = (uint32)block+requiredSize;
					new_block = (struct BlockElement *)newAddres;
					uint32  newSize=currentblockSize-(requiredSize);
					LIST_INSERT_AFTER(&(freeBlocksList),block,new_block);
					LIST_REMOVE(&freeBlocksList,block);
					set_block_data((void*)block,requiredSize, 1);
					set_block_data((void*)new_block,newSize , 0);
					return (struct BlockElement *)(block);
				}
				else{

					LIST_REMOVE(&freeBlocksList,block);
					set_block_data((void*)block,get_block_size(block), 1);
					return(struct BlockElement *) (block );
				}
			}
		}

		void* check = sbrk(ROUNDUP(requiredSize, PAGE_SIZE)/PAGE_SIZE);
		if(check == (void*)-1||check== NULL)
			return NULL;
		block=check;
		if(LIST_LAST(&freeBlocksList)!=NULL&&is_free_block((void*) (block-4))){
			requiredSize =get_block_size((check-4))+ROUNDUP(requiredSize, PAGE_SIZE);
			set_block_data(LIST_LAST(&freeBlocksList),requiredSize,0);
		}
		else{
			requiredSize=ROUNDUP(requiredSize, PAGE_SIZE);
			set_block_data(block,requiredSize,0);
			LIST_INSERT_TAIL(&freeBlocksList,block);
		}
	    struct BlockElement *AllocateBlock=alloc_block_FF(size);
	    return AllocateBlock;
}

//=========================================
// [4] ALLOCATE BLOCK BY BEST FIT:
//=========================================
void *alloc_block_BF(uint32 size)
{
	//TODO: [PROJECT'24.MS1 - BONUS] [3] DYNAMIC ALLOCATOR - alloc_block_BF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("alloc_block_BF is not implemented yet");
	//Your Code is Here...

	if (size % 2 != 0) size++;	//ensure that the size is even (to use LSB as allocation flag)
	if (size < DYN_ALLOC_MIN_BLOCK_SIZE)
	size = DYN_ALLOC_MIN_BLOCK_SIZE ;
	if (!is_initialized)
	{
		uint32 required_size = size + 2*sizeof(int) /*header & footer*/ + 2*sizeof(int) /*da begin & end*/ ;
		uint32 da_start = (uint32)sbrk(ROUNDUP(required_size, PAGE_SIZE)/PAGE_SIZE);
		uint32 da_break = (uint32)sbrk(0);
		initialize_dynamic_allocator(da_start, da_break - da_start);
	}
	if(size ==0)
		return NULL;
	uint32 smallestsizefound = UINT_MAX;
    struct BlockElement *bestblock ;
    struct BlockElement *block;
    uint32 required_size = size + 8;
	LIST_FOREACH(block ,&(freeBlocksList))
	{
		uint32 currentblockSize = get_block_size((void*) block);
	    if(currentblockSize >= required_size)
	    {
	        if(currentblockSize < smallestsizefound)
	        {
	        	smallestsizefound = currentblockSize;
	        	bestblock = block;
	        }
	        if(smallestsizefound==0)
	        	break;
	        }
		}
	if(smallestsizefound == UINT_MAX)
		return NULL;
	uint32 bestblocksize = get_block_size((void*) bestblock);
	if(bestblocksize - required_size >= 16)
	{
	    struct BlockElement *newblock;
	    uint32 NewAddress = (uint32)bestblock + required_size;
	    newblock = (struct BlockElement*) NewAddress;
	    uint32 newsize = bestblocksize - required_size;
	    LIST_INSERT_AFTER(&(freeBlocksList),bestblock,newblock);
	    LIST_REMOVE(&freeBlocksList,bestblock);
	    set_block_data((void*)bestblock,required_size, 1);
	    set_block_data((void*)newblock,newsize , 0);
	    return (struct BlockElement *)(bestblock);
	}else{
	    LIST_REMOVE(&freeBlocksList,bestblock);
	    set_block_data((void*)bestblock,get_block_size(bestblock), 1);
	    return (void *)(bestblock);
	}
	return NULL;
}
//===================================================
// [5] FREE BLOCK WITH COALESCING:
//===================================================

void free_block(void *va) {

//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
//COMMENT THE FOLLOWING LINE BEFORE START CODING
//panic("free_block is not implemented yet");
//Your Code is Here...

	if (va == NULL )  return;

	uint32 new_size = 0;
    bool is_Added = 0;

    struct BlockElement *current_block;
    LIST_FOREACH(current_block, &(freeBlocksList)) {
        if (current_block > (struct BlockElement*)va) {
            LIST_INSERT_BEFORE(&freeBlocksList, current_block,(struct BlockElement*)va);
            is_Added = 1;
            break;
        }
    }

    if (!is_Added) {
        LIST_INSERT_TAIL(&freeBlocksList, (struct BlockElement*)va);
    }

    // Variables for merging
    struct BlockElement *next_header = (struct BlockElement*)((char*)va + get_block_size(va));
    struct BlockElement *prev_block = LIST_PREV((struct BlockElement*)va);
    struct BlockElement *next_block = LIST_NEXT((struct BlockElement*)va);
    struct BlockElement *prev_footer = (struct BlockElement*)((char*)va - sizeof(uint32));


    // Merge with both previous and next free blocks
    if (prev_block && next_block && is_free_block(prev_footer) && is_free_block(next_header)) {
        new_size = get_block_size(next_block) + get_block_size(prev_block) + get_block_size(va);
        set_block_data(prev_block, new_size, 0);
        LIST_REMOVE(&freeBlocksList, next_block);
        LIST_REMOVE(&freeBlocksList, (struct BlockElement*)va);
        return;
    }

    // Merge with the previous free block
     if (prev_block && is_free_block(prev_footer)) {
        new_size = get_block_size(prev_block) + get_block_size(va);
        set_block_data(prev_block, new_size, 0);
        LIST_REMOVE(&freeBlocksList, (struct BlockElement*)va);
        return;
    }

    // Merge with the next free block
    if (next_block && is_free_block(next_header)) {
    	new_size = get_block_size(next_block) + get_block_size(va);
        set_block_data(va, new_size, 0);
        LIST_REMOVE(&freeBlocksList, next_block);
        return;
    }

    new_size = get_block_size(va);
    set_block_data(va, new_size, 0);
    return;
}

//void free_block(void *va) {
//	//TODO: [PROJECT'24.MS1 - #07] [3] DYNAMIC ALLOCATOR - free_block
//    //COMMENT THE FOLLOWING LINE BEFORE START CODING
//	//panic("free_block is not implemented yet");
//	//Your Code is Here...
//    if (va == NULL || is_free_block(va)) {
//        return;
//    }
//
//    struct BlockElement *current_block = (struct BlockElement*)va;
//    uint32 currentSize = get_block_size((void *)va);
//
//    struct BlockElement *nextblock = (struct BlockElement*)((uint32)current_block + currentSize);
//    struct BlockElement *prevblock = (struct BlockElement*)((uint32)current_block - get_block_size((uint32*)((uint32)va - 4)));
//
//    bool nextfree = is_free_block((void *)nextblock);
//    bool prevfree = is_free_block((void *)prevblock);
//
//    if (nextblock&&prevblock&&nextfree && prevfree) {
//        uint32 totalSize = currentSize + get_block_size((void *)nextblock) + get_block_size((void *)prevblock);
//        set_block_data((void*)prevblock, totalSize, 0);
//        struct BlockElement *i_block;
//        LIST_REMOVE(&freeBlocksList, nextblock);
//    }
//    else if (nextblock&&nextfree) {
//        uint32 totalSize = currentSize + get_block_size((void *)nextblock);
//        set_block_data((void*)current_block, totalSize, 0);
//        struct BlockElement *i_block;
//        bool is_add=1;
//        LIST_FOREACH(i_block,&(freeBlocksList))
//        {
//             if ((uint32)i_block->prev_next_info.le_next == (uint32)nextblock)// ==
//             {
//                LIST_INSERT_AFTER(&freeBlocksList, i_block, current_block);
//                LIST_REMOVE(&freeBlocksList, nextblock);
//                is_add=0;
//                break;
//             }
//        }
//        if(is_add){
//            LIST_INSERT_HEAD(&freeBlocksList, current_block);
//            LIST_REMOVE(&freeBlocksList, nextblock);
//
//        }
//    }
//    else if (prevblock&&prevfree) {
//        uint32 totalSize = currentSize + get_block_size((void *)prevblock);
//        set_block_data((void*)prevblock, totalSize, 0);
//    }
//    else {
//        set_block_data((void*)current_block, currentSize, 0);
//        struct BlockElement *i_block;
//        bool is_added=1;
//        LIST_FOREACH(i_block,&(freeBlocksList))
//        {
//
//             if ((uint32)i_block > (uint32)current_block)// ==
//             {
//                LIST_INSERT_BEFORE(&freeBlocksList, i_block, current_block);
//                is_added = 0;
//                break;
//             }
//        }
//        if(is_added)
//            LIST_INSERT_TAIL(&freeBlocksList, current_block);
//    }
//}
//=========================================
// [6] REALLOCATE BLOCK BY FIRST FIT:
//=========================================
void *realloc_block_FF(void* va, uint32 new_size)
{
	//TODO: [PROJECT'24.MS1 - #08] [3] DYNAMIC ALLOCATOR - realloc_block_FF
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("realloc_block_FF is not implemented yet");
	//Your Code is Here...
	if (new_size % 2 != 0)
	{
		new_size++;
	}

	if(va == NULL && new_size == 0)
	{
		return NULL;
	}
	if (va == NULL)
	{
		return alloc_block_FF(new_size);
	}
	if (new_size == 0)
	{
		free_block(va);
		return NULL;
	}

	uint32 old_size = (uint32)get_block_size(va)-8;
	struct BlockElement *block = (struct BlockElement*)va;
	void* next_block_address = (void*)((uint32)va+old_size+8);
	struct BlockElement *the_next_block = (struct BlockElement*)next_block_address;
	uint32 next_block_size = get_block_size(the_next_block);

	if (old_size == new_size){}
	else if (old_size > new_size && is_free_block(next_block_address))
	{
		LIST_REMOVE(&freeBlocksList, the_next_block);
		set_block_data(va, new_size+8, 1);
		next_block_address = (void*)((uint32)va+new_size+8);
		set_block_data(next_block_address, next_block_size+(old_size-new_size), 1);
		free_block(next_block_address);
	}
	else if (old_size > new_size && old_size-new_size >= 16)
	{
		set_block_data(va, new_size+8, 1);
		next_block_address = (void*)((uint32)va+new_size+8);
		set_block_data(next_block_address, old_size-new_size, 1);
		free_block(next_block_address);
	}
	else if (old_size > new_size){}
	else if (old_size < new_size && is_free_block(next_block_address) && new_size-old_size <= next_block_size)
	{
		if (next_block_size-(new_size-old_size) >= 16)
		{
			LIST_REMOVE(&freeBlocksList, the_next_block);
			set_block_data(va, new_size+8, 1);
			next_block_address = (void*)((uint32)va+new_size+8);
			free_block(next_block_address);
		}
		else{
			LIST_REMOVE(&freeBlocksList, the_next_block);
			set_block_data(va, next_block_size+new_size, 1);
		}
	}
	else if (old_size < new_size)
	{
		set_block_data(va, new_size+8, 1);
	}
	return va;
}
/*********************************************************************************************/
/*********************************************************************************************/
/*********************************************************************************************/
//=========================================
// [7] ALLOCATE BLOCK BY WORST FIT:
//=========================================
void *alloc_block_WF(uint32 size)
{
	panic("alloc_block_WF is not implemented yet");
	return NULL;
}

//=========================================
// [8] ALLOCATE BLOCK BY NEXT FIT:
//=========================================
void *alloc_block_NF(uint32 size)
{
	panic("alloc_block_NF is not implemented yet");
	return NULL;
}
