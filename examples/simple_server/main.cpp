//#include "http_client/request.h"
//#include <http_server/request.h>
//#include <http_server/http_server.h>
//#include <system/thread/thread.h>
#include <quill/Quill.h>
#include <CLI/CLI.hpp>
#include <network/platforms/system.h>
#include <http_server/http_server.h>

void print_request(bro::net::http::server::request &req) {
    std::cout << "---------- HTTP Request ----------\n";
    std::cout << "request type: " << bro::net::http::client::request::to_string(req.get_type()) << "\n";
    std::cout << "url: " << req.get_url() << "\n";
    std::cout << "version: " << bro::net::http::header::to_string(req.get_version()) << "\n";
    std::cout << "-------headers-------\n";
    for (auto const &hdr : req.get_headers()) {
        std::cout << hdr._type << ": " << hdr._value << "\n";
    }
    std::cout << "-------body-------\n";
    std::cout << req.get_body() << "\n";
}


int main(int argc, char **argv) {
    CLI::App app{"ssl_server"};
    std::string server_address_s = "127.0.0.1";
    uint16_t server_port = 22345;
    size_t test_time = 1000; // in seconds
    std::string certificate_path{"/home/vdm4k/projects/http_server/cert.pem"};
    std::string key_path{"/home/vdm4k/projects/http_server/key.pem"};
    std::string handler_path{"/"};
    std::string request_type{"GET"};
    std::string server_name{"awesome_server/1.0.0"};
    bool use_https = true;

    app.add_option("-a,--address", server_address_s, "server address");
    app.add_option("-p,--port", server_port, "server port");
    app.add_option("-t,--test_time", test_time, "test time in seconds");
    app.add_option("-c,--certificate_path", certificate_path, "certificate path");
    app.add_option("-k,--key_path", key_path, "key path");
    app.add_option("-o,--handler_path", handler_path, "handler path");
    app.add_option("-r,--request_type", request_type, "request type");
    app.add_option("-s,--use_https", use_https, "use https");
    app.add_option("-n,--server_name", server_name, "server name");
    CLI11_PARSE(app, argc, argv);

    using namespace bro::net;

    disable_sig_pipe();

    auto r_type = http::client::request::to_type(request_type);
    if(r_type == http::client::request::type::e_Unknown_Type) {
        std::cerr << "unknown request type - " << request_type << std::endl;
        return 0;
    }

    auto con_type = use_https ? http::server::connection_type::e_https : http::server::connection_type::e_http;

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


    tcp::ssl::listen::settings config;
    config._listen_address = {bro::net::proto::ip::address{server_address_s}, server_port};
    config._certificate_path = certificate_path;
    config._key_path = key_path;
    http::server::http_server server({._settings = config,
                                      ._connection_type = con_type,
                                      ._server_name = server_name,
                                      ._generate_date_in_response = true,
                                      });
    server.add_handler(r_type, handler_path, {._cb = [](http::server::request &&req, http::server::response &resp, std::any user_data) {
        print_request(req);
        resp.set_status_code(http::status::code::e_OK);
        resp.add_body(std::string_view(R"(<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
                                            "http://www.w3.org/TR/html4/strict.dtd">
<html>
    <head>
        <meta http-equiv="Content-Type" content="text/html;charset=utf-8">
        <title>Message to better you</title>
    </head>
    <body>
        <h1>You are looking good</h1>
        <p>How are you doing?</p>
    </body>
</html>")"), std::string("text/html;charset=utf-8"));
    }});
    if(!server.start()) {
        std::cerr << "couldn't start http server" << std::endl;
        return 0;
    }
    std::this_thread::sleep_for(std::chrono::seconds(test_time));
    server.stop();
    return 0;
}
