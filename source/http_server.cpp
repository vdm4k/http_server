#include <memory>
#include "network/stream/factory.h"
#include "quill/Quill.h"
#include <http_server/http_server.h>
#include <http_server/private/thread.h>
#include <http_server/request_handler.h>
#include <system/thread/thread.h>
#include <network/tcp/ssl/listen/settings.h>

namespace bro::net::http::server {

/**
 * \brief http server implementation
 */
class http_server_internal {
public:

    /**
     * @brief Construct a new http server
     * 
     * @param connection_settings listen settings 
     */
    explicit http_server_internal(std::unique_ptr<bro::net::listen::settings> &&connection_settings);

    /**
     * @brief Construct a new http server 
     * 
     * @param connection_settings listen settings 
     * @param server_settings setver settings
     */
    http_server_internal(std::unique_ptr<bro::net::listen::settings> &&connection_settings, config::server &&server_settings);
    
    /**
     * @brief dtor
     */
    ~http_server_internal();

    /**
     * @brief add handler for specific url(path)
     * 
     * @param type request type (get, post, ...)
     * @param path specific path (url)
     * @param request_h handler for this specific request/url
     * @return true if handler was successfully added 
     * @return false otherwise (collision for specific path/type)
     */
    bool add_handler(http::client::request::type type, std::string const &path, request_handler const & request_h);

    /**
     * @brief start server
     * 
     * @return true if server started
     * @return false otherwise
     */
    bool start();

    /**
     * @brief is server running
     * 
     * @return true if it's running
     * @return false otherwise
     */
    bool is_running() const;

    /**
     * @brief stop server
     */
    void stop();

private:

    void pre_start();
    void post_end();
    bool serve() ;
    void process_new_stream(strm::stream_ptr &&stream);
    bool create_listen_stream();
    void init_logger();

    std::unique_ptr<bro::net::listen::settings> _connection_settings;
    config::server _server_settings;                                 
    bro::strm::stream_ptr _listen_stream;                                         ///< connection to server
    std::vector<std::unordered_map<std::string, request_handler>> _handlers;
    std::vector<std::unique_ptr<http::server::private_::thread>> _threads;
    quill::Logger* _logger = nullptr;
    system::thread::thread _thread;
    size_t _new_stream_count{0};
};

http_server_internal::http_server_internal(std::unique_ptr<bro::net::listen::settings> &&connection_settings) : 
    _connection_settings(std::move(connection_settings)),
    _handlers(int(http::client::request::type::e_Unknown_Type)) {
    _server_settings._listener._factory = std::make_unique<bro::net::ev::factory>();
    init_logger();
}


http_server_internal::http_server_internal(std::unique_ptr<bro::net::listen::settings> &&connection_settings, config::server &&server_settings) : 
_connection_settings(std::move(connection_settings)),
_server_settings(std::move(server_settings)),
_handlers(int(http::client::request::type::e_Unknown_Type)) {
    init_logger();
    if(!_server_settings._listener._factory)
       _server_settings._listener._factory = std::make_unique<bro::net::ev::factory>();
}

http_server_internal::~http_server_internal() {
    _threads.clear();
}

void http_server_internal::init_logger() {
    quill::Config q_cfg;
    q_cfg.enable_console_colours = true;
    q_cfg.backend_thread_empty_all_queues_before_exit = true;
    if(!_server_settings._logger._thread_name.empty())
        q_cfg.backend_thread_name = _server_settings._logger._thread_name;
    if(_server_settings._logger._core)
        q_cfg.backend_thread_cpu_affinity = *_server_settings._logger._core;

    if(!_server_settings._logger._file_name.empty()) {
        auto file_handler = quill::file_handler(_server_settings._logger._file_name, "w", quill::FilenameAppend::DateTime);
        q_cfg.default_handlers.push_back(std::move(file_handler));
    }


    quill::configure(q_cfg);
    quill::start();

    _logger = quill::create_logger(_server_settings._logger._logger_name);
    _logger->set_log_level(_server_settings._logger._level);
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

    for(size_t i = 0; i < _server_settings._handlers._total; ++i) {
        std::optional<size_t> core;
        if(!_server_settings._handlers._core_ids.empty()) {
            auto & cores = _server_settings._handlers._core_ids;
            core = cores.front();
            cores.splice(cores.end(), cores, cores.begin());
        }
        system::thread::config config;
        config._sleep = _server_settings._handlers._sleep;
        config._call_sleep_on_n_empty_loop_in_a_row = _server_settings._handlers._call_sleep_on_n_empty_loop_in_a_row;
        config._flush_statistic = _server_settings._handlers._flush_statistic;

        private_::thread::config conf{._name = "handler_" + std::to_string(i), ._core = core };
        _threads.emplace_back(std::make_unique<private_::thread>(conf, config, _logger, _handlers, _server_settings._http_specific));
    }

    system::thread::config config;
    config._sleep = _server_settings._listener._sleep;
    config._call_sleep_on_n_empty_loop_in_a_row = _server_settings._listener._call_sleep_on_n_empty_loop_in_a_row;
    config._flush_statistic = _server_settings._listener._flush_statistic;

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
    _thread.stop();
    _threads.clear();    
}

void http_server_internal::pre_start() {
    if(!_server_settings._listener._name.empty())
        _thread.set_name(_server_settings._listener._name);
    if(_server_settings._listener._core_id)
        _thread.set_affinity({*_server_settings._listener._core_id});
    LOG_INFO(_logger, "Start Server thread {}", _thread.get_name());
}

void http_server_internal::post_end() {
    LOG_INFO(_logger, "Stop Server thread {}", _thread.get_name());
}

bool http_server_internal::serve() {
    _server_settings._listener._factory->proceed();
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

    if(!_connection_settings) {
        LOG_ERROR(_logger, "pointer on listen settings is empty");
        return false;
    }

    _connection_settings->_proc_in_conn = in_connections;
    _listen_stream = _server_settings._listener._factory->create_stream(_connection_settings.get());

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

    _server_settings._listener._factory->bind(_listen_stream);
    return true;
}

http_server::http_server(std::unique_ptr<bro::net::listen::settings> &&connection_settings) : 
    _server(std::make_unique<http_server_internal>(std::move(connection_settings))) {
}

http_server::http_server(std::unique_ptr<bro::net::listen::settings> &&connection_settings, config::server &&server_settings) : 
    _server(std::make_unique<http_server_internal>(std::move(connection_settings), std::move(server_settings))) {
}

http_server::~http_server(){}

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
