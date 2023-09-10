#pragma once
#include <http_server/request.h>

namespace bro::net::http::server::private_ {

/**
 * \brief request(client) specific settings
 */
struct client_settings {
    bool _accept_gzip_deflate = false;  ///< support compress data 
    bool _keep_alive_connection = true; ///< not close connection after send response
};

 /**
  * \brief fill client settings based on request sended by client
  * 
  * \param req[in] request from client
  * \return client_settings client settings
  */
inline client_settings get_client_settings(request const &req) {
    auto const & hdrs  = req.get_headers();
    client_settings cl_settings{};
    for(auto const & hdr : hdrs) {
        if(hdr._type == header::to_string(header::types::e_Accept_Encoding)) {
            std::string val;
            std::transform(hdr._value.begin(), hdr._value.end(), std::back_inserter(val), ::tolower);
            if(auto res = val.find("gzip"); res != val.npos) {
                cl_settings._accept_gzip_deflate = true;
            }
        }

        if(hdr._type == header::to_string(header::types::e_Connection)) {
            std::string val;
            std::transform(hdr._value.begin(), hdr._value.end(), std::back_inserter(val), ::tolower);
            if(auto res = val.find("keep-alive"); res != val.npos) {
                cl_settings._keep_alive_connection = true;
            }
        }
    }
    return cl_settings;
}
} // namespace bro::net::http::server::private_

