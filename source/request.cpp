#include <http_server/request.h>

namespace bro::net::http::server {

std::string const & request::get_url() const {
    return _url;
}

client::request::type request::get_type() const {
    return _type;
}

header::version request::get_version() const {
    return _version;
}

std::string_view request::get_body() const {
    return _body;
}

std::string request::move_body() {
    return std::move(_body);
}

void request::cleanup() {
    _type = {client::request::type::e_Unknown_Type};    
    _version = {header::version::e_Unknown_Version}; 
    _headers.clear();                              
    _body.clear();                                  
    _url.clear();                                   
    _is_gzip_encoded = {false};                    
    _body.clear();
}

std::vector<request::header_data> const &request::get_headers() const {
    return _headers;
}

} // namespace bro::net::http::client
