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

/**
 * \brief handler thread 
 */
class thread {
public:

    /**
     * \brief thread configuration
     */
    struct config {
        std::string _name;           ///< thread name
        std::optional<size_t> _core; ///< thread core
    };

    /**
     * \brief Construct a new thread
     * 
     * \param conf thread configuration
     * \param thread_conf system thread config (internal thread)
     * \param logger pointer on logger
     * \param handlers array of handlers
     * \param http_config htttp specific config
     */
    thread(config & conf,
           system::thread::config const & thread_conf,
           quill::Logger* logger,
           std::vector<std::unordered_map<std::string, request_handler>> const &handlers,
           bro::net::http::server::config::http_specific const & http_config);

    /**
     * \brief Set the new incomming stream 
     * 
     * \param stream 
     * \return true if stream set
     * \return false otherwise
     */
    bool set_new_stream(strm::stream_ptr &&stream);

    /**
     * \brief Get the number of active streams
     * 
     * \return size_t number of active streams
     */
    size_t get_number_of_active_streams() const noexcept;

private:

    /**
     * \brief thread pre start function
     */
    void pre_start();

    /**
     * \brief thread post(after) end function
     */
    void post_end();

    /**
     * \brief thread main function
     */
    bool serve();

    /**
     * \brief thread logic proceed function (handle new connections)
     */
    void logic_proceed();

    /**
     * \brief handle new connection
     * 
     * \param stream new connection
     */
    void process_new_stream(strm::stream_ptr &&stream);

    /**
     * \brief do compress body
     * 
     * \param resp response
     * \return true if compressed sucessfully
     * \return false otherwise
     */
    bool compress_body(response & resp);

    /**
     * \brief parser callback
     * 
     * \param req request 
     * \param user_data user data
     * \param error will be not null if error acquired 
     */
    static void parse_result_cb(request &req, std::any user_data, char const *error);

    config _config;                                                                             ///< thread configs
    quill::Logger* _logger = nullptr;                                                           ///< logger
    std::vector<std::unordered_map<std::string, request_handler>> const & _handlers;            ///< handlers
    bro::net::http::server::config::http_specific const & _http_config;                         ///< http specific configs
    bro::net::ev::factory _factory;                                                             ///< per thread factory for handle connections
    std::unordered_map<bro::strm::stream*, std::unique_ptr<connection_descriptor>> _streams;    ///< active streams
    std::set<connection_descriptor *> _failed_connections;                                      ///< array of failed connection to handle
    std::atomic_bool _has_new_stream;                                                           ///< new stream added
    std::mutex _guard;                                                                          ///< synchro guard
    std::list<strm::stream_ptr> _new_streams;                                                   ///< array of new streams
    system::thread::thread _thread;                                                             ///< internal thread
    size_t _processed_data{0};                                                                  ///< how many things were done in one call
};

} // namespace bro::net::http::server::private_

