#include "web/session.h"
#include "web/utils.h"

#include <boost/asio/dispatch.hpp>
#include <boost/json/value.hpp>
#include <iostream>

namespace web {

void SessionBase::Run() {
    net::dispatch(
        stream_.get_executor(),
        beast::bind_front_handler(&SessionBase::Read, GetSharedThis())
    );
}

void SessionBase::Read() {
    using namespace std::literals;
    request_ = {};
    stream_.expires_after(30s);
    http::async_read(
        stream_,
        buffer_,
        request_,
        beast::bind_front_handler(&SessionBase::OnRead, GetSharedThis())
    );
}

void SessionBase::OnRead(sys::error_code ec, size_t bytes_read) {
    using namespace std::literals;
    if (ec == http::error::end_of_stream) {
        return Close();
    }
    if (ec) {
        return ReportError(ec, "read");
    }

    HandleRequest(std::move(request_));
}

void SessionBase::OnWrite(
    bool close,
    sys::error_code ec,
    size_t bytes_written
) {
    if (ec) {
        ReportError(ec, "write");
    }
    if (close) {
        return Close();
    }

    Read();
}

void SessionBase::Close() {
    sys::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
}
}  // namespace web
