#ifndef LOGGER_H
#define LOGGER_H

#include "global.h"
#include "SingleTon.h"
#include "BlockQueue.h"

class Buffer;

class Logger : public SingleTon<Logger>
{
public:
    // 将数据写入到文件
    void write(int level, const char* format, ...);
    // 唤醒消费者线程 开始写日志
    void flush();
    // 获取当前日志系统的最低等级
    int GetLevel();
    // 设置当前日志系统的最低等级
    void SetLevel(int level);
    // 是否打开日志系统
    bool IsOpen() { return isOpen_; }

private:
    Logger();
    // 传入日志等级 获取 对应的日志信息 并且将结果写入到 缓冲区 buff_ 当中 
    void AppendLogLevelTitle_(int level);
    virtual ~Logger();
    // 异步写日志方法
    void AsyncWrite_();

private:
    int lineCount_;             //日志行数记录
    int toDay_;                 //按当天日期区分文件

    bool isOpen_;

    std::string buff_;       // 输出的内容，缓冲区
    int level_;         // 日志等级
    bool isAsync_;      // 是否开启异步日志

    FILE* fp_;                                          //打开log的文件指针
    std::unique_ptr<BlockQueue<std::string>> deque_;    //阻塞队列
    std::unique_ptr<std::thread> writeThread_;          //写线程的指针
};

#endif

#define LOG_BASE(level, format, ...) \
    do {\
        Log* log = Log::Instance();\
        if (log->IsOpen() && log->GetLevel() <= level) {\
            log->write(level, format, ##__VA_ARGS__); \
            log->flush();\
        }\
    } while(0);

// 四个宏定义，主要用于不同类型的日志输出，也是外部使用日志的接口
// ...表示可变参数，__VA_ARGS__就是将...的值复制到这里
// 前面加上##的作用是：当可变参数的个数为0时，这里的##可以把把前面多余的","去掉,否则会编译出错。
#define LOG_DEBUG(format, ...) do {LOG_BASE(0, format, ##__VA_ARGS__)} while(0);    
#define LOG_INFO(format, ...) do {LOG_BASE(1, format, ##__VA_ARGS__)} while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2, format, ##__VA_ARGS__)} while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3, format, ##__VA_ARGS__)} while(0);