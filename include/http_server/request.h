#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <http_client/status.h>
#include <http_client/version.h>
#include <http_client/request.h>


namespace bro::net::http::server {

/*!\ request to server
 */
class request {
public:
    /**
 * \brief header description
 */
    struct header_data {
        std::string _type;  ///<  header type
        std::string _value; ///< header value
    };

    /*! \brief get status code
  * \result status code
  */
    client::request::type get_type() const;

    /*! \brief get verson of http
  * \result version
  */
    header::version get_version() const;

    /*! \brief get body
  * \result body
  */
    std::string_view get_body() const;

    /*! \brief get headers
  * \result reference on headers
  */
    std::vector<header_data> const &get_headers() const;

private:
    friend class request_handler;

    client::request::type _type{client::request::type::e_Unknown_Type};      ///< status code
    header::version _version{header::version::e_Unknown_Version}; ///< http version
    std::vector<header_data> _headers;                            ///< array of headers
    std::string _body;                                            ///< body ( uncompressed if gzip )
    bool _is_gzip_encoded{false};                                 ///< is commpressed by gzip
    bool _set_error = false;
};

} // namespace bro::net::http::client
