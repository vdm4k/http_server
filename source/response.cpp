#include <http_server/response.h>
#include <http_client/common.h>

namespace bro::net::http::server {

void response::set_status_code(status::code code) {
    _status_code = code;
}

void response::add_header(std::string_view const &type, std::string const &value) {
    _header_total_size += value.size() + type.size() + client::e_header_add_size;
    _headers_v.push_back({type, value});
}

void response::add_header(std::string const &type, std::string const &value) {
    _header_total_size += value.size() + type.size() + client::e_header_add_size;
    _headers_s.push_back({type, value});
}

void response::add_body(std::string const &body, const std::string &content_type) {
    add_header(header::to_string(header::types::e_Content_Type), content_type);
    _body_s = body;
}

void response::add_body(std::string_view const &body, const std::string &content_type) {
    add_header(header::to_string(header::types::e_Content_Type), content_type);
    _body_v = body;
}

}
