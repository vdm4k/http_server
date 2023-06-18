#include "quill/Logger.h"
#include <http_server/request.h>

namespace bro::net::http::server {


class request_handler {
public:

    void init_parser();
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
    request _request;
    zlib::stream _zstream;                                                 ///< zlib decoder
    quill::Logger* _logger = nullptr;
};

} // namespace bro::net::http::server
