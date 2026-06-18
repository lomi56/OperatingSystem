#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// --- 宏定义 ---
#define INIT_NUM    5   // 仓库原有产品数   
#define TOTAL_NUM   10  // 仓库容量
#define RUN_TIME    1  // 程序运行时长

// --- 信号量声明 ---
sem_t p_sem;  // 空闲货架信号量 (Empty)
sem_t c_sem;  // 货物库存信号量 (Full)
sem_t sh_sem; // 互斥信号量 (Mutex)

// --- 共享变量 ---
int count = INIT_NUM;          // 当前仓库产品数量
volatile int running = 1;      // 程序运行标志

// --- 线程函数声明 ---
void *productor(void *arg);
void *consumer(void *arg);

int main() {
    pthread_t t1, t2;

    // 初始化信号量
    // p_sem 初始值为 TOTAL_NUM - INIT_NUM (空闲位置)
    sem_init(&p_sem, 0, TOTAL_NUM - INIT_NUM);

    // c_sem 初始值为 INIT_NUM (已有产品)
    sem_init(&c_sem, 0, INIT_NUM);

    // sh_sem 初始值为 1 (互斥锁)
    sem_init(&sh_sem, 0, 1);

    // 创建线程
    pthread_create(&t1, NULL, productor, NULL);
    pthread_create(&t2, NULL, consumer, NULL);

    // 等待一段时间后结束程序
    sleep(RUN_TIME);

    // 设置退出标志，通知线程停止
    running = 0;

    // 唤醒可能阻塞在信号量上的线程 (post 让它们有机会退出)
    sem_post(&p_sem);
    sem_post(&c_sem);

    // 等待线程结束
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // 销毁信号量并退出
    sem_destroy(&p_sem);
    sem_destroy(&c_sem);
    sem_destroy(&sh_sem);

    return 0;
}

// 生产者线程
void *productor(void *arg) {
    while(running) {

        // 2. P操作：申请空闲货架
        sem_wait(&p_sem);
        if (!running) { sem_post(&p_sem); break; }  // 检查退出标志

        // 3. P操作：申请仓库操作权限 (互斥)
        sem_wait(&sh_sem);

        // 4. 临界区：放入产品
        count++;
        printf("添加一个产品到仓库中, 仓库现有产品：%d个。\n", count);

        // 5. V操作：释放仓库操作权限
        sem_post(&sh_sem);

        // 6. V操作：增加库存信号量，通知消费者
        sem_post(&c_sem);
    }
    return NULL;
}

// 消费者线程
void *consumer(void *arg) {
    while(running) {
        // 1. P操作：申请库存 (如果没有产品则阻塞)
        sem_wait(&c_sem);
        if (!running) { sem_post(&c_sem); break; }  // 检查退出标志

        // 2. P操作：申请仓库操作权限 (互斥)
        sem_wait(&sh_sem);

        // 3. 临界区：取出产品
        count--;
        printf("从仓库中取出一个产品, 仓库现有产品：%d个。\n", count);

        // 4. V操作：释放仓库操作权限
        sem_post(&sh_sem);

        // 5. V操作：释放空闲货架，通知生产者
        sem_post(&p_sem);

    }
    return NULL;
}