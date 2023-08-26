#pragma once
#include <chrono>
#include <list>
#include <optional>
#include <string>
#include <quill/LogLevel.h>
#include <network/stream/listen/settings.h>
#include <http_client/version.h>
#include <stream/factory.h>

namespace bro::net::http::server::config {

enum class connection_strategy : uint8_t {
    e_close,
    e_keep_alive
};

struct logger {
    quill::LogLevel _level{quill::LogLevel::Critical};
    std::optional<size_t> _core;
    std::string _file_name;
    std::string _thread_name{"server_logger"};
    std::string _logger_name{"server_logger"};
};

struct handlers {
    size_t _total = 1;
    std::list<size_t> _core_ids;
    std::string _prefix_name{"handler"};
    std::chrono::microseconds _sleep{500};
    size_t _call_sleep_on_n_empty_loop_in_a_row = 10;
    std::chrono::milliseconds _flush_statistic{1000};
};

struct listener {    
    std::unique_ptr<bro::strm::factory> _factory;
    std::optional<size_t> _core_id;
    std::string _name{"http_server"};
    std::chrono::microseconds _sleep{500};
    size_t _call_sleep_on_n_empty_loop_in_a_row = 10;
    std::chrono::milliseconds _flush_statistic{1000};
};

struct http_specific {
    header::version _version{header::version::e_1_1};                      ///< http version
    connection_strategy _connection_strategy{connection_strategy::e_keep_alive};
    bool _compress_body = true;
    bool _generate_date_in_response = true;
    std::string _server_name;
};

struct server {
    handlers _handlers;
    listener _listener;
    http_specific _http_specific;
    logger _logger;
};

} // namespace bro::net::http::server::config
