#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "buf.h"
#include "file.h"


uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


// fetch the disk blocks
uint64
block_info(uint64 fd) {

  struct file *f_info = myproc()->ofile[fd]; // file str from file descriptor
  struct inode *ip = f_info->ip; 

  int ip_sz = ip->size; // inode from file str
  for(int i = 0; i < (ip_sz + BSIZE- 1)/ BSIZE; i++){
    printf("block %d : %d\n", i, ip->addrs[i]); // block no.
    struct buf *bfptr = bread(ip->dev, ip->addrs[i]); // read contents of block
    printf("%s\n", (char*)bfptr->data); //print the contents of the block
    brelse(bfptr);// release buffer
  }
  return 0;
}

uint64 
sys_getDiskBlock(void)
{
  uint64 fd;
  argaddr(0, &fd);
  return block_info(fd);
  
}
