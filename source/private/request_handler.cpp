#include <http_server/private/request_handler.h>
#include "quill/detail/LogMacros.h"

namespace bro::net::http::server {


int request_handler::on_url(llhttp_t *parser, char const *at, size_t length) {
    request_handler *req = (request_handler *) parser->data;
    req->_request._type = client::request::to_type({at, length});
    return 0;
}

int request_handler::on_method(llhttp_t *parser, char const *at, size_t length) {
    request_handler *req = (request_handler *) parser->data;
    req->_request._type = client::request::to_type({at, length});
    return 0;
}

void request_handler::decoded_data(Bytef *data, size_t lenght, std::any user_data, char const *error) {
    request_handler *req = std::any_cast<request_handler *>(user_data);
    if (error) {
        LOG_ERROR(req->_logger, "Error while decode message {}", error);
    } else {
        req->_request._body.append((char const *) data, lenght);
    }
}

int request_handler::on_body(llhttp_t *parser, char const *at, size_t length) {
    request_handler *req = (request_handler *) parser->data;
    if (req->_request._is_gzip_encoded) {
        req->_zstream.process((Bytef *) at, length, req, decoded_data);
    } else
        req->_request._body.append(at, length);
    return 0;
}

int request_handler::on_version(llhttp_t *parser, char const *at, size_t length) {
    request_handler *req = (request_handler *) parser->data;
    req->_request._version = header::to_version({at, length});
    return 0;
}

int request_handler::on_header_field(llhttp_t *parser, char const *at, size_t length) {
    request_handler *req = (request_handler *) parser->data;
    request::header_data hdr;
    hdr._type.append(at, length);
    req->_request._headers.push_back(hdr);
    return 0;
}

int request_handler::on_header_value(llhttp_t *parser, char const *at, size_t length) {
    request_handler *req = (request_handler *) parser->data;
    if (req->_request._headers.empty())
        return 0;
    auto &hdr = req->_request._headers.back();
    hdr._value.append(at, length);
    if (hdr._type == header::to_string(header::types::e_Content_Encoding))
        req->_request._is_gzip_encoded = hdr._value.find("gzip") != std::string::npos
                                         && req->_zstream.init(bro::zlib::stream::type::e_decompressor);

    return 0;
}

int request_handler::handle_on_message_complete(llhttp_t *h) {
    if (!h->data)
        return -1;
    //        server *req = (server *) h->data;
    //        req->_result._cb(std::move(res), nullptr, req->_result._data);
    //        req->cleanup();
    return 0;
}

void request_handler::init_parser() {
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
}

} // namespace bro::net::http::server
