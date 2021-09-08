# Linux 僵尸进程的产生和销毁

Linux中用fork函数创建子进程。例如：

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
int main()
{
    pid_t pid;
    pid = fork();
    if(pid==0){
        printf("子进程执行区域\n");
    }else{
        printf("父进程执行区域\n");
    }
  
    printf("子进程和父进程都将执行该代码\n");
    return 0;
}
```



## 1、僵尸进程产生原因

### 1.1 父进程忙碌

出现僵尸进程的一个很重要的原因就是子进程已经执行完成，但父进程还在执行或处于休眠状态，且没有主动释放子进程的资源，此时该子进程就会变成僵尸进程。应为Linux系统中，子进程资源必须由父进程来释放，或是父进程结束，子进程才会结束。

test-fork.c

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
int main()
{
    pid_t pid;

    pid = fork();
    if(pid==0){
        printf("子进程执行区域\n");
      	exit(0);
    }else{
        while(1){
            printf("父进程执行区域\n");
        }
    }
    printf("子进程和父进程都将执行该代码\n");
    return 0;
}
```

运行结果

```shell
$ps au
ubuntu    29768  0.0  0.0   4512   728 pts/5    S+   08:21   0:00 ./test-fork
ubuntu    29769  0.0  0.0      0     0 pts/5    Z+   08:21   0:00 [test-fork] <defunct>

# 可见父进程在忙碌，没有及时回收子进程资源，子进程就变成僵尸进程
```

上述代码，子进程输出了一句话后就调用exit(0)正常退出进程，但其占用的进程ID还没释放，等着父进程主动释放，但父进程还处于忙碌状态，一直在循环输出，没有主动释放子进程ID。这这种状态下，该子进程就成为僵尸进程。

系统中能使用的进程ID是有限的，应该避免僵尸进程的产生。



### 1.2 父进程处于休眠状态

```c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
    // #include <stdlib.h>
    // #include <sys/wait.h>
    int main()
{
    pid_t pid;

    pid = fork();
    if(pid==0){
        printf("子进程执行区域\n");
        exit(0);
    }else{
        printf("父进程执行区域\n");
        sleep(12);
    }
    printf("子进程和父进程都将执行该代码\n");
    return 0;
}
```

运行结果

```shell
$ ps au
ubuntu    31179  0.0  0.0   4512   772 pts/5    S+   08:27   0:00 ./test-fork
ubuntu    31180  0.0  0.0      0     0 pts/5    Z+   08:27   0:00 [test-fork] <defunct>

# 当父进程在休眠书，没有及时回收子进程资源，子进程就变成僵尸进程
```



## 2、僵尸进程的销毁

为了销毁子进程，父进程应该主动请求返回子进程的返回值，共有两种方法，分别是wait函数，waitpid函数

### 2.1 利用wait函数

```c
#include <sys/wait.h>

pid_t wait(int *statloc);
// 成功时返回终止的子进程ID，失败时返回-1
```

调用该函数后，相关信息保存在statloc指向的地址里。用宏WIFEXITED判断子进程是否正常终止，宏WEXITSTATUS获取子进程的返回值

wait.c

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

int main(int argc,char* argv[]){
    int status;
    pid_t pid = fork();

    if(pid==0){
        return 3;
    }else{
        printf("Child PID: %d \n",pid);
        pid = fork();
        if(pid==0){
            exit(7);
        }else{
            printf("Child PID: %d \n",pid);
            wait(&status);
            if(WIFEXITED(status))
                printf("Child send one: %d \n",WEXITSTATUS(status));

            wait(&status);
            if (WIFEXITED(status))
                printf("Child send one: %d \n", WEXITSTATUS(status));

            sleep(30);
        }
    }
}
```

运行结果

```shell
ubuntu     5053  0.0  0.0   4512   764 pts/5    S+   08:50   0:00 ./bin/wait

# 可知 父进程处于休眠状态时，子进程正常终止
```

上述代码，共产生两个子进程。父进程利用wait函数对子进程资源进行回收，因此在父进程休眠时，子进程依然可以被正常回收。

**注意：当没有子进程终止时，wait函数会阻塞，直到有子进程终止。**

### 2.2 利用waitpid函数

wait函数会引起阻塞，因此可以调用waitpid函数。

```c
#include <sys/wait.h>
#include <sys/types.h>

pid_t waitpid(pid_t pid, int *statloc, int options);
```

waitpid.c

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){
    int status;
    pid_t pid = fork();
    if(pid==0){
        sleep(15);
        return 24;
    }else{
        while(!waitpid(-1, &status,WNOHANG)){
            sleep(3);
            puts("sleep 3sec.");
        }

        if(WIFEXITED(status)){
            printf("Child send %d \n",WEXITSTATUS(status));
        }
    }
    return 0;
}
```

**运行结果**

```shell
$ gcc waitpid.c -o ./bin/waitpid
$ ./bin/waitpid
sleep 3sec.
sleep 3sec.
sleep 3sec.
sleep 3sec.
sleep 3sec.
Child send 24 

# 输出了5次sleep 3sec.，可知waitpid并未阻塞
```

```shell
$ ps au
ubuntu     8119  0.0  0.0   4512   808 pts/5    S+   09:03   0:00 ./bin/waitpid
ubuntu     8120  0.0  0.0   4380    72 pts/5    S+   09:03   0:00 ./bin/waitpid

# 可知 父进程和子进程都处于休眠状态，之后正常结束
```



## 3、信号处理——销毁僵尸进程最常用方法

wait函数和waitpid函数的处理还不够优雅，wait会阻塞父进程，waitpid也需要不断调用来监听子进程结束。接下来介绍信号处理，这是销毁僵尸进程最常用的方法。当子进程结束时，由操作系统向父进程发送一个信号，通知父进程销毁子进程，不管父进程在忙碌中，还是休眠状态。

alarm函数声明

```c
#include <unistd.h>

unsigned int alarm(unsigned int seconds);
// 返回0或以秒为单位的距SIGALRM信号发生所剩时间
// 若传递0，则之前对SIGALRM信号的预约取消
```



signal函数的声明

```c
#include <signal.h>

void (*signal(int signo, void (*func)(int)))(int);
// 为了在产生信时调用，返回之前注册的函数指针

//函数名 signal
//参数 int signo, void(* func)(int)
//返回类型 : 参数为int型，返回void型函数指针
```

实际开发中很少用signal函数编写程序，只是为了与旧版程序的兼容。为了更好的稳定性，应该使用sigaction函数进行信号处理。

sigaction函数声明

```c
#include <signal.h>

int sigaction(int signo, const struct sigaction *act, struct sigaction *oldact);
//成功时返回0，失败时返回-1

/*
	signo 与signal函数相同，传递信号信息
	act 信号处理函数
	oldact 获取之前注册的信号处理函数指针，若不需要则传递0
*/
```

声明并初始化sigaction结构体变量以调用上述函数

```c
struct sigaction{
	void (*sa_handler)(int);     //信号处理函数
  sigset_t sa_mask;						 
  int sa_flags;
}
```

当子进程结束时，操作系统向父进程发送信号，由父进程销毁子进程。

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

void read_childproc(int sig){
    int status;
    pid_t id = waitpid(-1, &status, WNOHANG); //WNOHANG为
    if(WIFEXITED(status)){
        printf("Remove oroc id: %d\n", id);
        printf("Child send: %d \n",WEXITSTATUS(status));
    }
}

int main(int argc, char* argv[]){
    pid_t pid;
    struct sigaction act;
    
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);
    pid = fork();
    if(pid==0){
        puts("Hi，I'm child process\n");
        sleep(10);
        return 12;
    }else{
        printf("Child proc id : %d\n",pid);
        pid = fork();
        if(pid==0){
            puts("Hi, I'm child process");
            sleep(10);
            exit(24);
        }else{
            printf("Child proc id : %d\n",pid);
            for(int i=0; i<5; i++){
                puts("wait...");
                sleep(5);
            }
        }
    }
    return 0;
}
```

输出

```shell
$ ./bin/remove_zombie 
Child proc id : 881
Child proc id : 883
wait...
Hi, I'm child process
Hi，I'm child process
wait...
wait...
Remove oroc id: 883
Child send: 24 
wait...
Remove oroc id: 881
Child send: 12 
wait...
```



