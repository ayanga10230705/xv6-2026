## 实验环境搭建
1. 克隆 xv6 源码
   git clone https://github.com/mit-pdos/xv6-public.git
   cd xv6-public
2. 安装编译依赖（Ubuntu）
3. 编译运行测试
   make qemu
   能正常进入 xv6 shell 表示环境搭建成功。

--------

## 第一层任务：机制观察
### 任务1：系统调用路径跟踪
- 用户程序：`trace.c`，打印 `[USER] calling write`
  新建`trace.c`文件
      ```C
      #include "types.h"
      #include "user.h"

      int main() {
         printf(1, "[USER] calling write\n");  // 第一层
         write(1, "hello xv6\n", 10);
         exit();
      }
      ```

- 内核 `syscall.c`：添加 `[KERNEL] enter syscall`
   把`syscall ()`函数修改为：
      ```C
      void
      syscall(void)
      {
      int num;
      struct proc *curproc = myproc();
      num = curproc->tf->eax;

      if(num == SYS_write && curproc->pid > 2)
         cprintf("[KERNEL] enter syscall\n");
      if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
         curproc->tf->eax = syscalls[num]();
      } else {
         cprintf("%d %s: unknown sys call %d\n",
                  curproc->pid, curproc->name, num);
         curproc->tf->eax = -1;}
      }
      ```

- 内核 `sysfile.c`：添加 `[KERNEL] sys_write invoked`
   把`sys_write ()`函数修改为：
      ```C
      int
      sys_write(void)
      {
      struct file *f;
      int n;
      char *p;
      struct proc *curproc = myproc(); // 加上这一行！

         if(curproc->pid > 2) {
         cprintf("[KERNEL] sys_write invoked\n");
      }

      if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
         return -1;
      return filewrite(f, p, n);
      }
      ```

- `Makefile`中`UPROGS`的最后一行添加：`_trace\`

- 运行输出：
  [USER] calling write
  [KERNEL] enter syscall
  [KERNEL] sys_write invoked
  hello xv6

### 任务2：调度过程观察
- 修改 `proc.c` 的 `scheduler()` 函数
   在 `swtch (&cpu->scheduler, p->context); `前添加：`cprintf("[SCHED] switch to pid=%d\n", p->pid);`

- 观察到进程按 PID 交替执行。

### 任务3：内存分配观察
- 修改 `kalloc.c`
   打开 `kalloc.c`，找到 `kalloc ()` 函数,在返回前添加：
      ```C
      if(r)
         cprintf("[MEM] alloc page at 0x%x\n", (uint)r);
      ```
- 观察到内存页连续分配、地址从高到低增长。

----------

## 第二层任务：系统调用扩展
新增系统调用 `hello()`
1. `syscall.h` 加调用号：`#define SYS_hello 22`

2. `syscall.c` 注册 
   在文件上方加声明：`extern int sys_hello(void);`
   在 `syscalls` 数组加：`[SYS_hello]   sys_hello,`

3. `sysproc.c` 实现内核输出
   在文件的最后添加：
   ```C
      int
      sys_hello(void)
      {
      cprintf("Hello from xv6 kernel!\n");
      return 0;
      }
   ```

4. `user.h`、`usys.S` 添加用户态接口
   `user.h` 中在一堆函数声明最后加：`int hello(void);`
   `usys.S` 最后面加：`SYSCALL(hello)`

5. 编写测试程序 `hello_test.c`
   ```C
   #include "types.h"
   #include "user.h"

   int main(void)
   {
   hello();
   exit();
   }
   ```

6. Makefile 添加 `_hello_test\`

7. 运行输出：
   ```
   Hello from xv6 kernel!
   ```

--------

## 实践心得
通过本次 xv6 操作系统内核实践，我对操作系统的运行机制有了更加直观、深入的理解。在之前的学习中，我只是从理论上了解系统调用、进程调度、内存管理等概念，而本次通过修改内核代码、添加日志、编译运行，真正看到了操作系统从用户态进入内核态、进程切换、物理内存分配的完整过程，让抽象的知识变得具体可感。

在完成系统调用跟踪任务时，我通过在用户程序、系统调用入口、write 实现处添加打印，清晰地看到一条完整调用路径，理解了用户程序如何通过中断陷入内核、内核如何分发并执行系统调用，体会到操作系统的分层设计与权限隔离思想。在观察进程调度时，不断切换的 PID 让我直观感受到时间片轮转调度的工作方式；内存分配日志则让我看到物理页的申请与地址分布，理解了内存管理的基本逻辑。新增系统调用让我掌握了在内核中添加功能的完整流程。

这次实践不仅巩固了理论知识，更让我体会到操作系统设计的严谨与精妙，为后续深入学习计算机系统底层原理打下了扎实基础。