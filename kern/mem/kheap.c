#include "kheap.h"

#include <inc/memlayout.h>
#include <inc/dynamic_allocator.h>
#include "memory_manager.h"


//Initialize the dynamic allocator of kernel heap with the given start address, size & limit
//All pages in the given range should be allocated
//Remember: call the initialize_dynamic_allocator(..) to complete the initialization
//Return:
//	On success: 0
//	Otherwise (if no memory OR initial size exceed the given limit): PANIC
int initialize_kheap_dynamic_allocator(uint32 daStart, uint32 initSizeToAllocate, uint32 daLimit)
{
	//TODO: [PROJECT'24.MS2 - #01] [1] KERNEL HEAP - initialize_kheap_dynamic_allocator
	// Write your code here, remove the panic and write your code
	//panic("initialize_kheap_dynamic_allocator() is not implemented yet...!!");
	if(daStart+initSizeToAllocate>daLimit)
			panic("ERROR: Initial size exceed the given limit)\n");

	initSizeToAllocate = ROUNDUP(initSizeToAllocate,PAGE_SIZE);
	start = daStart;
	brk = daStart + initSizeToAllocate;
	hard_limit = daLimit;
	cprintf("hard_limit : \n",hard_limit);
	uint32 endpage = daStart;
	while(endpage < brk){
	struct FrameInfo* frame_info;
	allocate_frame(&frame_info);
	frame_info->mappedVA = endpage;
	map_frame(ptr_page_directory,frame_info,endpage,PERM_WRITEABLE|PERM_PRESENT|PERM_MODIFIED);
		endpage += PAGE_SIZE;
	}

	initialize_dynamic_allocator(start,initSizeToAllocate);


	return 0;
}

void* sbrk(int numOfPages)
{
	/* numOfPages > 0: move the segment break of the kernel to increase the size of its heap by the given numOfPages,
	 * 				you should allocate pages and map them into the kernel virtual address space,
	 * 				and returns the address of the previous break (i.e. the beginning of newly mapped memory).
	 * numOfPages = 0: just return the current position of the segment break
	 *
	 * NOTES:
	 * 	1) Allocating additional pages for a kernel dynamic allocator will fail if the free frames are exhausted
	 * 		or the break exceed the limit of the dynamic allocator. If sbrk fails, return -1
	 */

	//MS2: COMMENT THIS LINE BEFORE START CODING==========
	//return (void*)-1 ;
	//====================================================

	//TODO: [PROJECT'24.MS2 - #02] [1] KERNEL HEAP - sbrk
	// Write your code here, remove the panic and write your code
	//panic("sbrk() is not implemented yet...!!");

	 if (numOfPages == 0)
	        return (void*)brk;
	    uint32 old_brk = brk;
	    uint32 allocation_limit = brk + numOfPages*PAGE_SIZE;

	    if (allocation_limit > hard_limit)
	    {
	    	return (void*)-1 ;
	    }
	    for (uint32 addr = brk; addr < allocation_limit; addr += PAGE_SIZE)
	    {
	        if (LIST_SIZE(&MemFrameLists.free_frame_list) == 0)
	        {
	        	return (void*)-1 ;
	        }

	        struct FrameInfo* frame_info;
	        allocate_frame(&frame_info);
	        frame_info->mappedVA = addr;

	        map_frame(ptr_page_directory, frame_info, addr, PERM_WRITEABLE|PERM_PRESENT|PERM_MODIFIED);
	    }

	    brk = allocation_limit;
	    uint32 *END_Block =(uint32*)allocation_limit-1 ;
	    *END_Block = 0|1;  //size | isAllocated  0x01
	    return (void*)old_brk;
}

//TODO: [PROJECT'24.MS2 - BONUS#2] [1] KERNEL HEAP - Fast Page Allocator

void* kmalloc(unsigned int size)
{
	//TODO: [PROJECT'24.MS2 - #03] [1] KERNEL HEAP - kmalloc
	// Write your code here, remove the panic and write your code
	//kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	// use "isKHeapPlacementStrategyFIRSTFIT() ..." functions to check the current strategy

	//cprintf("111\n");
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE){
		if(isKHeapPlacementStrategyFIRSTFIT())
			return alloc_block_FF(size);
		if(isKHeapPlacementStrategyBESTFIT())
			return alloc_block_BF(size);
	}

	int num_of_pages = ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	void* va = (void*)hard_limit + PAGE_SIZE;
	struct FrameInfo *i_frame,*frame;
	void* i_va = va;
	int counter = 0,found = 0;

	do{
		uint32 *i_page_table = (uint32*)get_page_table(ptr_page_directory, (uint32)i_va, &i_page_table);
		i_frame = get_frame_info(ptr_page_directory, (uint32)i_va, &i_page_table);
		if(i_frame == NULL || i_frame->references == 0){
			counter++;
			if(counter == num_of_pages){
				found = 1;
				uint32 *page_table = (uint32*)get_page_table(ptr_page_directory, (uint32)va, &page_table);
				frame = get_frame_info(ptr_page_directory, (uint32)va, &page_table);
				break;
			}
		}else{
			counter = 0;
			va = i_va + PAGE_SIZE;
		}
		i_va += PAGE_SIZE;
	}while(i_va < (void*)KERNEL_HEAP_MAX);

	if(found == 1)
	{
		i_va = va;
		for(int i = 0; i < num_of_pages; i++)
		{
			allocate_frame(&frame);
			map_frame(ptr_page_directory,frame,(uint32)i_va,PERM_WRITEABLE);
			uint32 *i_page_table = (uint32*)get_page_table(ptr_page_directory, (uint32)i_va, &i_page_table);
			frame = get_frame_info(ptr_page_directory, (uint32)i_va, &i_page_table);
			frame->mappedVA = (uint32)i_va;

			if (i==0)
			{
				Framearr[to_frame_number(frame)].number_of_frames = num_of_pages;

			}
			else
			{
				Framearr[to_frame_number(frame)].number_of_frames = 0;

			}
			Framearr[to_frame_number(frame)].virtual_adress = i_va;
			i_va += PAGE_SIZE;

		}
	}
	else{
		return NULL;
	}





	return va;
}

void kfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #04] [1] KERNEL HEAP - kfree
	// Write your code here, remove the panic and write your code
	//panic("kfree() is not implemented yet...!!");
    uint32 va = (uint32)virtual_address;
    if (va>= (PAGE_SIZE+hard_limit) && va< KERNEL_HEAP_MAX){
    	uint32 num_pages;
    	uint32* ptr_page_table = NULL;
    	struct FrameInfo* frame_info = get_frame_info(ptr_page_directory,va, &ptr_page_table);
    	if (frame_info!=NULL){
    		    	num_pages = Framearr[to_frame_number(frame_info)].number_of_frames;
    		    	Framearr[to_frame_number(frame_info)].number_of_frames = 0;
    	}

		uint32 i_va = va;
		for (uint32 i = 1; i <= num_pages; i++){

			uint32* ptr_page_table = NULL;

			struct FrameInfo* frame_info = get_frame_info(ptr_page_directory,i_va, &ptr_page_table);
			if (frame_info != NULL){
				unmap_frame(ptr_page_directory,i_va);
				frame_info->mappedVA = 0;
			}
			i_va +=(PAGE_SIZE);
		}
	}else if(va>=KERNEL_HEAP_START && va< hard_limit){


		   free_block(virtual_address);
	 }else{
		 panic("INVALID ADDRESS");
	 }

	//you need to get the size of the given allocation using its address
	//refer to the project presentation and documentation for details
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #05] [1] KERNEL HEAP - kheap_physical_address
	// Write your code here, remove the panic and write your code
	//panic("kheap_physical_address() is not implemented yet...!!");

	uint32* ptr_page_table = NULL;
	uint32 pagetable = get_page_table(ptr_page_directory,virtual_address,&ptr_page_table);
	struct FrameInfo* frameinfo = get_frame_info(ptr_page_directory,virtual_address,&ptr_page_table);

	if (frameinfo !=NULL){
		return (to_physical_address(frameinfo)  + PGOFF(virtual_address)) ;
	}else{
		return 0;
	}

	//return the physical address corresponding to given virtual_address
	//refer to the project presentation and documentation for details

	//EFFICIENT IMPLEMENTATION ~O(1) IS REQUIRED ==================
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
    struct FrameInfo* frameOfpa = to_frame_info(physical_address);
    uint32 offset = physical_address % PAGE_SIZE;

    if (frameOfpa == NULL || frameOfpa->references == 0) {
        return 0;
    }else{
    	{
    		return (frameOfpa->mappedVA)+offset;
    	}

    	}
    }

//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, if moved to another loc: the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT'24.MS2 - BONUS#1] [1] KERNEL HEAP - krealloc
	// Write your code here, remove the panic and write your code
	panic("krealloc() is not implemented yet...!!");
	//return NULL;
}
