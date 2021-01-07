//
// Created by imgcr on 2020/12/6.
//

#ifndef IIO_FETCH_DEV_CHANNEL_HXX
#define IIO_FETCH_DEV_CHANNEL_HXX

#include "dev_context.hxx"
#include <memory>
#include <iio.h>
#include <thread>
#include <functional>
#include <safe_queue.hxx>
#include "thread.hxx"

class dev_context;

class dev_channel: public std::enable_shared_from_this<dev_channel> {
    friend class dev_content;
public:
    dev_channel(std::shared_ptr<dev_context> owner, std::shared_ptr<struct iio_device> dev, const char* dev_name, const char* chn_name, std::function<float(int16_t)>&& conv);

    void setup();
    void start(std::shared_ptr<struct iio_buffer> buf);

    float pop_val();
    const char* get_dev_name() {
        return dev_name;
    }

protected:
    virtual void on_fetch(float value);

public:
    const static int kBufferSize = 100;

private:
    std::shared_ptr<dev_context> owner;
    std::shared_ptr<struct iio_device> dev;
    std::shared_ptr<struct iio_channel> chn;
    std::thread thread;
    safe_queue<float> queue;
    const char *chn_name, *dev_name;
    std::function<float(int16_t)> conv;
};


#endif //IIO_FETCH_DEV_CHANNEL_HXX
