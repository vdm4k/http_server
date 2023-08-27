#include <thread>
#include <signal.h>
#include "fmt/color.h"
#include "fmt/core.h"
#include <CLI/CLI.hpp>
#include <network/platforms/system.h>
#include <http_server/http_server.h>
#include <network/tcp/ssl/listen/settings.h>
#include "config.h"


static std::atomic_bool s_active{true};
void stop_signal_h(int s){
    s_active = false;
}

void print_request(bro::net::http::server::request &req) {
    return;
    fmt::print(fg(fmt::color::dark_green) | fmt::emphasis::bold,"---------- HTTP Response ----------\n");
    fmt::print("statuc code: {}\n", bro::net::http::client::request::to_string(req.get_type()));
    fmt::print("version: {}\n", bro::net::http::header::to_string(req.get_version()));
    fmt::print("---------- headers ----------\n");
    for (auto const &hdr : req.get_headers()) {
        fmt::print("{}: {}\n", hdr._type, hdr._value);
    }
    fmt::print("---------- body ----------\n{}\n", req.get_body());
}


int main(int argc, char **argv) {
    CLI::App app{"http_simple_server"};
    std::string config_path{"/home/vdm4k/projects/http_server/examples/simple_server/config/config.yaml"};
    app.add_option("-c,--config", config_path, "path to config file");
   // CLI11_PARSE(app, argc, argv);

    auto config = bro::net::http::server::simple::config::parse(config_path);
    if(!config)
        return -1;

    using namespace bro::net;

    disable_sig_pipe();

    std::unique_ptr<bro::net::listen::settings> connection_settings;
    if(config->_ssl) {
        tcp::ssl::listen::settings * settings = new tcp::ssl::listen::settings;
        settings->_listen_address = config->_address;
        settings->_certificate_path = config->_ssl->_certificate_path;
        settings->_key_path = config->_ssl->_key_path;
        connection_settings.reset(settings);
    } else {
        tcp::listen::settings * settings = new tcp::listen::settings;
        settings->_listen_address = config->_address;
        connection_settings.reset(settings);
    }

    http::server::http_server server(std::move(connection_settings), std::move(config->_server_config));
    for(auto const & handler : config->_handlers) {
        server.add_handler(handler._type, handler._path, {._cb = [&handler](http::server::request &&req, http::server::response &resp, std::any user_data) {
            print_request(req);
            resp.set_status_code(handler._code);
            if(!handler._response_body.empty()) {
                resp.add_body(handler._response_body, handler._response_body_type);
            }
        }});
    }

    if(!server.start()) {
        std::cerr << "couldn't start http server" << std::endl;
        return 0;
    }

    if(config->_test_time) {
        std::this_thread::sleep_for(std::chrono::seconds(*config->_test_time));
    } else {
        struct sigaction act{};
        act.sa_handler = stop_signal_h;
        sigaction(SIGINT, &act, NULL);
        while(s_active.load(std::memory_order_acquire))
            std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    server.stop();
    return 0;
}
