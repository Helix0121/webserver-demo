#include "blockqueue.h"

template<class T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity) : capacity_(MaxCapacity)
{
    assert(MaxCapacity > 0);
    isClose_ = false;
}

template<class T>
BlockDeque<T>::~BlockDeque()
{
    Close();
}

template<class T>
void BlockDeque<T>::Close()
{
    {
        std::lock_guard<std::mutex> locker(mtx_);//简化对mutex的上锁和解锁操作，属于是mutex的封装
        //在构造的时候锁定一次并在销毁的时候解锁。
        deq_.clear();
        isClose_=true;
    }
    condProducer_.notify_all();
    condConsumer_.notify_all();
}

template<class T>
void BlockDeque<T>::flush()
{
    condConsumer_.notify_all();
}

template<class T>
void BlockDeque<T>::clear()
{
    std::lock_guard<std::mutex> locker(mtx_);
    deq_.clear();
}

template<class T>
T BlockDeque<T>::front()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}

template<class T>
T BlockDeque<T>::back()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}

template<class T>
size_t BlockDeque<T>::size()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}

template<class T>
size_t BlockDeque<T>::capacity()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

template<class T>
void BlockDeque<T>::push_back(const T& item)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_)
    {
        condProducer_.wait(locker);
    }
    deq_.push_back(item);
    condConsumer_.notify_one();
}

template<class T>
void BlockDeque<T>::push_front(const T& item)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.size() >= capacity_)
    {
        condProducer_.wait(locker);
    }
    deq_.push_back(item);
    condProducer_.notify_one();
}

template<class T>
bool BlockDeque<T>::empty()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.empty();
}

template<class T>
bool BlockDeque<T>::full()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

template<class T>
bool BlockDeque<T>::pop(T& item)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.empty())
    {
        condConsumer_.wait(locker);
        if(isClose_)
            return false;
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

template<class T>
bool BlockDeque<T>::pop(T& item,int timeout)
{
    std::unique_lock<std::mutex> locker(mtx_);
    while(deq_.empty())
    {
        if(condConsumer_.wait_for(locker,std::chrono::seconds(timeout))
            == std::cv_status::timeout)
            return false;
        if(isClose_)
            return false;
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}
