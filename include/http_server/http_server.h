#pragma once
#include "quill/Logger.h"
#include <http_server/request.h>
#include <system/thread/thread.h>

namespace bro::net::http::server {

class respons_sender {
public:
    virtual void send_reponse() = 0;
};

class http_server : public respons_sender {
public:

    /**
  * \brief connection type
  */
    enum class connection_type {
        e_http, ///< http
        e_https ///< https
    };

    /**
    * \brief header types
    */
    struct request_handler {
        /**
        *  \brief A type alias for a callback function that process incoming requests
        */
        using cbt = std::function<void(request &&req, respons_sender & srv, std::any user_data)>;
        cbt _cb;        ///< request handler
        std::any _data; ///< user data
    };

    struct config {
        size_t handler_threads_count = 1;
        bool use_thread_for_server = false;
        connection_type _connection_type{connection_type::e_https};            ///< connection type
        std::vector<size_t> affinity_cores;
    };

    http_server(proto::ip::full_address const & addr, config * conf = nullptr);
    bool add_handler(http::client::request::type type, std::string const &path, request_handler const & request_h);
    bool start();
    bool is_running() const;
    void stop();

private:

    void pre_start();
    void post_end();
    void remove_handler(http::client::request::type type, std::string const &path);
    void serve() ;
    void send_reponse() override;
    void process_new_stream(strm::stream_ptr &&stream);
    bool create_stream();


    bro::net::proto::ip::full_address _self_addr;
    bro::net::ev::factory _factory;                                        ///< send stream factory
    bro::strm::stream_ptr _listen_stream;                                         ///< connection to server
    std::vector<std::unordered_map<std::string, request_handler>> _handlers;
    config _config;
    quill::Logger* _logger = nullptr;
    system::thread::thread _thread;
};

} // namespace bro::net::http::server
