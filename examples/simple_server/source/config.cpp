#include <fstream>
#include <fmt/color.h>
#include <fmt/core.h>
#include <yaml-cpp/yaml.h>
#include "config.h"
#include <http_client/status.h>
#include <http_client/request.h>

namespace YAML {

template<>
struct convert<bro::net::http::server::simple::config::request_handler> {
    static Node encode(const bro::net::http::server::simple::config::request_handler& rhs) {
        Node node;
        return node;
    }

    static bool decode(const Node& node, bro::net::http::server::simple::config::request_handler& rhs) {

        if (node["path"]) {
            rhs._path = node["path"].as<std::string>();
        }

        if (node["method"]) {
            auto r_type = bro::net::http::client::request::to_type(node["method"].as<std::string>());
            if(r_type == bro::net::http::client::request::type::e_Unknown_Type) {
                fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "incorrect method {}.\n", node["method"].as<std::string>());
                return false;
            }
            rhs._type = r_type;
        }

        if (node["status_code"]) {
            auto s_code = bro::net::http::status::code(node["status_code"].as<std::size_t>());
            if(bro::net::http::status::to_string(s_code) == bro::net::http::status::to_string(bro::net::http::status::code::e_Unknown_Code)) {
                fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "incorrect status code {}.\n", node["status_code"].as<std::string>());
                return false;
            }
            rhs._code = s_code;
        }

        if (node["response_body"]) {
            rhs._response_body = node["response_body"].as<std::string>();
        }

        if (node["response_body_type"]) {
            rhs._response_body_type = node["response_body_type"].as<std::string>();
        }

        if (node["server_name"]) {
            rhs._server_name = node["server_name"].as<std::string>();
        }

        return true;
    }
};
template<>
struct convert<bro::net::http::server::config::http_specific> {
    static Node encode(const bro::net::http::server::config::http_specific& rhs) {
        Node node;
        return node;
    }

    static bool decode(const Node& node, bro::net::http::server::config::http_specific& rhs) {

        if (node["compress_body"]) {
            rhs._compress_body = node["compress_body"].as<bool>();
        }

        if (node["generate_date_in_response"]) {
            rhs._generate_date_in_response = node["generate_date_in_response"].as<bool>();
        }

        if (node["server_name"]) {
            rhs._server_name = node["server_name"].as<std::string>();
        }

        return true;
    }
};

template<>
struct convert<bro::net::http::server::config::listener> {
    static Node encode(const bro::net::http::server::config::listener& rhs) {
        Node node;
        return node;
    }

    static bool decode(const Node& node, bro::net::http::server::config::listener& rhs) {

        if (node["name"]) {
            rhs._name = node["name"].as<std::string>();
        }

        if (node["core_id"]) {
            rhs._core_id = node["core_id"].as<size_t>();
        }

        if (node["sleep"]) {
            rhs._sleep = std::chrono::microseconds(node["sleep"].as<size_t>());
        }

        if (node["call_sleep_on_n_empty_loop_in_a_row"]) {
            rhs._call_sleep_on_n_empty_loop_in_a_row = node["call_sleep_on_n_empty_loop_in_a_row"].as<size_t>();
        }

        if (node["flush_statistic"]) {
            rhs._flush_statistic = std::chrono::milliseconds(node["flush_statistic"].as<size_t>());
        }

        return true;
    }
};

template<>
struct convert<bro::net::http::server::config::handlers> {
    static Node encode(const bro::net::http::server::config::handlers& rhs) {
        Node node;
        return node;
    }

    static bool decode(const Node& node, bro::net::http::server::config::handlers& rhs) {

        if (node["prefix_name"]) {
            rhs._prefix_name = node["prefix_name"].as<std::string>();
        }

        if (node["total"]) {
            rhs._total = node["total"].as<size_t>();
        }

        if (node["sleep"]) {
            rhs._sleep = std::chrono::microseconds(node["sleep"].as<size_t>());
        }

        if (node["call_sleep_on_n_empty_loop_in_a_row"]) {
            rhs._call_sleep_on_n_empty_loop_in_a_row = node["call_sleep_on_n_empty_loop_in_a_row"].as<size_t>();
        }

        if (node["flush_statistic"]) {
            rhs._flush_statistic = std::chrono::milliseconds(node["flush_statistic"].as<size_t>());
        }

        if (node["core_ids"]) {
            auto core_ids = node["core_ids"];
            for(auto it = core_ids.begin(); it != core_ids.end(); ++it) {
                rhs._core_ids.push_back(it->as<size_t>());
            }
        }
        return true;
    }
};


template<>
struct convert<bro::net::http::server::config::logger> {
    static Node encode(const bro::net::http::server::config::logger& rhs) {
        Node node;
        return node;
    }

    static bool decode(const Node& node, bro::net::http::server::config::logger& rhs) {

        if (node["thread_name"]) {
            rhs._thread_name = node["thread_name"].as<std::string>();
        }

        if (node["logger_name"]) {
            rhs._thread_name = node["logger_name"].as<std::string>();
        }

        if (node["core_id"]) {
            rhs._core = node["core_id"].as<size_t>();
        }

        if (node["level"]) {
            rhs._level = quill::loglevel_from_string(node["level"].as<std::string>());
        }
        return true;
    }
};

template<>
struct convert<bro::net::http::server::simple::config::ssl> {
    static Node encode(const bro::net::http::server::simple::config::ssl& rhs) {
        Node node;
        return node;
    }

    static bool decode(const Node& node, bro::net::http::server::simple::config::ssl& rhs) {
        if (node["certificate_path"]) {
            rhs._certificate_path = node["certificate_path"].as<std::string>();
        }

        if (node["key_path"]) {
            rhs._key_path = node["key_path"].as<std::string>();
        }
        return true;
    }
};

template<>
struct convert<bro::net::http::server::simple::config::server> {
    static Node encode(const bro::net::http::server::simple::config::server& rhs) {
        Node node;
        return node;
    }

    static bool decode(const Node& node, bro::net::http::server::simple::config::server& rhs) {

        if (node["test_time"])
            rhs._test_time = std::chrono::seconds(node["test_time"].as<size_t>());

        if (node["address"] && node["port"]) {
            rhs._address = bro::net::proto::ip::full_address(bro::net::proto::ip::address(node["address"].as<std::string>()), node["port"].as<uint16_t>());
            if(rhs._address.get_address().get_version() == bro::net::proto::ip::address::version::e_none) {
                fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "incorrect ip address - {}. couldn't convert it to internal represenation\n", node["address"].as<std::string>());
                return false;
            }
        } else {
            fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, "address and port mandatory fields\n");
            return false;
        }

        if (node["ssl"])
            rhs._ssl = node["ssl"].as<bro::net::http::server::simple::config::ssl>();

        if (node["logger"])
            rhs._server_config._logger = node["logger"].as<bro::net::http::server::config::logger>();

        if (node["handlers"])
            rhs._server_config._handlers = node["handlers"].as<bro::net::http::server::config::handlers>();

        if (node["listener"])
            rhs._server_config._listener = node["listener"].as<bro::net::http::server::config::listener>();

        if (node["http_specific"])
            rhs._server_config._http_specific = node["http_specific"].as<bro::net::http::server::config::http_specific>();

        if (node["request_handlers"]) {
            auto req = node["request_handlers"];
            for(auto it = req.begin(); it != req.end(); ++it) {
                rhs._handlers.push_back(it->as<bro::net::http::server::simple::config::request_handler>());
            }
        }
        return true;
    }
};

} // namespace YAML

namespace bro::net::http::server::simple::config {

std::optional<server> parse(std::string const &path) {
    std::ifstream fin(path);
    if(!fin.is_open()) {
        fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,"couldn't open config file, path to file {}\n", path);
        return {};
    }

    YAML::Node doc;
    try {
        doc = YAML::Load(fin);
    } catch (YAML::ParserException &error) {
        fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold,"config file {}, parsing error {}\n", path, error.msg);
        return {};
    }

    return doc.as<config::server>();
}

} // namespace bro::net::http::server::simple::config
