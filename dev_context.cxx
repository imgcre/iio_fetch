//
// Created by imgcr on 2020/12/6.
//

#include "dev_context.hxx"
#include <functional>
#include <iio.h>
#include <vector>
#include <iostream>
#include "thread.hxx"
#include <memory>
#include <algorithm>

using namespace std;
const char* dev_context::kTrigDevName = "trigger0";

dev_context::dev_context():
  ctx(iio_create_default_context(), [](auto p) { iio_context_destroy(p); }),
  trig(iio_context_find_device(ctx.get(), kTrigDevName), [](auto p) { }) {
    set_trig_freq(kTrigFreq);
}

std::shared_ptr<dev_channel> dev_context::create_channel(const char* dev_name, const char* chn_name, std::function<float(int16_t)>&& conv) {
    auto device = std::shared_ptr<struct iio_device>(iio_context_find_device(ctx.get(), dev_name), [](auto p){ });
    auto chn = make_shared<dev_channel>(shared_from_this(), device, dev_name, chn_name, std::move(conv));
    device_map[device].chn_list.push_back(chn);
    return chn;
}

void dev_context::set_trig_freq(int freq) {
    iio_device_attr_write_longlong(trig.get(), "sampling_frequency", freq);
}

void dev_context::fetch_all(fetch_all_cb_t&& cb) {
    //对于每个设备、都创建buffer
    for(auto& device: device_map) {
        auto buf_size = 0;
        for(auto& chn: device.second.chn_list) {
            chn->setup();
            buf_size += chn->kBufferSize;
        }
        auto buffer = std::shared_ptr<struct iio_buffer>(iio_device_create_buffer(device.first.get(), buf_size, false), [](auto p) {
            iio_buffer_destroy(p);
        });
        iio_buffer_set_blocking_mode(buffer.get(), false);
        device.second.buffer = buffer;
        device.second.poll_fd = iio_buffer_get_poll_fd(buffer.get());

    }

    fetch_all_cb = move(cb);
    start();
}

void dev_context::start() {
    is_cancel = false;

    //生成fd数组
    auto poll_fds = std::vector<struct pollfd>(device_map.size());

    auto i = 0u;
    for(auto& device: device_map) {
        poll_fds[i].fd = device.second.poll_fd;
        poll_fds[i].events = POLLIN;
        i++;
    }

    while(!is_cancel) {
        poll(&poll_fds[0], poll_fds.size(), -1);
        for(auto& poll_fd: poll_fds) {
            if(!(poll_fd.revents & POLLIN)) continue;
            auto device = find_if(device_map.begin(), device_map.end(), [&poll_fd](auto& kvp) {
                return poll_fd == kvp.second.poll_fd;
            });
            if(device == device_map.end()) continue;
            auto buffer = device->second.buffer;
            int n_recv = iio_buffer_refill(buffer.get());
            if(n_recv == -EAGAIN) continue;
            auto inc = iio_buffer_step(buffer.get());
            auto end = iio_buffer_end(buffer.get());
            for(auto& chn: device->second.chn_list) {
                for(auto ptr = (int16_t*)iio_buffer_first(buffer.get(), chn->chn.get()); ptr < end; ptr += inc) {
                    chn->on_fetch(*ptr);
                }
            }



        }
    }


//    for(const auto& chn : chn_list) {
//        chn->start(dev_map[chn->get_dev_name()].second);
//    }

//    auto self(shared_from_this());
//    fetch_thr = std::thread([this, self](){
//        while (!is_cancel) {
//            auto values = std::vector<float>(chn_list.size());
//            auto i = 0;
//            for(auto& chn : chn_list) {
//                values[i] = chn->pop_val();
//                ++i;
//            }
//            thread_ex::setScheduling(fetch_thr, SCHED_RR, 50);
//            fetch_all_cb(move(values));
//            thread_ex::setScheduling(fetch_thr, SCHED_OTHER, 0);
//        }
//    });


}

void dev_context::stop() {
    is_cancel = true;
}