#include <queue>
#include "network/stream/factory.h"
#include "quill/Quill.h"
#include <http_server/http_server.h>
#include <http_server/private/thread.h>
#include <http_server/request_handler.h>
#include <system/thread/thread.h>
#include <network/tcp/ssl/listen/settings.h>

namespace bro::net::http::server {

class http_server_internal {
public:

    http_server_internal(config const &conf);
    ~http_server_internal();
    bool add_handler(http::client::request::type type, std::string const &path, request_handler const & request_h);
    bool start();
    bool is_running() const;
    void stop();

private:

    std::unique_ptr<http_server_internal> _pimpl;

    void pre_start();
    void post_end();
    bool serve() ;
    void process_new_stream(strm::stream_ptr &&stream);
    bool create_listen_stream();

    config _config;
    bro::net::ev::factory _factory;                                        ///< send stream factory
    bro::strm::stream_ptr _listen_stream;                                         ///< connection to server
    std::vector<std::unordered_map<std::string, request_handler>> _handlers;
    std::vector<std::unique_ptr<http::server::private_::thread>> _threads;
    quill::Logger* _logger = nullptr;
    system::thread::thread _thread;
    size_t _new_stream_count{0};
};

http_server_internal::http_server_internal(const config &conf) : _config(conf), _handlers(int(http::client::request::type::e_Unknown_Type)) {
    /*auto stdout_handler_1 = quill::stdout_handler("stdout_1");
    stdout_handler_1->set_pattern(
        "%(ascii_time) [%(process)] [%(thread)] LOG_%(level_name) %(logger_name) - %(message)", // message format
        "%D %H:%M:%S.%Qms %z",     // timestamp format
        quill::Timezone::GmtTime); // timestamp's timezone
    _logger = quill::create_logger("http_server_logger", std::move(stdout_handler_1)); */
    _logger = quill::create_logger("http_server_logger");
    _logger->set_log_level(quill::LogLevel::TraceL3);
}

http_server_internal::~http_server_internal() {
    _threads.clear();
}

bool http_server_internal::add_handler(http::client::request::type type, std::string const &path, request_handler const & request_h) {
    if(_thread.is_running()) {
        LOG_ERROR(_logger, "Couldn't add new handler while serser in active state");
        return false;
    }
    int pos = (int)type;
    return (pos < (int)_handlers.size()) ? _handlers[(int)type].insert({path, request_h}).second : false;
}

bool http_server_internal::start() {
    if(!create_listen_stream())
        return false;

    for(size_t i = 0; i < _config._handler_threads; ++i) {
        std::optional<size_t> core;
        if(!_config._handler_threads_affinity_cores.empty()) {
            auto & cores = _config._handler_threads_affinity_cores;
            core = cores.front();
            cores.splice(cores.end(), cores, cores.begin());
        }
        private_::thread::config conf{._name = "request_handler_" + std::to_string(i), ._core = core };
        _threads.emplace_back(std::make_unique<private_::thread>(conf, _logger, _handlers, _config));
    }

    system::thread::config config;
    config._sleep = std::chrono::microseconds(100);
    config._call_sleep_on_n_empty_loop_in_a_row = 10'000;

    _thread.run_with_pre_post(system::thread::callable(&http_server_internal::serve, this),
                              system::thread::callable(&http_server_internal::pre_start, this),
                              system::thread::callable(&http_server_internal::post_end, this),
                              &config);
    return true;
}

bool http_server_internal::is_running() const {
    return _thread.is_running();
}

void http_server_internal::stop() {
    _threads.clear();
    _thread.stop();
}

void http_server_internal::pre_start() {
    _thread.set_name("http_server");
    if(_config._listener_affinity_core)
        _thread.set_affinity({*_config._listener_affinity_core});
    LOG_INFO(_logger, "Start Server thread {}", _thread.get_name());
}

void http_server_internal::post_end() {
    LOG_INFO(_logger, "Stop Server thread {}", _thread.get_name());
}

bool http_server_internal::serve() {
    _factory.proceed();
    bool had_new_streams = _new_stream_count > 0;
    _new_stream_count = 0;
    return had_new_streams;
}

void http_server_internal::process_new_stream(strm::stream_ptr &&stream) {
    _new_stream_count++;
    auto min_thr = std::min_element(_threads.begin(), _threads.end(), [](auto const &l, auto const &r) {
        return l->get_number_of_active_streams() < r->get_number_of_active_streams();
    });
    (*min_thr)->set_new_stream(std::move(stream));
}

bool http_server_internal::create_listen_stream() {

    auto in_connections = [&](strm::stream_ptr &&stream, tcp::listen::settings::in_conn_handler_data_cb /*data*/) {
        if (!stream->is_active()) {
            LOG_ERROR(_logger, "fail to create incomming connection {}", stream->get_error_description());
            return;
        }

        process_new_stream(std::move(stream));
    };

    switch (_config._connection_type) {
    case connection_type::e_http: {
        bro::net::tcp::listen::settings settings = _config._settings;
        settings._proc_in_conn = in_connections;
        _listen_stream = _factory.create_stream(&settings);
        break;
    }
    case connection_type::e_https: {
        _config._settings._proc_in_conn = in_connections;
        _listen_stream = _factory.create_stream(&_config._settings);
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
                LOG_ERROR(_logger, "Listen stream {} closed with error - {}", fmt::ptr(strm), _listen_stream->get_error_description());
            } else {
                LOG_INFO(_logger, "Listen stream {} state changed with status - {}", fmt::ptr(strm), bro::strm::state_to_string(strm->get_state()));
            }
        },
        nullptr);

    _factory.bind(_listen_stream);
    return true;
}


http_server::http_server(config const &conf) : _server(std::make_unique<http_server_internal>(conf)) {
}

http_server::~http_server() {

}

bool http_server::add_handler(http::client::request::type type, std::string const &path, request_handler const & request_h) {
    return _server->add_handler(type, path, request_h);
}

bool http_server::start() {
    return _server->start();
}

bool http_server::is_running() const {
    return _server->is_running();
}

void http_server::stop() {
    return _server->stop();
}

} // namespace bro::net::http::server
