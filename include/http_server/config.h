#pragma once
#include <network/tcp/ssl/listen/settings.h>
#include <http_client/version.h>
#include <list>
#include <optional>
#include <string>

namespace bro::net::http::server {
/**
  * \brief connection type
  */
enum class connection_type {
    e_http, ///< http
    e_https ///< https
};

enum class connection_strategy : uint8_t {
    e_close,
    e_keep_alive
};

struct config {
    bro::net::tcp::ssl::listen::settings _settings;

    size_t _handler_threads = 1;
    connection_type _connection_type{connection_type::e_https};            ///< connection type
    header::version _version{header::version::e_1_1}; ///< http version
    std::optional<size_t> _listener_affinity_core;
    std::list<size_t> _handler_threads_affinity_cores;
    connection_strategy _connection_strategy{connection_strategy::e_keep_alive};
    std::string _server_name{"http_server"};
    bool _compress_body = true;
    bool _generate_date_in_response = true;
};


} // namespace bro::net::http::server
