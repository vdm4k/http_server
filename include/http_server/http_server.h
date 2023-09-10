#pragma once
#include <http_server/request.h>
#include <http_server/response.h>
#include <http_server/config.h>
#include <http_server/request_handler.h>

namespace bro::net::http::server {

class http_server_internal;
/**
 * \brief http server 
 */
class http_server {
public:

    /**
     * @brief Construct a new http server
     * 
     * @param connection_settings listen settings 
     */
    explicit http_server(std::unique_ptr<bro::net::listen::settings> &&connection_settings);

    /**
     * @brief Construct a new http server 
     * 
     * @param connection_settings listen settings 
     * @param server_settings setver settings
     */
    http_server(std::unique_ptr<bro::net::listen::settings> &&connection_settings, config::server &&server_settings);

    /**
     * @brief dtor
     */
    ~http_server();

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

    std::unique_ptr<http_server_internal> _server; ///< pointer on implementation
};

} // namespace bro::net::http::server
