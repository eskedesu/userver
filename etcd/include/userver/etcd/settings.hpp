#pragma once

#include <chrono>
#include <string>
#include <vector>

#include <userver/yaml_config/yaml_config.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

struct ClientSettings final {
    const std::vector<std::string> endpoints;
    const std::uint32_t attempts;
    const std::chrono::microseconds request_timeout_ms;
};

}  // namespace etcd

namespace formats::parse {

etcd::ClientSettings Parse(const yaml_config::YamlConfig& value, To<etcd::ClientSettings>);

}

USERVER_NAMESPACE_END
