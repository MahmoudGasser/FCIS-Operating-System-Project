// User-level Semaphore

#include "inc/lib.h"

struct semaphore create_semaphore(char *semaphoreName, uint32 value)
{
	//TODO: [PROJECT'24.MS3 - #02] [2] USER-LEVEL SEMAPHORE - create_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("create_semaphore is not implemented yet");
	//Your Code is Here...
	struct __semdata *sd = smalloc(semaphoreName,sizeof(struct __semdata),1);

    strcpy(sd->name, semaphoreName);
	sd->count= (int)value;
	sd->lock=0;
	LIST_INIT(&sd->queue);

	struct semaphore s;
	s.semdata = sd;

	return s;
}

struct semaphore get_semaphore(int32 ownerEnvID, char* semaphoreName)
{
	//TODO: [PROJECT'24.MS3 - #03] [2] USER-LEVEL SEMAPHORE - get_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("get_semaphore is not implemented yet");
	//Your Code is Here...
	void* va = sget(ownerEnvID,semaphoreName);
	struct semaphore s ;
	if(va == NULL)
	{
		s.semdata=NULL;
	}
	else
	{
	s.semdata = va;
	}
	return s;

}

void wait_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #04] [2] USER-LEVEL SEMAPHORE - wait_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("wait_semaphore is not implemented yet");
	//Your Code is Here...
	while(xchg(&(sem.semdata->lock), 1) != 0);
	sem.semdata->count--;

	if(sem.semdata->count < 0)
	{
		sem.semdata->lock = 0;
		sys_enqueue_env((uint32)&sem.semdata->queue, (uint32)&sem);
	}
	else{
	sem.semdata->lock = 0;
	}
}


void signal_semaphore(struct semaphore sem)
{
	//TODO: [PROJECT'24.MS3 - #05] [2] USER-LEVEL SEMAPHORE - signal_semaphore
	//COMMENT THE FOLLOWING LINE BEFORE START CODING
	//panic("signal_semaphore is not implemented yet");
	//Your Code is Here...
	while(xchg(&(sem.semdata->lock), 1) != 0);

	sem.semdata->count++;

	if (sem.semdata->count <= 0)
	{
		sem.semdata->lock = 0;
		struct Env *env = (struct Env*)sys_dequeue_env((uint32)&sem.semdata->queue);

	}
	sem.semdata->lock = 0;
}


int semaphore_count(struct semaphore sem)
{
	return sem.semdata->count;
}
