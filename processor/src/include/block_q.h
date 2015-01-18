#ifndef SCRIBE_BLOCK_Q_H
#define SCRIBE_BLOCK_Q_H
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

//code change from here as a convinient
//[https://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html]
//
template<typename T>
class BlockQueue {
public:
    void push(T const& data) {
        boost::mutex::scoped_lock lock(_q_mutex);
        _queue.push(data);
        lock.unlock();
        _q_cond.notify_one();
    }

    bool empty() const {
        boost::mutex::scoped_lock lock(_q_mutex);
        return _queue.empty();
    }

    bool try_pop(T& value) {
        boost::mutex::scoped_lock lock(_q_mutex);
        if(_queue.empty())
        {
            return false;
        }
        
        value=_queue.front();
        _queue.pop();
        return true;
    }

    void wait_and_pop(T& value) {
        boost::mutex::scoped_lock lock(_q_mutex);
        while(_queue.empty()) {
            _q_cond.wait(lock);
        }
        
        value=_queue.front();
        _queue.pop();
    }

private:
    std::queue<T> _queue;
    mutable boost::mutex _q_mutex;
    boost::condition_variable _q_cond;
};

#endif
