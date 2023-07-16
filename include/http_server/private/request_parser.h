#pragma once
#include "http_server/request.h"
#include <llhttp.h>
#include <http_client/zlib_helper.h>

namespace bro::net::http::server::private_ {

class request_parser {
public:

    /**
  * \brief A type alias for a callback function that returns a result.
  *
  * This function will be called with filled data and set lenght ( > 0) or filled error.
  * For pointers no need to call free
  */
    using result_fun_t = void (*)(request &req, std::any user_data, char const *error);

    bool init(result_fun_t result_fun, std::any user_data);
    bool process_data(std::byte *buffer, size_t buffer_size);
private:

    static int on_url(llhttp_t *parser, char const *at, size_t length) ;
    static int on_method(llhttp_t *parser, char const *at, size_t length);
    static void decoded_data(Bytef *data, size_t lenght, std::any user_data, char const *error);
    static int on_body(llhttp_t *parser, char const *at, size_t length);
    static int on_version(llhttp_t *parser, char const *at, size_t length);
    static int on_header_field(llhttp_t *parser, char const *at, size_t length);
    static int on_header_value(llhttp_t *parser, char const *at, size_t length);
    static int handle_on_message_complete(llhttp_t *h);

    llhttp_t _parser;                                                      ///< http parser
    llhttp_settings_t _parser_settings{};                                  ///< settings for parser http
    zlib::stream _zstream;                                                 ///< zlib decoder
    request _request;
    result_fun_t _result_fun;
    std::any _user_data;
};

} // namespace bro::net::http::server::private_
