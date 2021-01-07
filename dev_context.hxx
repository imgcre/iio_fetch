//
// Created by imgcr on 2020/12/6.
//

#ifndef IIO_FETCH_DEV_CONTEXT_HXX
#define IIO_FETCH_DEV_CONTEXT_HXX

#include <memory>
#include <iio.h>
#include "dev_channel.hxx"
#include <functional>
#include <list>
#include <vector>
#include <map>
#include <string>
#include <utility>
#include <poll.h>
#include <queue>

class dev_channel;

class dev_context: public std::enable_shared_from_this<dev_context> {
    friend class dev_channel;
    using fetch_all_cb_t = std::function<void(const std::vector<float>)>;
public:
    dev_context();
    std::shared_ptr<dev_channel> create_channel(const char* dev_name, const char* chn_name, std::function<float(int16_t)>&& conv);
    void fetch_all(fetch_all_cb_t&& cb);
    void start();
    void stop();

private:
    void set_trig_freq(int freq);

private:
    std::shared_ptr<struct iio_context> ctx;
    std::shared_ptr<struct iio_device> trig;

    struct device_info {
        std::list<std::shared_ptr<dev_channel>> chn_list = {};
        std::shared_ptr<struct iio_buffer> buffer = nullptr;
        int poll_fd = 0;
    };

    std::map<std::shared_ptr<struct iio_device>, device_info> device_map = {};


    fetch_all_cb_t fetch_all_cb;
    bool is_cancel = false;

    std::map<std::shared_ptr<dev_channel>, std::queue<float>> queues;

    static const char* kTrigDevName;
    static const int kTrigFreq = 1000;
};


#endif //IIO_FETCH_DEV_CONTEXT_HXX
