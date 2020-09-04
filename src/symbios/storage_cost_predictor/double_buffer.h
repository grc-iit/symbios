//
// Created by lukemartinlogan on 9/3/20.
//

#ifndef SYMBIOS_DOUBLE_BUFFER_H
#define SYMBIOS_DOUBLE_BUFFER_H

#include <common/debug.h>
#include <mutex>
#include <list>

template<typename T>
class DoubleBuffer {
private:
    std::mutex locks_[2];
    std::list<T> lists_[2];
public:
    DoubleBuffer() {}
    void push(int tid, const T &obj) {
        for(int i = 0; i < 2; ++i) {
            if(locks_[tid].try_lock()) {
                lists_[tid].push_back(obj);
                locks_[tid].unlock();
                return;
            }
            tid = !tid;
        }
        locks_[tid].lock();
        lists_[tid].push_back(obj);
        locks_[tid].unlock();
    }
    bool pop_sync(T &obj) {
        for(int i = 0; i < 2; ++i) {
            if(lists_[i].size()) {
                locks_[i].lock();
                obj = std::move(lists_[i].front());
                lists_[i].pop_front();
                locks_[i].unlock();
                return true;
            }
        }
        return false;
    }
    bool pop(int tid, T &obj) {
        for(int i = 0; i < 2; ++i) {
            if(lists_[tid].size()) {
                locks_[tid].lock();
                obj = std::move(lists_[tid].front());
                lists_[tid].pop_front();
                locks_[tid].unlock();
                return true;
            }
            tid = !tid;
        }
        return false;
    }
};

#endif //SYMBIOS_DOUBLE_BUFFER_H
