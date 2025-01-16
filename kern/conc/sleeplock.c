// Sleeping locks

#include "inc/types.h"
#include "inc/x86.h"
#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/environment_definitions.h"
#include "inc/assert.h"
#include "inc/string.h"
#include "sleeplock.h"
#include "channel.h"
#include "../cpu/cpu.h"
#include "../proc/user_environment.h"

void init_sleeplock(struct sleeplock *lk, char *name)
{
	init_channel(&(lk->chan), "sleep lock channel");
	init_spinlock(&(lk->lk), "lock of sleep lock");
	strcpy(lk->name, name);
	lk->locked = 0;
	lk->pid = 0;
}
int holding_sleeplock(struct sleeplock *lk)
{
	int r;
	acquire_spinlock(&(lk->lk));
	r = lk->locked && (lk->pid == get_cpu_proc()->env_id);
	release_spinlock(&(lk->lk));
	return r;
}
//==========================================================================

void acquire_sleeplock(struct sleeplock *lk)
{
    //TODO: [PROJECT'24.MS1 - #13] [4] LOCKS - acquire_sleeplock
    //COMMENT THE FOLLOWING LINE BEFORE START CODING
    //panic("acquire_sleeplock is not implemented yet");
    //Your Code is Here...

    if(holding_sleeplock(lk))
      panic("acquire_sleeplock: lock \"%s\" is already held by the same CPU.", lk->name);

    acquire_spinlock(&(lk->lk));
    while (lk->locked == 1)
    {
        sleep(&(lk->chan),&(lk->lk));
    }
    lk->locked = 1;
    __sync_synchronize();
    lk->pid = get_cpu_proc()->env_id;

    release_spinlock(&(lk->lk));
}

void release_sleeplock(struct sleeplock *lk)
{
    //TODO: [PROJECT'24.MS1 - #14] [4] LOCKS - release_sleeplock
    //COMMENT THE FOLLOWING LINE BEFORE START CODING
    //panic("release_sleeplock is not implemented yet");
    //Your Code is Here...

  if(!holding_sleeplock(lk))
    panic("release_sleeplock: lock \"%s\" is already held by the same CPU.", lk->name);

    acquire_spinlock(&(lk->lk));
    lk->locked = 0;

    if(queue_size(&(lk->chan.queue)) != 0)
      wakeup_all(&(lk->chan));

    release_spinlock(&(lk->lk));
}