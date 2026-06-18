#include <stdio.h>   /* 标准输入输出库，用于 printf/scanf */
#include <stdlib.h>  /* 标准库，用于 malloc 动态内存分配 */

#define N 5              /* 任务数量 */
#define TIME_SLICE 2     /* 时间片轮转(RR)算法的时间片大小 */

/* 进程/任务控制块（PCB）结构体 */
typedef struct {
    int id;          /* 任务ID（1~N） */
    int start;       /* 到达时间 */
    int exec;        /* 所需总执行时间 */
    int remain;      /* 剩余执行时间（动态减少） */
    int priority;    /* 优先级（数值越大优先级越高） */
    int state;       /* 状态：0=未到达/就绪，1=执行中，2=已完成 */
    int *chart;      /* 动态分配的数组，记录该进程在哪些时间片被分配了 CPU */
} Task;

Task tasks[N];              /* 全局任务数组 */
int timeline[100];          /* 记录全局时间轴上每个时刻运行的进程 ID */
int time_len = 0;           /* 总调度时长 */

/* 初始化所有任务的数据 */
void init() {
    int i;
    /* 预定义的到达时间 */
    int start_times[N] = {0, 2, 4, 6, 8};
    /* 预定义的执行时间 */
    int exec_times[N] = {3, 6, 4, 5, 2};
    /* 预定义的优先级 */
    int prios[N] = {2, 5, 2, 4, 3};
    for(i = 0; i < N; i++) {
        tasks[i].id = i + 1;                  /* 任务编号从1开始 */
        tasks[i].start = start_times[i];      /* 设置到达时间 */
        tasks[i].exec = exec_times[i];        /* 设置总执行时间 */
        tasks[i].remain = exec_times[i];      /* 剩余时间初始等于总执行时间 */
        tasks[i].priority = prios[i];         /* 设置优先级 */
        tasks[i].state = 0;                   /* 初始状态：未到达 */
        tasks[i].chart = (int*)malloc(100 * sizeof(int)); /* 为甘特图分配内存 */
        for(int j = 0; j < 100; j++) tasks[i].chart[j] = 0; /* 初始化为0 */
    }
    time_len = 0;   /* 重置调度时长 */
}

/* 打印所有任务的基本信息（ID、到达时间、执行时间、优先级） */
void print_task_info() {
    printf("task id\tstart\texecute\n");
    for(int i = 0; i < N; i++) {
        printf("task %d:\t%d\t%d\n", tasks[i].id, tasks[i].start, tasks[i].exec);
    }
    printf("Task priorities: ");
    for(int i = 0; i < N; i++) {
        printf("%d ", tasks[i].priority);
    }
    printf("\n");
}

/* 打印甘特图（Gantt Chart），直观显示每个任务在时间轴上的CPU占用情况 */
void print_chart() {
    int i, j;
    printf("time  : 0");
    for(j=0;j<17;j++) printf(" ");
    printf("20\n");
    for(i = 0; i < N; i++) {
        printf("task %d: ", tasks[i].id);
        for(j = 0; j < 20; j++) {
            if(j < time_len && tasks[i].chart[j]) {
                printf("#");     /* 该时刻此任务占用CPU，打印# */
            } else {
                printf(" ");     /* 该时刻此任务未占用CPU，留空 */
            }
        }
        printf("\n");
    }
}

void fcfs() {
    /* FCFS（先来先服务）调度算法 */
    int i, t = 0, done = 0;    /* t: 当前时间, done: 已完成任务数 */
    init();
    print_task_info();
    while(done < N) {           /* 循环直到所有任务都完成 */
        int sel = -1;
        /* 按顺序查找第一个已到达且未开始的任务 */
        for(i = 0; i < N; i++) {
            if(tasks[i].state == 0 && tasks[i].start <= t) {
                sel = i;
                break;          /* FCFS：找到第一个就绪任务即选中，非抢占 */
            }
        }
        if(sel == -1) {         /* 没有就绪任务，CPU空闲 */
            t++;
            continue;
        }
        tasks[sel].state = 1;   /* 标记为执行中 */
        /* 非抢占式：一次性执行完该任务的全部剩余时间 */
        while(tasks[sel].remain > 0) {
            tasks[sel].chart[t] = 1;   /* 记录该任务在时间t占用CPU */
            timeline[t] = tasks[sel].id; /* 记录全局时间轴 */
            tasks[sel].remain--;
            t++;
        }
        tasks[sel].state = 2;   /* 标记为已完成 */
        done++;
    }
    time_len = t;
    print_chart();
}

void rr() {
    /* RR（时间片轮转）调度算法 */
    int t = 0, done = 0, i;
    init();
    print_task_info();
    while(done < N) {           /* 循环直到所有任务都完成 */
        int found = 0;          /* 标记当前轮是否找到就绪任务 */
        for(i = 0; i < N; i++) {
            /* 将新到达的任务标记为就绪 state 0 -> 1 */
            if(tasks[i].state == 0 && tasks[i].start <= t) {
                tasks[i].state = 1;
            }
            /* 如果有就绪任务，执行一个时间片 */
            if(tasks[i].state == 1 && tasks[i].remain > 0) {
                int run = 0;
                /* 每次最多执行 TIME_SLICE 个单位时间 */
                while(run < TIME_SLICE && tasks[i].remain > 0) {
                    tasks[i].chart[t] = 1;   /* 记录该任务在时间t占用CPU */
                    timeline[t] = tasks[i].id; /* 记录全局时间轴 */
                    tasks[i].remain--;
                    t++;
                    run++;
                }
                if(tasks[i].remain == 0) {   /* 该任务执行完毕 */
                    tasks[i].state = 2;       /* 标记为已完成 */
                    done++;
                }
                found = 1;                   /* 本轮有任务被执行 */
            }
        }
        if(!found) t++;         /* 没有就绪任务，CPU空闲 */
    }
    time_len = t;
    print_chart();
}

/* 优先级调度算法（Priority-based Scheduling，抢占式） */
void priority_sched() {
    int t = 0, done = 0, i;    /* t: 当前时间, done: 已完成任务数 */
    init();
    print_task_info();
    while(done < N) {           /* 循环直到所有任务都完成 */
        /* 将新到达的任务标记为就绪 state 0 -> 1 */
        for(i = 0; i < N; i++) {
            if(tasks[i].state == 0 && tasks[i].start <= t) {
                tasks[i].state = 1;
            }
        }

        /* 在所有就绪任务中选择优先级最高的（抢占式） */
        int sel = -1, max_prio = -1;
        for(i = 0; i < N; i++) {
            if(tasks[i].state == 1 && tasks[i].remain > 0 && tasks[i].priority > max_prio) {
                max_prio = tasks[i].priority;
                sel = i;
            }
        }

        if(sel == -1) {
            /* 没有就绪任务，CPU空闲 */
            t++;
            continue;
        }

        /* 抢占式：每时间单位只执行1个单位，下一时刻重新选择最高优先级任务 */
        tasks[sel].chart[t] = 1;   /* 记录该任务在时间t占用CPU */
        timeline[t] = tasks[sel].id; /* 记录全局时间轴 */
        tasks[sel].remain--;
        t++;

        if(tasks[sel].remain == 0) {
            tasks[sel].state = 2;   /* 标记为已完成 */
            done++;
        }
    }
    time_len = t;
    print_chart();
}

/* 主函数：菜单循环，供用户选择调度算法 */
int main() {
    char op;                     /* 用户输入的选项 */
    while(1) {                   /* 无限循环，直到用户输入q退出 */
        printf("please choose the schedule measure:\n");
        printf("p : priority-based scheduling\n");   /* 优先级调度 */
        printf("f : FCFS scheduling\n");             /* 先来先服务 */
        printf("r : round-robin scheduling\n");       /* 时间片轮转 */
        printf("q : exit\n");                        /* 退出程序 */
        printf("choice = ");
        scanf(" %c", &op);                           /* 读取用户输入 */
        if(op == 'q') break;                         /* 输入q则退出 */
        switch(op) {
            case 'p': priority_sched(); break;       /* 执行优先级调度 */
            case 'f': fcfs(); break;                 /* 执行FCFS调度 */
            case 'r': rr(); break;                   /* 执行时间片轮转 */
            default: printf("Invalid choice\n");     /* 非法输入 */
        }
    }
    return 0;
}
