#include <stdio.h>   /* 标准输入输出库，用于 printf/scanf */
#include <stdlib.h>  /* 标准库 */

#define MAX_PAGES  100   /* 页面引用序列最大长度 */
#define MAX_FRAMES 10    /* 物理内存帧最大数量 */

int n;                    /* 页面范围 / 进程大小 */
int f;                    /* 物理帧数（即内存中最多存放的页面数） */
int page_count;           /* 页面引用串的实际长度 */
int pages[MAX_PAGES];     /* 页面引用序列 */
int frames[MAX_FRAMES];   /* 当前内存中的帧（存放的页面号，-1表示空） */
int ref_bits[MAX_FRAMES]; /* CLOCK 算法用的引用位（访问位） */

/* 函数声明 */
void print_frames(int step, int occupied);
void reset_state();
void run_opt();
void run_fifo();
void run_lru();
void run_clock();

/* 主函数：获取输入并展示菜单 */
int main() {
    char choice;

    /* 获取页面范围、物理帧数、引用序列长度和具体序列 */
    printf("Enter number of pages in program (e.g. 10): ");
    scanf("%d", &n);
    printf("Enter number of frames in physical memory (e.g. 3): ");
    scanf("%d", &f);

    printf("Enter the total count of page references (e.g. 30): ");
    scanf("%d", &page_count);

    printf("Enter the page reference sequence separated by space:\n");
    for (int i = 0; i < page_count; i++) {
        scanf("%d", &pages[i]);
    }

    /* 打印用户输入的页面引用序列 */
    printf("\nPage reference : |");
    for (int i = 0; i < page_count; i++) printf(" %d |", pages[i]);
    printf("\n\n");

    /* 算法选择菜单循环 */
    while(1) {
        printf("Choice: Please select an action\n");
        printf("o - OPT\nf - FIFO\nr - LRU\nc - CLOCK\nq - quit\n");
        scanf(" %c", &choice);
        if(choice == 'q') break;     /* 输入 q 退出 */
        reset_state();               /* 每次运行前重置帧状态 */
        switch(choice) {
            case 'o': run_opt(); break;     /* 最佳置换算法 */
            case 'f': run_fifo(); break;    /* 先进先出算法 */
            case 'r': run_lru(); break;     /* 最近最少使用算法 */
            case 'c': run_clock(); break;   /* 时钟置换算法 */
            default: printf("Invalid!\n");
        }
        printf("\n");
    }
    return 0;
}

/* 重置状态：清空帧和引用位，为下一次算法运行做准备 */
void reset_state() {
    for(int i = 0; i < MAX_FRAMES; i++) {
        frames[i] = -1;    /* -1 表示帧为空 */
        ref_bits[i] = 0;   /* 引用位清零 */
    }
}

/* 打印当前步骤的帧状态（仅打印已占用的帧，未占用的不显示） */
void print_frames(int step, int occupied) {
    printf("no. %2d step: |", step);
    for(int i = 0; i < occupied; i++) {
        printf(" %d |", frames[i]);
    }
    printf("\n");
}

/* =================== OPT（最佳置换算法） =================== */
/* 核心思想：淘汰未来最长时间内不会再被访问的页面 */
void run_opt() {
    int page_faults = 0;   /* 缺页次数统计 */
    int occupied = 0;      /* 当前已占用的帧数 */

    for(int i = 0; i < page_count; i++) {
        int cur = pages[i];   /* 当前要访问的页面 */
        int found = 0;        /* 是否在内存中找到 */

        /* 查找当前页面是否已在内存中 */
        for(int j = 0; j < f; j++) {
            if(frames[j] == cur) {
                found = 1;   /* 命中，无需换页 */
                break;
            }
        }

        if(!found) {
            if(occupied < f) {
                /* 帧未满：直接装入，不计入缺页次数 */
                frames[occupied++] = cur;
            } else {
                /* 帧已满：需要置换，计入缺页次数 */
                page_faults++;
                int replace_idx = 0;   /* 被替换的帧下标 */
                int farthest = -1;     /* 最远未来使用位置 */

                /* 遍历每个帧中的页面，找出未来最远被使用 / 不再使用的页面 */
                for(int j = 0; j < f; j++) {
                    int k;
                    /* 从当前位置 i+1 向后查找该页面下一次出现的位置 */
                    for(k = i + 1; k < page_count; k++) {
                        if(frames[j] == pages[k]) break;
                    }
                    if(k == page_count) {
                        /* 该页面以后不再被使用，优先替换它 */
                        replace_idx = j;
                        break;
                    }
                    /* 记录出现位置最远的页面 */
                    if(k > farthest) {
                        farthest = k;
                        replace_idx = j;
                    }
                }
                frames[replace_idx] = cur;   /* 执行替换 */
            }
        }
        print_frames(i, occupied);   /* 打印当前步骤的帧状态 */
    }
    printf("缺页次数： %d次\n", page_faults);
}

/* =================== FIFO（先进先出置换算法） =================== */
/* 核心思想：淘汰最先进入内存的页面（队列头），新页面加入队尾 */
void run_fifo() {
    int page_faults = 0;   /* 缺页次数统计 */
    int occupied = 0;      /* 当前已占用的帧数 */
    int hand = 0;          /* 队列指针：指向最早进入的页面所在帧 */

    for(int i = 0; i < page_count; i++) {
        int cur = pages[i];   /* 当前要访问的页面 */
        int found = 0;        /* 是否在内存中找到 */

        /* 查找当前页面是否已在内存中 */
        for(int j = 0; j < f; j++) {
            if(frames[j] == cur) {
                found = 1;   /* 命中，无需换页 */
                break;
            }
        }

        if(!found) {
            if(occupied < f) {
                /* 帧未满：直接装入，不计入缺页次数 */
                frames[occupied++] = cur;
            } else {
                /* 帧已满：淘汰最早进入的页面（hand 指向的位置） */
                page_faults++;
                frames[hand] = cur;
                hand = (hand + 1) % f;   /* hand 循环移动，实现先进先出 */
            }
        }
        print_frames(i, occupied);   /* 打印当前步骤的帧状态 */
    }
    printf("缺页次数： %d次\n", page_faults);
}

/* =================== LRU（最近最少使用置换算法） =================== */
/* 核心思想：淘汰最长时间没有被访问的页面（最久未使用） */
void run_lru() {
    int page_faults = 0;           /* 缺页次数统计 */
    int occupied = 0;              /* 当前已占用的帧数 */
    int last_used[MAX_FRAMES] = {0}; /* 记录每个帧中页面最近一次被访问的时间（即引用序列下标） */

    for(int i = 0; i < page_count; i++) {
        int cur = pages[i];   /* 当前要访问的页面 */
        int found = 0;        /* 是否在内存中找到 */

        /* 查找当前页面是否已在内存中 */
        for(int j = 0; j < f; j++) {
            if(frames[j] == cur) {
                found = 1;
                last_used[j] = i;   /* 命中，更新该页面的最近使用时间 */
                break;
            }
        }

        if(!found) {
            if(occupied < f) {
                /* 帧未满：直接装入，记录使用时间，不计入缺页次数 */
                frames[occupied] = cur;
                last_used[occupied] = i;
                occupied++;
            } else {
                /* 帧已满：找出最久未使用的页面进行淘汰 */
                page_faults++;
                int min_time = last_used[0];
                int replace_idx = 0;
                /* 遍历所有帧，找到 last_used 最小的页面（即最久未被访问的） */
                for(int j = 1; j < f; j++) {
                    if(last_used[j] < min_time) {
                        min_time = last_used[j];
                        replace_idx = j;
                    }
                }
                frames[replace_idx] = cur;    /* 执行替换 */
                last_used[replace_idx] = i;   /* 更新新页面的使用时间 */
            }
        }
        print_frames(i, occupied);   /* 打印当前步骤的帧状态 */
    }
    printf("缺页次数： %d次\n", page_faults);
}

/* =================== CLOCK（时钟置换算法） =================== */
/* 核心思想：利用引用位循环扫描，淘汰引用位为0的页面，将引用位为1的置0后继续扫描 */
void run_clock() {
    int page_faults = 0;   /* 缺页次数统计 */
    int occupied = 0;      /* 当前已占用的帧数 */
    int hand = 0;          /* 时钟指针，循环扫描各帧 */

    for(int i = 0; i < page_count; i++) {
        int cur = pages[i];   /* 当前要访问的页面 */
        int found = 0;        /* 是否在内存中找到 */

        /* 查找当前页面是否已在内存中 */
        for(int j = 0; j < f; j++) {
            if(frames[j] == cur) {
                found = 1;
                ref_bits[j] = 1;   /* 命中，将引用位置1 */
                break;
            }
        }

        if(!found) {
            if(occupied < f) {
                /* 帧未满：直接装入，引用位置1，不计入缺页次数 */
                frames[occupied] = cur;
                ref_bits[occupied] = 1;
                occupied++;
            } else {
                /* 帧已满：执行时钟扫描置换 */
                page_faults++;
                while(1) {
                    if(ref_bits[hand] == 0) {
                        /* 引用位为0：淘汰该页面，装入新页面并置引用位为1 */
                        frames[hand] = cur;
                        ref_bits[hand] = 1;
                        hand = (hand + 1) % f;   /* 指针后移 */
                        break;
                    } else {
                        /* 引用位为1：给它第二次机会，置0后继续扫描 */
                        ref_bits[hand] = 0;
                        hand = (hand + 1) % f;   /* 指针后移 */
                    }
                }
            }
        }
        print_frames(i, occupied);   /* 打印当前步骤的帧状态 */
    }
    printf("缺页次数： %d次\n", page_faults);
}