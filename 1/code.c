#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

// 全局变量，用于 func_C 接收参数
int Internal;
int duration;

void func_A()
{
    // 观察 CPU 及内核信息
    FILE *fp;
    char buffer[1024];
    char cpu_model[256];
    char kernel_version[256];

    // 1. 获取 CPU 型号，只读方式打开
    fp = fopen("/proc/cpuinfo", "r");
    if (fp)
    {       //指针循环
        while (fgets(buffer, sizeof(buffer), fp))
        {
            if (strstr(buffer, "model name"))
            {
                // 解析冒号后面的内容
                sscanf(buffer, "model name : %[^\n]", cpu_model);
                printf("CPU类型及型号: %s\n", cpu_model);
                break; // 假设只需要第一行信息
            }
        }
        fclose(fp);
    }

    // 2. 获取内核版本 (使用 uname 命令获取，或者直接读 /proc/version)
    // 直接解析 /proc/version
    fp = fopen("/proc/version", "r");
    if (fp)
    {
        fgets(buffer, sizeof(buffer), fp);
        // 简单提取版本字符串部分
        sscanf(buffer, "Linux version %s", kernel_version);

        printf("内核版本: Linux version %s\n", kernel_version);

        // 由于 /proc/version 包含 gcc version 等信息，直接打印 buffer 可能更符合
        // printf("内核版本: %s", buffer);

        fclose(fp);
    }
}

void func_B()
{
    // 系统最近一次启动以来经历的时间、CPU 三状态时间花费、上下文转换次数和创建进程数
    FILE *fp;
    char buffer[1024];
    double uptime_seconds;
    time_t current_time, boot_time;
    long uptime_long;
    long user, nice, system, idle;
    long ctxt, processes;
    struct tm *timeinfo;
    char time_str[64];

    // 1. 获取系统运行时间 (Uptime)
    fp = fopen("/proc/uptime", "r");
    if (fp)
    {
        fscanf(fp, "%lf", &uptime_seconds);
        fclose(fp);
    }
    else
    {
        printf("无法打开 /proc/uptime\n");
        return;
    }

    // 计算启动时间
    current_time = time(NULL);
    boot_time = current_time - (time_t)uptime_seconds;
    timeinfo = localtime(&boot_time);
    strftime(time_str, sizeof(time_str), "%Y/%m/%d %H:%M:%S", timeinfo);
    printf("系统最后启动时间为:%s\n", time_str);

    // 格式化运行时长
    long hours = (long)uptime_seconds / 3600;
    long mins = ((long)uptime_seconds % 3600) / 60;
    long secs = (long)uptime_seconds % 60;
    printf("系统最后一次启动以来的时间: %02ld:%02ld:%02ld\n", hours, mins, secs);

    // 2. 获取 CPU 状态和进程/上下文信息
    fp = fopen("/proc/stat", "r");
    if (fp)
    {
        while (fgets(buffer, sizeof(buffer), fp))
        {
            // 解析 CPU 时间
            if (strncmp(buffer, "cpu ", 4) == 0)
            {   //用户态，用户态+优先级降低，内核态，空闲态
                // user, nice, system, idle 是 jiffies (时钟滴答数)
                sscanf(buffer, "cpu %ld %ld %ld %ld", &user, &nice, &system, &idle);
                // 转换为秒 (假设 HZ=100)
                printf("用户态时间:%ld(%.2f秒) 系统态时间:%ld(%.2f秒) 空闲态时间:%ld(%.2f秒)\n",
                       user, user / 100.0,
                       system, system / 100.0,
                       idle, idle / 100.0);
            }
            // 解析上下文切换次数
            else if (strncmp(buffer, "ctxt", 4) == 0)
            {
                sscanf(buffer, "ctxt %ld", &ctxt);
                printf("上下文转换的次数:%ld\n", ctxt);
            }
            // 解析创建进程数
            else if (strncmp(buffer, "processes", 9) == 0)
            {
                sscanf(buffer, "processes %ld", &processes);
                printf("从系统启动开始创建的进程数:%ld\n", processes);
            }
        }
        fclose(fp);
    }
}

void func_C()
{
    // 系统内存使用情况、平均负载
    FILE *fp;
    char buffer[1024];
    long mem_total, mem_free, buffers, cached;
    double load1, load5, load15;

    // 1. 内存信息
    fp = fopen("/proc/meminfo", "r");
    if (fp)
    {
        while (fgets(buffer, sizeof(buffer), fp))
        {   //比较前九个字符
            if (strncmp(buffer, "MemTotal:", 9) == 0)
            {
                sscanf(buffer, "MemTotal: %ld kB", &mem_total);
                // 字节，这里乘以 1024
                printf("计算机配置的内存数量:%ld\n", mem_total * 1024);
            }
            else if (strncmp(buffer, "MemFree:", 8) == 0)
            {
                sscanf(buffer, "MemFree: %ld kB", &mem_free);
                printf("当前可用的内存数量:%ld\n", mem_free * 1024);
            }
        }
        fclose(fp);
    }

    // 2. 平均负载
    fp = fopen("/proc/loadavg", "r");
    if (fp)
    {
        fscanf(fp, "%lf %lf %lf", &load1, &load5, &load15);
        printf("平均负载列表至上一分钟的平均数:\n");
        printf("%.6f\n", load1);
        printf("%.6f\n", load5);
        printf("%.6f\n", load15);
        fclose(fp);
    }
}

int main(int argc, char *argv[])
{
    // printf("argc=%d\n", argc);
    // for (int i = 0; i < argc; i++) {
    //     printf("argv[%d]=%s\n", i, argv[i]);
    // }
    char c1, c2;

    if (argc == 1) // 观察 cpu 及内核信息
    {
        func_A();
        return 0;
    }

    if (argc > 1)
    {
        sscanf(argv[1], "%c%c", &c1, &c2);
        if (c1 != '-')
            return 0;

        if (c2 == 's') // 系统最近一次启动以来经历的时间和所创建的进程数...
        {
            func_B();
            return 0;
        }

        if (c2 == 'l') // 内存的使用情况
        {
            if (argc < 4)
                return 0;
            // 这里的 Internal 和 duration 在截图中似乎没有直接打印，
            // 但根据框架逻辑，它们用于接收参数。
            // 如果实验要求根据参数监控一段时间，需要在这里加循环，
            // 但根据截图，func_C 只打印了一次静态信息。
            Internal = atoi(argv[2]);//ASCII to Integer
            duration = atoi(argv[3]);//将字符串转换成整数
            func_C();
            return 0;
        }
    }

    return 0;
}