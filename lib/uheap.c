#include <inc/lib.h>


struct FrameElements
{
	uint32 number_of_frames;
	 void* virtual_adress;

	 uint32 enter;
};

struct FrameElements framearr[NUM_OF_UHEAP_PAGES];

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=============================================
// [1] CHANGE THE BREAK LIMIT OF THE USER HEAP:
//=============================================
/*2023*/
void* sbrk(int increment)
{
	return (void*) sys_sbrk(increment);
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================
void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #12] [3] USER HEAP [USER SIDE] - malloc()
	//Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	//return NULL;
	//Use sys_isUHeapPlacementStrategyFIRSTFIT() and sys_isUHeapPlacementStrategyBESTFIT()
	//to check the current strategy
	if (size <= DYN_ALLOC_MAX_BLOCK_SIZE){
		if(sys_isUHeapPlacementStrategyFIRSTFIT())
			return alloc_block_FF(size);
		if(sys_isUHeapPlacementStrategyBESTFIT())
			return alloc_block_BF(size);
	}

	uint32 newSize = ROUNDUP(size, PAGE_SIZE);
	int num_of_pages = newSize / PAGE_SIZE;
	void* va = (void*)myEnv->user_hard_limit + PAGE_SIZE;
	void* i_va = va;
	int counter = 0,found = 0;

	do{
		int page_permissions = sys_get_page_permissions((uint32)i_va);

		if((page_permissions & PERM_AVAILABLE) != PERM_AVAILABLE|| page_permissions==-1)
		{
		//	cprintf("page_permissions & PERM_AVAILABLE %x\n",page_permissions & PERM_AVAILABLE);


			counter++;
			if(counter == num_of_pages)
			{


				found = 1;
				break;
			}
		}
		else
		{
			//cprintf("x\n");

			counter = 0;
			va = i_va + PAGE_SIZE;
		}
		i_va += PAGE_SIZE;
	}while(i_va < (void*)USER_HEAP_MAX);

	if(found == 1)
	{

		sys_allocate_user_mem((uint32)va, newSize);
		uint32* ptr_page_table = NULL;
		//struct FrameInfo* frame_info = sys_get_frame_info((uint32)va,&ptr_page_table);
		for (int i=0;i<NUM_OF_UHEAP_PAGES;i++)
		{

			if (framearr[i].enter == 0)
			{

				framearr[i].number_of_frames = num_of_pages;
				framearr[i].virtual_adress = va;
				framearr[i].enter =1;

				break;
			}
		}


     return va;
	}
	else
	{
		return NULL;
	}

//cprintf("va %x \n",va);
	return va;
}


//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
void free(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #14] [3] USER HEAP [USER SIDE] - free()
	// Write your code here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");
	uint32 va = (uint32)virtual_address;

	if(va>=USER_HEAP_START && va< myEnv->user_hard_limit)//Not myEnv
	{
          free_block(virtual_address);
	}
	else if (va<USER_HEAP_MAX && va >= (myEnv->user_hard_limit) + PAGE_SIZE)
	{
		uint32 numofpages;
		uint32* ptr_page_table = NULL;
		//struct FrameInfo* frame_info = sys_get_frame_info((uint32)va,&ptr_page_table);
				//if (frame_info!=NULL){
		for(int i=0;i<NUM_OF_UHEAP_PAGES;i++)
		{
			if(framearr[i].virtual_adress == virtual_address  && framearr[i].enter == 1)
			{
				numofpages = framearr[i].number_of_frames;
			    framearr[i].number_of_frames = 0;
			    framearr[i].enter =0;
			    break;
			}
		}

				//}
		uint32 size = numofpages * PAGE_SIZE;
		sys_free_user_mem(va,size);
//		uint32 i_va = va;
//		for (uint32 i = 1; i <= numofpages; i++){
//	  	    uint32* ptr_page_table = NULL;
//	    	struct FrameInfo* frame_info = sys_get_frame_info(myEnv->env_page_directory,i_va, &ptr_page_table);
//	    	if (frame_info != NULL){
//		    	sys_free_user_mem(va,size);
//		    }
//	     	i_va +=(PAGE_SIZE);
//		}
	}
	else
	{
		panic("INVALID ADDRESS");
	}
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	if (size == 0) return NULL ;
	//==============================================================
	//TODO: [PROJECT'24.MS2 - #18] [4] SHARED MEMORY [USER SIDE] - smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");
	uint32 newSize = ROUNDUP(size, PAGE_SIZE);
	void *va =malloc(newSize);

	if(va!=NULL)
	{
		 uint32 X = sys_createSharedObject(sharedVarName, newSize, isWritable,va);

         if (X!=E_SHARED_MEM_EXISTS && X!=E_NO_SHARE)
         {
        	 return va;
         }


	}
	else
	{
		return NULL;
	}

return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	uint32 size = sys_getSizeOfSharedObject(ownerEnvID,sharedVarName);

	if (size==0)
	{
		return NULL;
	}
	void* va = malloc(size);

	if (va!=NULL)
	{
		uint32 X = sys_getSharedObject(ownerEnvID,sharedVarName,va);

		if (E_SHARED_MEM_NOT_EXISTS == X)
		{
			return NULL;
		}
		return va;
	}
	else{
		return NULL;
	}


}




//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [USER SIDE] - sfree()
	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//[PROJECT]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
	return NULL;

}


//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//

void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");

}
