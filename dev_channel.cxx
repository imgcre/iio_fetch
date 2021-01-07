//
// Created by imgcr on 2020/12/6.
//

#include "dev_channel.hxx"
#include <thread>
#include <iostream>
#include <functional>
#include <cerrno>
#include <utility>
#include <memory.h>
#include <ctime>
#include <chrono>
#include <cstdio>


using namespace std::chrono_literals;

dev_channel::dev_channel(
    std::shared_ptr<dev_context> owner,
    std::shared_ptr<struct iio_device> dev,
    const char* dev_name,
    const char* chn_name,
    std::function<float(int16_t)>&& conv
):
  owner(owner),
  dev(dev),
  dev_name(dev_name),
  chn_name(chn_name),
  chn(iio_device_find_channel(dev.get(), chn_name, false), [](auto p) {
      if(iio_channel_is_enabled(p)) {
          iio_channel_disable(p);
      }
  }),
  conv(move(conv)) {
    iio_device_set_trigger(dev.get(), owner->trig.get());

}

void dev_channel::setup() {
    iio_channel_enable(chn.get());
}

void dev_channel::start(std::shared_ptr<struct iio_buffer> buf) {
    auto self(shared_from_this());
    thread = std::thread([this, self, buf](){
        std::cout << "Fetching " << chn_name << "@" << buf << std::endl;

        while(buf.get()) {
            int n_recv = iio_buffer_refill(buf.get());
            if(n_recv == -EAGAIN) {
                std::this_thread::yield();
                continue;
            }
            auto inc = iio_buffer_step(buf.get());
            auto end = iio_buffer_end(buf.get());

            system("");
            system("echo 0 > /sys/class/leds/red/brightness");
            thread_ex::setScheduling(thread, SCHED_RR, 99);
            auto clock_before = clock();
            for(auto ptr = (int16_t*)iio_buffer_first(buf.get(), chn.get()); ptr < end; ptr += inc) {
                //on_fetch(conv(*ptr));
            }

            auto fps = 1000. / (clock() - clock_before);
            if(fps < 10) {
                printf("[%s] fps: %4.2f\n", chn_name, fps);
            }
            thread_ex::setScheduling(thread, SCHED_IDLE, 0);
            system("echo 1 > /sys/class/leds/red/brightness");
            //system("echo 1 > /sys/class/leds/red/brightness");


        }

    });


}

void dev_channel::on_fetch(float value) {
    static int i = 0;

    if(strcmp(chn_name, "voltage1") == 0) {
        i++;
        //printf("%04.2f%s", value, i % 16 ? " ": "\n");
    }

    //queue.enqueue(value);
}

float dev_channel::pop_val() {
    return queue.dequeue();
}