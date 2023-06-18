#include "quill/Quill.h"
#include <http_server/http_server.h>
#include "network/tcp/ssl/listen/settings.h"

namespace bro::net::http::server {


http_server::http_server(proto::ip::full_address const & addr, config * conf) : _self_addr(addr), _handlers(int(http::client::request::type::e_Unknown_Type)) {
    //init_parser();
    _logger = quill::create_logger("http_server_logger");
    _logger->set_log_level(quill::LogLevel::TraceL3);
    if(conf)
        _config = *conf;
}

bool http_server::add_handler(http::client::request::type type, std::string const &path, request_handler const & request_h) {
    int pos = (int)type;
    return (pos < (int)_handlers.size()) ? _handlers[(int)type].insert({path, request_h}).second : false;
}

bool http_server::start() {
    if(!create_stream())
        return false;

    system::thread::config config;
    config._sleep = std::chrono::milliseconds(1);
    config._to_sleep_cycles = 1000;

    _thread.run_with_pre_post(system::thread::callable(&http_server::serve, this),
                              system::thread::callable(&http_server::pre_start, this),
                              system::thread::callable(&http_server::post_end, this),
                              &config);
    return true;
}

bool http_server::is_running() const {
    return _thread.is_running();
}

void http_server::stop() {
    _thread.stop();
}


void http_server::pre_start() {
    _thread.set_name("http_server");
    LOG_INFO(_logger, "Start Server");
}

void http_server::post_end() {
    LOG_INFO(_logger, "Stop Server");
}

void http_server::remove_handler(http::client::request::type type, std::string const &path) {
    int pos = (int)type;
    if(pos < (int)_handlers.size())
        _handlers[(int)type].erase(path);
}

void http_server::serve() {
    _factory.proceed();
}

void http_server::send_reponse() {

}


void http_server::process_new_stream(strm::stream_ptr &&/*stream*/) {
    //        auto *linux_stream = dynamic_cast<strm::settings const *>(stream->get_settings());
    //std::cout << "incoming connection from - " << linux_stream->_peer_addr << ", to - " << *linux_stream->_self_addr
    //          << std::endl;

    /*auto *cdata = std::any_cast<data_per_thread *>(data);
            stream->set_received_data_cb(::received_data_cb, data);
            stream->set_state_changed_cb(::state_changed_cb, data);
            cdata->_manager->bind(stream);
            cdata->_streams[stream.get()] = std::move(stream);*/
}

bool http_server::create_stream() {

    auto in_connections = [&](strm::stream_ptr &&stream, tcp::listen::settings::in_conn_handler_data_cb /*data*/) {
        if (!stream->is_active()) {
            LOG_ERROR(_logger, "fail to create incomming connection %s", stream->get_error_description());
            return;
        }

        process_new_stream(std::move(stream));
    };

    switch (_config._connection_type) {
    case connection_type::e_http: {
        bro::net::tcp::listen::settings settings;
        settings._listen_address = _self_addr;
        settings._proc_in_conn = in_connections;
        _listen_stream = _factory.create_stream(&settings);
        break;
    }
    case connection_type::e_https: {
        bro::net::tcp::ssl::listen::settings settings;
        settings._listen_address = _self_addr;
        settings._proc_in_conn = in_connections;
        _listen_stream = _factory.create_stream(&settings);
        break;
    }
    }

    if (!_listen_stream->is_active()) {
        LOG_ERROR(_logger, "Create stream failed with error - {}", _listen_stream->get_error_description());
        return false;
    }

    _listen_stream->set_state_changed_cb(
        [&](bro::strm::stream *strm, std::any) {
            if (!strm->is_active()) {
                LOG_ERROR(_logger, "Stream became not active - {}", _listen_stream->get_error_description());
            }
        },
        nullptr);

    _factory.bind(_listen_stream);
    return true;
}

} // namespace bro::net::http::server
