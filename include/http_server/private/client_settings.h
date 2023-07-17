#pragma once
#include <http_server/request.h>

namespace bro::net::http::server::private_ {

struct client_settings {
    bool _accept_gzip_deflate = false;
    bool _keep_alive_connection = true;
};

inline void get_client_settings(request &req, client_settings & cl_settings) {
    auto const & hdrs  = req.get_headers();
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
}
} // namespace bro::net::http::server::private_

