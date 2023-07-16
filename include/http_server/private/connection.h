#pragma once
#include "http_server/private/transaction.h"
#include <stream/stream.h>
#include <http_server/request.h>

namespace bro::net::http::server::private_ {

class thread;

struct connection {
    thread * _thread = nullptr;
    strm::stream_ptr _stream;
    transaction _transaction;
};

} // namespace bro::net::http::server::private_

