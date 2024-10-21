#pragma once
#ifndef TWIN_COUNTINGSEMAPHORE_H
#define TWIN_COUNTINGSEMAPHORE_H
//This is necessary because c++17 doesn't have countingsemaphores itself
#include "twinDefines.h"
#include <mutex>
#include <condition_variable>

TWIN_NAMESPACE_BEGIN
    template<int max>
    class CountSemaphore {
    public:
        static_assert(max >= 0);

        explicit CountSemaphore(int init_count = max) : m_count(init_count) {}

        // P-operation / acquire
        void wait() {
            std::unique_lock<std::mutex> lk(m_lock);
            m_cv.wait(lk, [this] { return 0 < m_count; });
            --m_count;
        }

        bool try_wait() {
            std::lock_guard<std::mutex> lk(m_lock);
            if (0 < m_count) {
                --m_count;
                return true;
            } else {
                return false;
            }
        }

        // V-operation / release
        void signal() {
            {
                std::lock_guard<std::mutex> lg(
                        m_lock);//we want to explicitly unlock before notify_one, because see api conditional variable
                if (m_count < max) {
                    ++m_count;
                }
            }
            m_cv.notify_one();
        }

        // Lockable requirements
        void lock() { wait(); }

        bool try_lock() { return try_wait(); }

        void unlock() { signal(); }

    private:
        int m_count;
        std::mutex m_lock;
        std::condition_variable m_cv;
    };

    using BinarySemaphore = CountSemaphore<1>;

TWIN_NAMESPACE_END
#endif //TWIN_COUNTINGSEMAPHORE_H
