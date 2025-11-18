#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include "global.h"

// explicit 关键字 https://blog.csdn.net/weixin_45031801/article/details/137796214

template<typename T>
class BlockQueue {
public:
    // 初始化阻塞队列(explicit 关键字禁止出现隐式的类型转换，禁止使用拷贝构造函数)
    explicit BlockQueue(size_t maxsize = 1000);
    // 释放资源
    ~BlockQueue();
    // 阻塞队列是否是空的
    bool empty();
    // 阻塞队列是否是满的
    bool full();
    // 添加任务在队尾
    void push_back(const T& item);
    // 添加任务在队头
    void push_front(const T& item);
    // 取出任务并且放在 item 中
    bool pop(T& item);  
    // 取出任务 但是如果等待了 timeout 时间之后，没有取出任务就直接返回false
    bool pop(T& item, int timeout);  // 等待时间
    // 清空任务队列
    void clear();
    // 返回阻塞队列的队头元素
    T front();
    // 返回阻塞队列的队尾元素
    T back();
    // 返回阻塞队列的容量
    size_t capacity();
    // 返回阻塞队列的大小
    size_t size();
    // 唤醒消费者
    void flush();
    // 关闭线程池
    void Close();

private:
    // 存放任务的真正队列
    deque<T> deq_;                   
    // 是否打开阻塞队列                     
    bool isClose_;        
    // 任务队列的最大容量       
    size_t capacity_; 
    // 互斥锁
    mutex mtx_;
    // 消费者条件变量    
    condition_variable condConsumer_;   
    // 生产者条件变量
    condition_variable condProducer_;   
};

#endif

// 内联函数：https://blog.csdn.net/qq_35902025/article/details/127912415

template<typename T>
inline BlockQueue<T>::BlockQueue(size_t maxsize) : capacity_(maxsize)
{
    assert(maxsize > 0);
    isClose_ = false;
}

template<typename T>
inline BlockQueue<T>::~BlockQueue()
{
	// 在析构之前，需要先将线程池关闭
    Close();
}

template<typename T>
inline bool BlockQueue<T>::empty()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.empty();
}

template<typename T>
inline bool BlockQueue<T>::full()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

// 在以下四个函数中，大家可以会疑惑，为什么不写 if判断，而要写while循环判断，
// 这其实是相当于“加了双层保险”，大家想，如果是有多个 生产者/线程消费者线程 被唤醒的话，
// 如果是if的话，就会导致多个线程同时去执行 push/pop 操作（也就相当于多个线程同时
// 去操作共享资源），那么这是不符合我们的想法的:我们只希望在某一个时刻，只有一个线程去对共享资源操作。 

template<typename T>
inline void BlockQueue<T>::push_back(const T& item)
{
	// 加锁
    unique_lock<mutex> locker(mtx_);
    // 阻塞队列没有关闭 并且 任务队列容量达到最大值了
    while (!isClose_ && deq_.size() >= capacity_)
    {
    	// 那么就将生产者线程就阻塞，并且会释放锁资源。让消费者从任务队列取数据
        condProducer_.wait(locker);
    }
    if(isClose_)
    {
        // 如果是因为队列关闭而退出循环，那么就直接返回
        return;
    }
    // 将任务放入队列
    deq_.push_back(item);
    // 唤醒一个消费者线程
    condConsumer_.notify_one();
}

template<typename T>
inline void BlockQueue<T>::push_front(const T& item)
{
    unique_lock<mutex> locker(mtx_);
    while (!isClose_ && deq_.size() >= capacity_)
    {
        condProducer_.wait(locker);
    }
    if(isClose_)
    {
        return;
    }
    deq_.push_front(item);
    condConsumer_.notify_one();
}

template<typename T>
inline bool BlockQueue<T>::pop(T& item)
{
	//加锁
    unique_lock<mutex> locker(mtx_);
    // 如果队列为空
    while (!isClose_ && deq_.empty())
    {
    	// 将消费者线程阻塞，让生产者去投放任务
        condProducer_.wait(locker);
    }
    if(isClose_)
    {
        // 如果是因为队列关闭而退出循环，那么就直接返回
        return;
    }
    // 获取任务
    item = deq_.front();
    deq_.pop_front();
    // 唤醒一个生产者线程
    condProducer_.notify_one();
    return true;
}

template<typename T>
inline bool BlockQueue<T>::pop(T& item, int timeout)
{
	// 加锁
    unique_lock<std::mutex> locker(mtx_);
    // 任务队列为空
    while (!isClose_ && .empty()) 
    {
    	// 如果等待了  std::chrono::seconds(timeout) 时间之后，这个线程还没有被唤醒，那么就会直接返回false
        if (condConsumer_.wait_for(locker, std::chrono::seconds(timeout))
            == std::cv_status::timeout) 
        {
            return false;
        }
    }
    if(isClose_)
    {
        // 如果是因为队列关闭而退出循环，那么就直接返回
        return false;
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template<typename T>
inline void BlockQueue<T>::clear()
{
    lock_guard<mutex> locker(mtx_);
    deq_.clear();
}

template<typename T>
inline T BlockQueue<T>::front()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.front();
}

template<typename T>
inline T BlockQueue<T>::back()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.back();
}

template<typename T>
inline size_t BlockQueue<T>::capacity()
{
    lock_guard<mutex> locker(mtx_);
    return capacity_;
}

template<typename T>
inline size_t BlockQueue<T>::size()
{
    lock_guard<mutex> locker(mtx_);
    return deq_.size();
}

template<typename T>
inline void BlockQueue<T>::flush()
{
    condConsumer_.notify_one();
}

template<typename T>
inline void BlockQueue<T>::Close()
{
	// 清空任务队列
    clear();
    // 将标志状态置为 true
    isClose_ = true;
    // 唤醒阻塞的所有生产者 / 消费者 线程(这里的阻塞是指：调用了 ConditionVariable::wait() 的线程)
    condProducer_.notify_all();
    condConsumer_.notify_all();
}

