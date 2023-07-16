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

std::vector<request::header_data> const &request::get_headers() const {
    return _headers;
}

} // namespace bro::net::http::client
