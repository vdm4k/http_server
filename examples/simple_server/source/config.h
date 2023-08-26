#pragma once
#include "http_server/config.h"
#include <http_client/request.h>

namespace bro::net::http::server::simple::config {

struct request_handler  {
    std::string _path;
    std::string _response_body;
    std::string _response_body_type;
    std::string _server_name;
    bro::net::http::client::request::type _type;
    bro::net::http::status::code _code{bro::net::http::status::code::e_OK};
};

struct ssl {
    std::string _certificate_path;
    std::string _key_path;
};

struct server {
    bro::net::http::server::config::server _server_config;
    std::vector<request_handler> _handlers;
    std::optional<ssl> _ssl;
    bro::net::proto::ip::full_address _address;
    std::optional<std::chrono::seconds> _test_time;
};

std::optional<server> parse(std::string const &path);

} // namespace bro::net::http::server::simple::config
