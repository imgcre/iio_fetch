//
// Created by imgcr on 2020/12/6.
//

#include "dev_context.hxx"
#include <functional>
#include <iio.h>
#include <vector>
#include <iostream>
#include "thread.hxx"

const char* dev_context::kTrigDevName = "trigger0";

dev_context::dev_context():
  ctx(iio_create_default_context(), [](auto p) { iio_context_destroy(p); }),
  trig(iio_context_find_device(ctx.get(), kTrigDevName), [](auto p) { }) {
    set_trig_freq(kTrigFreq);
}

std::shared_ptr<dev_channel> dev_context::create_channel(const char* dev_name, const char* chn_name, std::function<float(int16_t)>&& conv) {
    if(dev_map.find(dev_name) == dev_map.end()) {
        auto dev = std::shared_ptr<struct iio_device>(iio_context_find_device(ctx.get(), dev_name), [](auto p){ });
//        auto buf =

        //std::cout << "Buf created: " << dev_name << "(" << dev << ") @ " << buf << std::endl;

        dev_map[dev_name] = {
            dev,
            nullptr,
        };
    }

    auto dev_info = dev_map[dev_name];

    auto chn = std::make_shared<dev_channel>(shared_from_this(), dev_info.first, dev_name, chn_name, std::move(conv));
    chn_list.push_back(chn);
    return chn;
}

void dev_context::set_trig_freq(int freq) {
    iio_device_attr_write_longlong(trig.get(), "sampling_frequency", freq);
}

void dev_context::fetch_all(fetch_all_cb_t&& cb) {
    auto sum_map = std::map<std::string, int> {};

    for(const auto& chn : chn_list) {
        chn->setup();
        sum_map[chn->get_dev_name()] += chn->kBufferSize;
    }

    for(auto& kvp : dev_map) {
        auto buf = std::shared_ptr<struct iio_buffer>(iio_device_create_buffer(kvp.second.first.get(), sum_map[kvp.first], false), [](auto p) {
            iio_buffer_destroy(p);
        });
        iio_buffer_set_blocking_mode(buf.get(), false);
        kvp.second.second = buf;
    }

    fetch_all_cb = move(cb);
    start();
}

void dev_context::start() {
    is_cancel = false;

    for(const auto& chn : chn_list) {
        chn->start(dev_map[chn->get_dev_name()].second);
    }

    auto self(shared_from_this());
    fetch_thr = std::thread([this, self](){
        while (!is_cancel) {
            auto values = std::vector<float>(chn_list.size());
            auto i = 0;
            for(auto& chn : chn_list) {
                values[i] = chn->pop_val();
                ++i;
            }
            thread_ex::setScheduling(fetch_thr, SCHED_RR, 50);
            fetch_all_cb(move(values));
            thread_ex::setScheduling(fetch_thr, SCHED_OTHER, 0);
        }
    });


}

void dev_context::stop() {
    is_cancel = true;
}