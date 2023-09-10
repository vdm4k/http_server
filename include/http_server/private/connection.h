#pragma once
#include <http_server/private/request_parser.h>
#include <stream/stream.h>
#include <http_server/request.h>

namespace bro::net::http::server::private_ {

class thread;

/**
 * \brief connection descriptor
 */
struct connection_descriptor {
    thread * _thread = nullptr;     ///< pointer on handle thread 
    strm::stream_ptr _stream;       ///< specific stream (tcp(ssl))
    request_parser _request_parser; ///< request parser
};

} // namespace bro::net::http::server::private_

