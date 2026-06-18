#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

/* 定义子进程接收到信号时的处理函数 */
void stop_child1() {
    printf("child process 1 is killed by parent!\n");
    exit(0);
}

void stop_child2() {
    printf("child process 2 is killed by parent!\n");
    exit(0);
}

/* 定义父进程接收到 SIGQUIT 时的处理函数 */
int wait_flag = 1;
void stop_parent() {
    wait_flag = 0;
}

int main() {
    int pid1, pid2;

    /* 第一阶段：创建第一个子进程 P1 */
    while ((pid1 = fork()) == -1);//创建进程，直到创建失败，即保证了一定能创建进程1
    if (pid1 > 0) {
        /* 父进程继续执行，准备创建第二个子进程 P2 */
        while ((pid2 = fork()) == -1);
        if (pid2 > 0) {
            /* 父进程代码 */
            /* 设置父进程捕获 SIGQUIT (Ctrl+\) 信号 */
            signal(SIGQUIT, stop_parent);
            //按下 Ctrl+\，操作系统会向前台进程组发送 SIGQUIT 信号
            /* 等待用户按下 Ctrl+\ */
            while (wait_flag);

            /* 向两个子进程发送自定义信号 16 和 17 */
            kill(pid1, 16);
            kill(pid2, 17);

            /* 等待两个子进程结束 */
            wait(NULL);
            wait(NULL);
            printf("parent process is killed!\n");
            exit(0);
        } else {//防止ctrl\将子进程终止
            /* 子进程 P2 代码 */
            /* 忽略 SIGQUIT 信号，只响应 17 号信号 */
            signal(SIGQUIT, SIG_IGN);//无视终端的终止操作
            signal(17, stop_child2);
            printf("task2 start\n");
            while (1) {
                pause();
            }
        }
    } else {
        /* 子进程 P1 代码 */
        /* 忽略 SIGQUIT 信号，只响应 16 号信号 */
        signal(SIGQUIT, SIG_IGN);//无视终端的终止操作
        signal(16, stop_child1);
        printf("task1 start\n");
        while (1) {
            pause();
        }
    }

    return 0;
}
