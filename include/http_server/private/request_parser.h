#pragma once
#include "http_server/request.h"
#include <llhttp.h>
#include <http_client/zlib_helper.h>

namespace bro::net::http::server::private_ {

/**
 * \brief http request parser
 */
class request_parser {
public:

    /**
  * \brief A type alias for a callback function that returns a result or error.
  */
    using result_fun_t = void (*)(request &req, std::any user_data, char const *error);

    /**
     * \brief initialize request parser (can be called many times)
     * 
     * \param result_fun function to call for success or fail
     * \param user_data user data
     * \return true if successfully inited
     * \return false otherwise
     */
    bool init(result_fun_t result_fun, std::any user_data);

    /**
     * \brief process raw data
     * 
     * \param buffer pointer on raw buffer
     * \param buffer_size buffer size
     * \return true if data handled successfully
     * \return false otherwise
     */
    bool process_data(std::byte *buffer, size_t buffer_size);
private:

    /** \brief callback for http parser (see @llhttp_data_cb) */
    static int on_url(llhttp_t *parser, char const *at, size_t length) ;
    /** \brief callback for http parser (see @llhttp_data_cb) */
    static int on_method(llhttp_t *parser, char const *at, size_t length);
    /** \brief callback for http parser (see @llhttp_data_cb) */
    static void decoded_data(Bytef *data, size_t lenght, std::any user_data, char const *error);
    /** \brief callback for http parser (see @llhttp_data_cb) */
    static int on_body(llhttp_t *parser, char const *at, size_t length);
    /** \brief callback for http parser (see @llhttp_data_cb) */
    static int on_version(llhttp_t *parser, char const *at, size_t length);
    /** \brief callback for http parser (see @llhttp_data_cb) */
    static int on_header_field(llhttp_t *parser, char const *at, size_t length);
    /** \brief callback for http parser (see @llhttp_data_cb) */
    static int on_header_value(llhttp_t *parser, char const *at, size_t length);
    /** \brief callback for http parser (see @llhttp_data_cb) */
    static int handle_on_message_complete(llhttp_t *h);

    llhttp_t _parser;                                                      ///< http parser
    llhttp_settings_t _parser_settings{};                                  ///< settings for parser http
    zlib::stream _zstream;                                                 ///< zlib decoder
    request _request;                                                      ///< request to fill
    result_fun_t _result_fun;                                              ///< result function callback 
    std::any _user_data;                                                   ///< user data pass in @_result_function
};

} // namespace bro::net::http::server::private_
