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
/**
 * @brief what should we do after process request
 */
enum class connection_strategy : uint8_t {
    e_close,                                                    ///< close connection after handle request
    e_keep_alive                                                ///< set keep alive
};

/**
 * \brief logger settings
 */
struct logger {
    quill::LogLevel _level{quill::LogLevel::Critical};          ///< log leverl
    std::optional<size_t> _core;                                ///< what core should logger thread use
    std::string _file_name;                                     ///< log filename
    std::string _thread_name{"server_logger"};               ///< what name should logger thread use
    std::string _logger_name{"server_logger"};               ///< what name should logger thread use
};

/**
 * \brief handlers settings
 */
struct handlers {
    size_t _total = 1;                                          ///< how many handlers(threads) will process requests
    std::list<size_t> _core_ids;                                ///< which cores should we bind handlers(threads)
    std::string _prefix_name{"handler"};                     ///< prefix name for handler(thread) naming threads with pattern _prefix_name + id
    std::chrono::microseconds _sleep{500};                 ///< how long should thread sleep if nothing to do
    size_t _call_sleep_on_n_empty_loop_in_a_row = 10;          ///< cretaria to sleep. if n calls of main handler function were useless
    std::chrono::milliseconds _flush_statistic{1000};      ///< how often should thread update statistic of it's works (do not print it now)
};

/**
 * \brief listener thread settings
 */
struct listener {    
    std::unique_ptr<bro::strm::factory> _factory;               ///< pointer on factory how we create streams. If not set use default. (more for tests)
    std::optional<size_t> _core_id;                             ///< which core should we use
    std::string _name{"http_server"};                         ///< name for thread
    std::chrono::microseconds _sleep{500};                  ///< how long should thread sleep if nothing to do
    size_t _call_sleep_on_n_empty_loop_in_a_row = 10;           ///< cretaria to sleep. if n calls of main handler function were useless (there were no new connections)
    std::chrono::milliseconds _flush_statistic{1000};       ///< how often should thread update statistic of it's works (do not print it now)
};

/**
 * \brief http specific settings
 */
struct http_specific {
    header::version _version{header::version::e_1_1};                               ///< http version
    connection_strategy _connection_strategy{connection_strategy::e_keep_alive};    ///< what should do after handle request
    bool _compress_body = true;                                                     ///< compress response body   
    bool _generate_date_in_response = true;                                         ///< if true will geneate date in reponse
    std::string _server_name;                                                       ///< if set, will set server name in response
};

/**
 * \brief server settings
 */
struct server {
    handlers _handlers;             ///< handler(threads) settings
    listener _listener;             ///< listener(thread) settings
    http_specific _http_specific;   ///< http specifc settings
    logger _logger;                 ///< logger settings
};

} // namespace bro::net::http::server::config
