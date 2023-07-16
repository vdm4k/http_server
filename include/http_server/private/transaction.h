#pragma once
#include <http_server/request.h>
#include <http_server/response.h>
#include <http_server/private/request_parser.h>

namespace bro::net::http::server::private_ {

struct transaction {
    request_parser _request_parser;
};

} // namespace bro::net::http::server::private_

