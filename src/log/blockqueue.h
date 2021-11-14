/*
******************************************
阻塞队列的实现，用于后面实现日志系统。
******************************************
*/
#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include<mutex>
#include<deque>
#include<condition_variable>
#include<sys/time.h>
#include<assert.h>

template<class T>
class BlockDeque
{
public:
    explicit BlockDeque(size_t MaxCapacity = 1000);

    ~BlockDeque();

    void clear();

    bool empty();

    bool full();

    void Close();

    size_t size();

    size_t capacity();

    T front();

    T back();

    void push_back(const T& item);

    void push_front(const T& item);

    bool pop(T& item);

    bool pop(T& item,int timeout);

    void flush();

private:
    std::deque<T> deq_;

    size_t capacity_;

    std::mutex mtx_;

    bool isClose_;

    std::condition_variable condConsumer_;

    std::condition_variable condProducer_;

};

#endif