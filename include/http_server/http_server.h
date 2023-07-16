#pragma once
#include <http_server/request.h>
#include <http_server/response.h>
#include <http_server/config.h>
#include <http_server/request_handler.h>

namespace bro::net::http::server {

class http_server_internal;

class http_server {
public:

    http_server(config const &conf);
    ~http_server();
    bool add_handler(http::client::request::type type, std::string const &path, request_handler const & request_h);
    bool start();
    bool is_running() const;
    void stop();

private:

    std::unique_ptr<http_server_internal> _server;
};

} // namespace bro::net::http::server
