#pragma once
#include <http_server/private/connection.h>
#include <http_server/request_handler.h>
#include <network/stream/factory.h>
#include "quill/Logger.h"
#include <list>
#include <mutex>
#include <set>
#include <system/thread/thread.h>
#include <http_server/private/request_parser.h>
#include <http_server/request.h>
#include <http_server/config.h>

namespace bro::net::http::server::private_ {

class thread {
public:

    struct config {
        std::string _name;
        std::optional<size_t> _core;
    };

    thread(config & conf, quill::Logger* logger, std::vector<std::unordered_map<std::string, request_handler>> const &handlers, server::config const & server_config);

    bool set_new_stream(strm::stream_ptr &&stream);
    size_t get_number_of_active_streams() const noexcept;

private:

    void pre_start();
    void post_end();
    bool serve();
    void logic_proceed();
    void process_new_stream(strm::stream_ptr &&stream);
    bool compress_body(response & resp);
    static void parse_result_cb(request &req, std::any user_data, char const *error);

    config _config;
    quill::Logger* _logger = nullptr;
    std::vector<std::unordered_map<std::string, request_handler>> const & _handlers;
    server::config const & _server_config;
    bro::net::ev::factory _factory;
    std::unordered_map<bro::strm::stream*, std::unique_ptr<connection>> _streams;
    std::set<connection *> _failed_connections;
    std::atomic_bool _has_new_stream;
    std::mutex _guard;
    std::list<strm::stream_ptr> _new_streams;
    system::thread::thread _thread;
    size_t _processed_data{0};
};

} // namespace bro::net::http::server::private_

