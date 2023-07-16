#include <http_server/private/request_parser.h>
#include "fmt/printf.h"
#include "http_server/request.h"

namespace bro::net::http::server::private_ {


bool request_parser::init(result_fun_t result_fun, std::any user_data) {
    /* Initialize user callbacks and settings */
    llhttp_settings_init(&_parser_settings);
    /* Set user callback */
    _parser_settings.on_message_complete = handle_on_message_complete;
    _parser_settings.on_url = on_url;
    _parser_settings.on_method = on_method;
    _parser_settings.on_body = on_body;
    _parser_settings.on_version = on_version;
    _parser_settings.on_header_field = on_header_field;
    _parser_settings.on_header_value = on_header_value;
    llhttp_init(&_parser, HTTP_REQUEST, &_parser_settings);
    _parser.data = this;
    _result_fun = result_fun;
    _user_data = user_data;
    return true;
}

bool request_parser::process_data(std::byte *buffer, size_t buffer_size) {
    auto err = llhttp_execute(&_parser, (char *)buffer, buffer_size);
    if (err != HPE_OK) {
        std::string error = fmt::sprintf("Parse error: %s %s", llhttp_errno_name(err), _parser.reason);
        _result_fun(_request, _user_data, error.c_str());
        return false;
    }
    return true;
}

int request_parser::on_url(llhttp_t *parser, char const *at, size_t length) {
    request_parser *req = (request_parser *) parser->data;
    req->_request._url.append((char const *) at, length);
    return 0;
}

int request_parser::on_method(llhttp_t *parser, char const *at, size_t length) {
    request_parser *req = (request_parser *) parser->data;
    req->_request._type = client::request::to_type({at, length});
    if (req->_request._type == client::request::type::e_Unknown_Type) {
        std::string error = fmt::sprintf("Unexpected request type {}", std::string(at, length));
        req->_result_fun(req->_request, req->_user_data, error.c_str());
    }
    return 0;
}

void request_parser::decoded_data(Bytef *data, size_t lenght, std::any user_data, char const *error) {
    request_parser *req = std::any_cast<request_parser *>(user_data);
    if (error) {
        std::string err = fmt::sprintf("Error while decode message {}", error);
        req->_result_fun(req->_request, req->_user_data, err.c_str());
    } else {
        req->_request._body.append((char const *) data, lenght);
    }
}

int request_parser::on_body(llhttp_t *parser, char const *at, size_t length) {
    request_parser *req = (request_parser *) parser->data;
    if (req->_request._is_gzip_encoded) {
        req->_zstream.process((Bytef *) at, length, req, decoded_data);
    } else
        req->_request._body.append(at, length);
    return 0;
}

int request_parser::on_version(llhttp_t *parser, char const *at, size_t length) {
    request_parser *req = (request_parser *) parser->data;
    req->_request._version = header::to_version({at, length});
    if (req->_request._version == header::version::e_Unknown_Version) {
        std::string error = fmt::sprintf("Unexpected version field {}", std::string(at, length));
        req->_result_fun(req->_request, req->_user_data, error.c_str());
    }
    return 0;
}

int request_parser::on_header_field(llhttp_t *parser, char const *at, size_t length) {
    request_parser *req = (request_parser *) parser->data;
    request::header_data hdr;
    hdr._type.append(at, length);
    req->_request._headers.push_back(hdr);
    return 0;
}

int request_parser::on_header_value(llhttp_t *parser, char const *at, size_t length) {
    request_parser *req = (request_parser *) parser->data;
    if (req->_request._headers.empty())
        return 0;
    auto &hdr = req->_request._headers.back();
    hdr._value.append(at, length);
    if (hdr._type == header::to_string(header::types::e_Content_Encoding))
        req->_request._is_gzip_encoded = hdr._value.find("gzip") != std::string::npos
                                         && req->_zstream.init(bro::zlib::stream::type::e_decompressor);

    return 0;
}

int request_parser::handle_on_message_complete(llhttp_t *h) {
    if (!h->data)
        return -1;
    request_parser *req = (request_parser *)h->data;
    req->_result_fun(req->_request, req->_user_data, nullptr);
    return 0;
}

} // namespace bro::net::http::server::private_
