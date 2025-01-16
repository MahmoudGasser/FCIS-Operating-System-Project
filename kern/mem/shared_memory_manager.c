#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/environment_definitions.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/syscall.h>
#include "kheap.h"
#include "memory_manager.h"

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
struct Share* get_share(int32 ownerID, char* name);

//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_spinlock(&AllShares.shareslock, "shares lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}

//==============================
// [2] Get Size of Share Object:
//==============================
int getSizeOfSharedObject(int32 ownerID, char* shareName)
{
	//[PROJECT'24.MS2] DONE
	// This function should return the size of the given shared object
	// RETURN:
	//	a) If found, return size of shared object
	//	b) Else, return E_SHARED_MEM_NOT_EXISTS
	//
	struct Share* ptr_share = get_share(ownerID, shareName);
	if (ptr_share == NULL)
		return E_SHARED_MEM_NOT_EXISTS;
	else
		return ptr_share->size;

	return 0;
}

//===========================================================


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] Create frames_storage:
//===========================

// Create the frames_storage and initialize it by 0
inline struct FrameInfo** create_frames_storage(int numOfFrames)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_frames_storage()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_frames_storage is not implemented yet");
	//Your Code is Here...
    struct FrameInfo** frames_storage_arr = (struct FrameInfo**)kmalloc(numOfFrames * sizeof(struct FrameInfo*));
     if(frames_storage_arr==NULL)
     {
    	 return NULL;
     }
     else
     {
    	 for(int i = 0 ; i < numOfFrames ; i++){
    	 		frames_storage_arr[i] = 0;
    	 	}

    	     return  frames_storage_arr;
     }

}

//=====================================
// [2] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* create_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'24.MS2 - #16] [4] SHARED MEMORY - create_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_share is not implemented yet");
	//Your Code is Here...
	struct Share* new_share =  (struct Share*)kmalloc(sizeof(struct Share));

	 if (new_share == NULL) {

	        return NULL;
	    }
	struct FrameInfo** framesStorage = create_frames_storage(ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE);
	if(framesStorage == NULL){
		kfree((void*)new_share);
		return NULL;
	}
	new_share->ownerID = ownerID;
	new_share->size = size;
	new_share->isWritable = isWritable;
	new_share->references = 1;
	new_share->ID =((int32)new_share & 0x7FFFFFFF);
	new_share->framesStorage = framesStorage;
    strcpy(new_share->name, shareName);

	return new_share;

}

//=============================
// [3] Search for Share Object:
//=============================
//Search for the given shared object in the "shares_list"
//Return:
//	a) if found: ptr to Share object
//	b) else: NULL
struct Share* get_share(int32 ownerID, char* name)
{
	//TODO: [PROJECT'24.MS2 - #17] [4] SHARED MEMORY - get_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
    //panic("get_share is not implemented yet");
	//Your Code is Here...

	struct Share *MyShare;
	acquire_spinlock(&AllShares.shareslock);
	LIST_FOREACH(MyShare,&(AllShares.shares_list))
	{
		if(MyShare->ownerID == ownerID && strcmp(MyShare->name, name) == 0 )
		{
			release_spinlock(&AllShares.shareslock);
			return MyShare;
		}
	}
	release_spinlock(&AllShares.shareslock);
    return NULL;

}

//=========================
// [4] Create Share Object:
//=========================
int createSharedObject(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #19] [4] SHARED MEMORY [KERNEL SIDE] - createSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("createSharedObject is not implemented yet");
	//Your Code is Here...
	 uint32 va = (uint32)virtual_address;
	struct Env* myenv = get_cpu_proc(); //The calling environment

	struct Share* sharedobj=get_share(ownerID,shareName);
	if (sharedobj!=NULL)
	{
		return E_SHARED_MEM_EXISTS;
	}

		struct Share* obj=create_share(ownerID, shareName,  size, isWritable);
		if (obj==NULL)
		{
			return E_NO_SHARE;
		}
		else
		{


		   uint32* ptr_page_table =NULL;
		   int index =0;
		   for(uint32 i_va = va ;i_va<(va+size);i_va+=PAGE_SIZE)
		   {
			   struct FrameInfo* frame;
			   allocate_frame(&frame);
                  get_page_table(myenv->env_page_directory,i_va,&ptr_page_table);
                  if(ptr_page_table==NULL)
                  {
                	  create_page_table(myenv->env_page_directory,i_va);
                  }
				   map_frame(myenv->env_page_directory,frame,(uint32)i_va,PERM_WRITEABLE|PERM_USER);


			   obj->framesStorage[index++] = frame;
		    }

		   acquire_spinlock(&AllShares.shareslock);
		   LIST_INSERT_TAIL(&AllShares.shares_list,obj);
		   release_spinlock(&AllShares.shareslock);


			return obj->ID;
		}




}


//======================
// [5] Get Share Object:
//======================
int getSharedObject(int32 ownerID, char* shareName, void* virtual_address)
{
	//TODO: [PROJECT'24.MS2 - #21] [4] SHARED MEMORY [KERNEL SIDE] - getSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("getSharedObject is not implemented yet");

	//Your Code is Here...

	struct Env* myenv = get_cpu_proc(); //The calling environment
	uint32 size =getSizeOfSharedObject( ownerID, shareName);
	int num_of_pages= ROUNDUP(size, PAGE_SIZE) / PAGE_SIZE;
	struct Share* obj=get_share(ownerID,shareName);
	if (obj==NULL)
	{
		return E_SHARED_MEM_NOT_EXISTS;
	}

	uint32 i_va = (uint32)virtual_address;
	for(int i=0;i<num_of_pages;i++)
	{
		struct FrameInfo* frame =obj->framesStorage[i];
		if (obj->isWritable)
		{


			map_frame(myenv->env_page_directory,frame,i_va,PERM_WRITEABLE | PERM_USER);
		}
		else
		{

			map_frame(myenv->env_page_directory,frame,i_va,PERM_USER);
		}
		i_va+=PAGE_SIZE;

	}


	return obj->ID;
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//==========================
// [B1] Delete Share Object:
//==========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - free_share()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("free_share is not implemented yet");
	//Your Code is Here...

}
//========================
// [B2] Free Share Object:
//========================
int freeSharedObject(int32 sharedObjectID, void *startVA)
{
	//TODO: [PROJECT'24.MS2 - BONUS#4] [4] SHARED MEMORY [KERNEL SIDE] - freeSharedObject()
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	panic("freeSharedObject is not implemented yet");
	//Your Code is Here...

}
