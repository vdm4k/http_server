#pragma once
#include <http_server/request.h>
#include <http_server/response.h>

namespace bro::net::http::server {

class request_handler {
public:
    /**
      *  \brief A type alias for a callback function that process incoming requests
      */
    using cbt = std::function<void(request &&req, response & res, std::any user_data)>;
    cbt _cb;        ///< request handler
    std::any _data; ///< user datacompress_body
};

} // namespace bro::net::http::server
