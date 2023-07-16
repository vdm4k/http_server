#include "http_client/request.h"
#include <http_server/request.h>
#include <http_server/http_server.h>
#include <system/thread/thread.h>
#include "quill/Quill.h"


int main() {
    quill::Config cfg;
    cfg.enable_console_colours = true;
    quill::configure(cfg);
    quill::start();

//    quill::Logger* logger = quill::get_logger();
//    logger->set_log_level(quill::LogLevel::TraceL3);

//    // enable a backtrace that will get flushed when we log CRITICAL
//    logger->init_backtrace(2, quill::LogLevel::Critical);

//    LOG_BACKTRACE(logger, "Backtrace log {}", 1);
//    LOG_BACKTRACE(logger, "Backtrace log {}", 2);

//    LOG_INFO(logger, "Welcome to Quill!");
//    LOG_ERROR(logger, "An error message. error code {}", 123);
//    LOG_WARNING(logger, "A warning message.");
//    LOG_CRITICAL(logger, "A critical error.");
//    LOG_DEBUG(logger, "Debugging foo {}", 1234);
//    LOG_TRACE_L1(logger, "{:>30}", "right aligned");
//    LOG_TRACE_L2(logger, "Positional arguments are {1} {0} ", "too", "supported");
//    LOG_TRACE_L3(logger, "Support for floats {:03.2f}", 1.23456);

    using namespace bro::net::http;
    server::http_server server({._bind_addr{bro::net::proto::ip::address{"127.0.0.1"}, 22345},._connection_type = server::connection_type::e_http,._server_name = "nginx/1.14.0 (Ubuntu)"});
    server.add_handler(client::request::type::e_GET, "/", {._cb = [](server::request &&req, server::response &srv, std::any user_data) {
                        //srv.
                    }});
    server.start();
    std::this_thread::sleep_for(std::chrono::seconds(1000));
    server.stop();
    return 0;
}
