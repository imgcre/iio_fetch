//
// Created by imgcr on 2020/12/7.
//

#ifndef IIO_FETCH_THREAD_HXX
#define IIO_FETCH_THREAD_HXX

#include <thread>
#include <pthread.h>
#include <iostream>
#include <cstring>

class thread_ex : public std::thread
{
public:
    static void setScheduling(std::thread &th, int policy, int priority) {
        sched_param sch_params{};
        sch_params.sched_priority = priority;
        if(pthread_setschedparam(th.native_handle(), policy, &sch_params)) {
            std::cerr << "Failed to set Thread scheduling : " << std::strerror(errno) << std::endl;
        }
    }
private:
};

#endif //IIO_FETCH_THREAD_HXX
