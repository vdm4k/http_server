#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <http_client/status.h>
#include <http_client/version.h>
#include <http_client/request.h>

namespace bro::net::http::server {

/*!\ response from server (fill up in request_handler)
 */
class response {
public:

   /*! \brief set status code
   * \param code status code
   */
    void set_status_code(status::code code);

   /*! \brief get status code
   * \result status code
   */
    status::code get_status_code() const noexcept {
        return _status_code;
    }

   /*! \brief add headers
   * \param [in] type header type
   * \param [in] value header value
   */
    void add_header(std::string_view const &type, std::string const &value);

   /*! \brief add headers
   * \param [in] type header type
   * \param [in] value header value
   */
    void add_header(std::string const &type, std::string const &value);

   /*! \brief add body
   * \param [in] body body value
   * \param [in] content_type content type
   */
    void add_body(std::string const &body, std::string const &content_type);

   /*! \brief add body
   * \param [in] body body value
   * \param [in] content_type content type
   */
    void add_body(std::string_view const &body, const std::string &content_type);

    /**
     * \brief Get the http headers
     * 
     * \return std::vector<std::pair<std::string_view, std::string>> const& array of headers
     */
    std::vector<std::pair<std::string_view, std::string>> const & get_headers_v() const {
        return _headers_v;
    }

    /**
     * \brief et the http headers
     * 
     * \return std::vector<std::pair<std::string, std::string>> const& array of headers
     */
    std::vector<std::pair<std::string, std::string>> const & get_headers_s() const {
        return _headers_s;
    }

    /**
     * \brief Get the http body
     * 
     * \return std::string_view body 
     */
    std::string_view get_body_v() const noexcept {
        return _body_v;
    }

    /**
     * \brief Get the http body
     * 
     * \return std::string const& body
     */
    std::string const & get_body_s() const noexcept {
        return _body_s;
    }

    /**
     * \brief Get the total size of set headers
     * 
     * \return size_t all set headers size
     */
    size_t get_header_total_size() const noexcept {
        return _header_total_size;
    }

private:

    status::code _status_code{status::code::e_Unknown_Code};                ///< status code
    size_t _header_total_size = 0;                                          ///< total header size
    std::vector<std::pair<std::string_view, std::string>> _headers_v;       ///< headers to set in request
    std::vector<std::pair<std::string, std::string>> _headers_s;            ///< headers to set in request
    std::string _body_s;                                                    ///< body to set in request
    std::string_view _body_v;                                               ///< body to set in request as string view
};

} // namespace bro::net::http::client
