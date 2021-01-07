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

void dev_channel::on_fetch(int16_t value) {
    //TODO: 转换并存入context类的队列中
}