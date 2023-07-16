#include <quill/detail/LogMacros.h>
#include <http_client/common.h>
#include <system/stack_allocator.h>
#include "http_server/private/client_settings.h"
#include <http_server/private/thread.h>

namespace bro::net::http::server::private_ {

thread::thread(config &conf, quill::Logger *logger, std::vector<std::unordered_map<std::string, request_handler>> const &handlers, server::config const &server_config) :
    _config(conf),
    _logger(logger),
    _handlers(handlers),
    _server_config(server_config),
    _has_new_stream(false) {
    system::thread::config config;
    config._sleep = std::chrono::microseconds(100);
    config._call_sleep_on_n_empty_loop_in_a_row = 10'000;

    _thread.run_with_logic_pre_post(system::thread::callable(&thread::serve, this),
                                    system::thread::callable(&thread::logic_proceed, this),
                              system::thread::callable(&thread::pre_start, this),
                              system::thread::callable(&thread::post_end, this),
                              &config);
}

bool thread::set_new_stream(strm::stream_ptr &&stream) {
    std::lock_guard lg(_guard);
    _has_new_stream.store(true, std::memory_order_release);
    _new_streams.push_back(std::move(stream));
    return true;
}

void fill_headers(response & resp, config const & conf, client_settings const &cl_settings) {
    if(!conf._server_name.empty())
        resp.add_header(header::to_string(header::types::e_Server), conf._server_name.c_str());

    if (conf._connection_strategy == connection_strategy::e_keep_alive && cl_settings._keep_alive_connection) {
        char const *keep_alive = "Keep-Alive";
        resp.add_header(header::to_string(header::types::e_Connection), keep_alive);
    } else {
        char const *close = "Keep-Alive";
        resp.add_header(header::to_string(header::types::e_Connection), close);
    }

    int body_size = resp.get_body_v().empty() ? resp.get_body_s().size() : resp.get_body_v().size();
    if(body_size)
        resp.add_header(header::to_string(header::types::e_Content_Length), std::to_string(body_size));
}

void thread::parse_result_cb(request &req, std::any user_data, char const *error) {
    connection *con = std::any_cast<connection *>(user_data);
    if(error) {
        LOG_ERROR(con->_thread->_logger, "{}", error);
        con->_thread->_failed_connections.insert(con);
    } else {
        client_settings cl_settings;
        get_client_settings(req, cl_settings);
        response resp;
        auto const & handlers = con->_thread->_handlers[(int)req.get_type()];
        if(auto it = handlers.find(req.get_url()); it != handlers.end()) {
            auto & v = it->second;
            v._cb(std::move(req), resp, it->second._data);
        } else {
            LOG_INFO(con->_thread->_logger, "Receive request on url {} without handler", req.get_url());
            resp.add_header(std::string_view("date"), "Sun, 16 Jul 2023 16:05:02 GMT");
//            resp.add_header(std::string_view("x-cache-status"), "from content-cache-gs2/1");
//            resp.add_header(std::string_view("x-networkmanager-status"), "online");
            resp.set_status_code(status::code::e_Not_Found);
            cl_settings._keep_alive_connection = false;

            resp.add_body(std::string_view(R"(<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
        "http://www.w3.org/TR/html4/strict.dtd">
<html>
    <head>
        <meta http-equiv="Content-Type" content="text/html;charset=utf-8">
        <title>Error response</title>
    </head>
    <body>
        <h1>Error response</h1>
        <p>Error code: 404</p>
        <p>Message: File not found.</p>
        <p>Error code explanation: HTTPStatus.NOT_FOUND - Nothing matches the given URI.</p>
    </body>
</html>")"), std::string("text/html;charset=utf-8"));
        }


        auto const & conf = con->_thread->_server_config;
        fill_headers(resp, con->_thread->_server_config, cl_settings);

        int total_size = resp.get_total_size();
        int body_size = resp.get_body_v().empty() ? resp.get_body_s().size() : resp.get_body_v().size();
        total_size += header::to_string(conf._version).size() + status::to_string_as_number(resp.get_status_code()).size() +
                      status::to_string(resp.get_status_code()).size() + client::e_eol_size + client::e_2_spoce_size;

        total_size += body_size + client::e_eol_size;

        // generate data
        system::stack_allocator<std::byte, client::e_max_msg_size> st_allocator(total_size);
        auto res = fmt::format_to_n((char *) st_allocator.get_array(),
                                    total_size,
                                    "{} {} {}\r\n",
                                    header::to_string(conf._version),
                                    status::to_string_as_number(resp.get_status_code()),
                                    status::to_string(resp.get_status_code()));
        total_size -= res.size;
        for (auto const &hdr : resp.get_headers_s()) {
            res = fmt::format_to_n(res.out, total_size, "{}: {}\r\n", hdr.first, hdr.second);
            total_size -= res.size;
        }
        for (auto const &hdr : resp.get_headers_v()) {
            res = fmt::format_to_n(res.out, total_size, "{}: {}\r\n", hdr.first, hdr.second);
            total_size -= res.size;
        }
        res = fmt::format_to_n(res.out, total_size, "\r\n");
        total_size -= res.size;
        if(body_size) {
            if(!resp.get_body_s().empty()) {
                fmt::format_to_n(res.out, total_size, "{}", resp.get_body_s());
            } else {
                fmt::format_to_n(res.out, total_size, "{}", resp.get_body_v());
            }
        }

        LOG_INFO(con->_thread->_logger, "bum bum {}", std::string((char *)st_allocator.get_array(), st_allocator.get_size()));
        con->_stream->send(st_allocator.get_array(), st_allocator.get_size());
//        con->_stream.reset();
    }
}

void thread::process_new_stream(strm::stream_ptr &&stream) {
    if (!stream->is_active()) {
        LOG_ERROR(_logger, "Create stream failed with error - {}", stream->get_error_description());
        return;
    }

    _factory.bind(stream);
    auto strm = stream.get();
    std::unique_ptr<connection> strm_node(new connection{this, std::move(stream), {}});
    request_parser * req_h = &strm_node->_transaction._request_parser;
    req_h->init(parse_result_cb, strm_node.get());

    LOG_INFO(_logger, "Add new stream {} to thread - {}", fmt::ptr(strm), _config._name);

    strm->set_state_changed_cb(
        [&](strm::stream *strm, std::any user_data) {
            if (!strm->is_active()) {
                LOG_ERROR(_logger, "Send stream {} closed with status - {}", fmt::ptr(strm), strm->get_error_description());
                connection *con = std::any_cast<connection *>(user_data);
                _failed_connections.insert(con);
            } else {
                LOG_INFO(_logger, "Send stream {} state changed with status - {}", fmt::ptr(strm), bro::strm::state_to_string(strm->get_state()));
            }
        },
        strm_node.get());
    strm->set_received_data_cb(
        [&](strm::stream *strm, std::any user_data) {
            system::stack_allocator<std::byte, client::e_max_msg_size> st_allocator;
            _processed_data++;
            auto res = strm->receive(st_allocator.get_array(), st_allocator.get_size());
            if (res > 0) {
                request_parser *req = std::any_cast<request_parser *>(user_data);
                req->process_data(st_allocator.get_array(), res);
            }
        },
        req_h);
    _streams.insert({strm, std::move(strm_node)});
}

size_t thread::get_number_of_active_streams() const noexcept {
    return _streams.size();
}

void thread::pre_start() {
    _thread.set_name(_config._name);
    if(_config._core)
        _thread.set_affinity({*_config._core});
    LOG_INFO(_logger, "Start handler thread {}", _thread.get_name());
}

void thread::post_end() {
    LOG_INFO(_logger, "Stop handler thread {}", _thread.get_name());
}

bool thread::serve() {
    _factory.proceed();
    bool had_new_streams = _processed_data > 0;
    _processed_data = 0;
    return had_new_streams;
}

void thread::logic_proceed() {
    if(_has_new_stream.load(std::memory_order_acquire)) {
        std::lock_guard lg(_guard);
        while(!_new_streams.empty()) {
            process_new_stream(std::move(_new_streams.front()));
            _new_streams.pop_front();
        }
        _has_new_stream.store(false, std::memory_order_release);
    }
    if(!_failed_connections.empty()) {
        for(auto fc : _failed_connections) {
            _streams.erase(fc->_stream.get());
        }
        _failed_connections.clear();
    }
}

} // namespace bro::net::http::server::private_
