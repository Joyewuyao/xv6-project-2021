#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/* Possible states of a thread: */
#define FREE        0x0
#define RUNNING     0x1
#define RUNNABLE    0x2

#define STACK_SIZE  8192
#define MAX_THREAD  4

// 结构体定义，用于保存线程的上下文信息
struct thread_context {
  uint64 ra;    // 返回地址寄存器
  uint64 sp;    // 栈指针寄存器
  uint64 fp;    // 帧指针寄存器（s0）
  uint64 s1;    // 通用寄存器 s1
  uint64 s2;    // 通用寄存器 s2
  uint64 s3;    // 通用寄存器 s3
  uint64 s4;    // 通用寄存器 s4
  uint64 s5;    // 通用寄存器 s5
  uint64 s6;    // 通用寄存器 s6
  uint64 s7;    // 通用寄存器 s7
  uint64 s8;    // 通用寄存器 s8
  uint64 s9;    // 通用寄存器 s9
  uint64 s10;   // 通用寄存器 s10
  uint64 s11;   // 通用寄存器 s11
};

// 结构体定义，表示一个线程的基本信息和状态
struct thread {
  char stack[STACK_SIZE];    // 线程的栈，用于保存局部变量和调用栈信息
  int state;                 // 线程的状态，可能为 FREE、RUNNING、RUNNABLE 等
  struct thread_context context;  // 保存线程的上下文信息的结构体
};


struct thread all_thread[MAX_THREAD];
struct thread *current_thread;
extern void thread_switch(uint64, uint64);
              
void 
thread_init(void)
{
  // main() is thread 0, which will make the first invocation to
  // thread_schedule().  it needs a stack so that the first thread_switch() can
  // save thread 0's state.  thread_schedule() won't run the main thread ever
  // again, because its state is set to RUNNING, and thread_schedule() selects
  // a RUNNABLE thread.
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
}

void 
thread_schedule(void)
{
  struct thread *t, *next_thread;

  /* Find another runnable thread. */
  next_thread = 0;
  t = current_thread + 1;
  for(int i = 0; i < MAX_THREAD; i++){
    if(t >= all_thread + MAX_THREAD)
      t = all_thread;
    if(t->state == RUNNABLE) {
      next_thread = t;
      break;
    }
    t = t + 1;
  }

  if (next_thread == 0) {
    printf("thread_schedule: no runnable threads\n");
    exit(-1);
  }

  if (current_thread != next_thread) {         /* switch threads?  */
    next_thread->state = RUNNING;
    t = current_thread;
    current_thread = next_thread;
    /* YOUR CODE HERE
     * Invoke thread_switch to switch from t to next_thread:
     * thread_switch(??, ??);
     */
    thread_switch((uint64)&t->context, (uint64)&next_thread->context);
  } else
    next_thread = 0;
}

// 函数定义，用于创建一个新线程
void 
thread_create(void (*func)()) {
  struct thread *t;  // 声明指向 struct thread 结构体的指针

  // 遍历线程数组以找到一个空闲的位置来创建新线程
  for (t = all_thread; t < all_thread + MAX_THREAD; t++) {
    if (t->state == FREE) break;  // 找到空闲位置即退出循环
  }
  
  t->state = RUNNABLE;  // 将线程状态设置为 RUNNABLE（可运行）

  // 设置新线程的上下文信息
  t->context.ra = (uint64)func;  // 设置返回地址寄存器为传入的函数地址
  t->context.sp = (uint64)&t->stack[STACK_SIZE - 1];  // 设置栈指针寄存器为栈的顶部
  t->context.fp = (uint64)&t->stack[STACK_SIZE - 1];  // 设置帧指针寄存器为栈的顶部
}


void 
thread_yield(void)
{
  current_thread->state = RUNNABLE;
  thread_schedule();
}

volatile int a_started, b_started, c_started;
volatile int a_n, b_n, c_n;

void 
thread_a(void)
{
  int i;
  printf("thread_a started\n");
  a_started = 1;
  while(b_started == 0 || c_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_a %d\n", i);
    a_n += 1;
    thread_yield();
  }
  printf("thread_a: exit after %d\n", a_n);

  current_thread->state = FREE;
  thread_schedule();
}

void 
thread_b(void)
{
  int i;
  printf("thread_b started\n");
  b_started = 1;
  while(a_started == 0 || c_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_b %d\n", i);
    b_n += 1;
    thread_yield();
  }
  printf("thread_b: exit after %d\n", b_n);

  current_thread->state = FREE;
  thread_schedule();
}

void 
thread_c(void)
{
  int i;
  printf("thread_c started\n");
  c_started = 1;
  while(a_started == 0 || b_started == 0)
    thread_yield();
  
  for (i = 0; i < 100; i++) {
    printf("thread_c %d\n", i);
    c_n += 1;
    thread_yield();
  }
  printf("thread_c: exit after %d\n", c_n);

  current_thread->state = FREE;
  thread_schedule();
}

int 
main(int argc, char *argv[]) 
{
  a_started = b_started = c_started = 0;
  a_n = b_n = c_n = 0;
  thread_init();
  thread_create(thread_a);
  thread_create(thread_b);
  thread_create(thread_c);
  thread_schedule();
  exit(0);
}
