#pragma once

#include "logger/report.h"

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>

#include <chrono>

namespace datetime {

namespace net = boost::asio;
namespace sys = boost::system;

class Ticker : public std::enable_shared_from_this<Ticker> {
  public:
    using Strand = net::strand<net::io_context::executor_type>;
    using Handler = std::function<void(std::chrono::milliseconds delta)>;

    Ticker(Strand strand, std::chrono::milliseconds period, Handler handler) :
        strand_(std::move(strand)),
        timer_(strand_),
        period_(std::move(period)),
        handler_(std::move(handler)) {}

    void Start() {
        last_tick_ = std::chrono::steady_clock::now();
        ScheduleTick();
    }

  private:
    void ScheduleTick() {
        timer_.expires_after(period_);
        timer_.async_wait(net::bind_executor(
            strand_,
            [self = shared_from_this()](sys::error_code ec) {
                self->OnTick(ec);
            }
        ));
    }

    void OnTick(sys::error_code ec) {
        if (ec) {
            logger::ReportError(ec, "ticker");
        }

        auto current_tick = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            current_tick - last_tick_
        );

        handler_(duration);
        last_tick_ = current_tick;
        ScheduleTick();
    }

    Strand strand_;
    net::steady_timer timer_ {strand_};
    std::chrono::milliseconds period_;
    Handler handler_;
    std::chrono::time_point<std::chrono::steady_clock> last_tick_;
};

}  // namespace datetime
