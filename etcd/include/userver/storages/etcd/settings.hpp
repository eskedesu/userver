#pragma once

#include <chrono>
#include <string>
#include <vector>

#include <userver/yaml_config/yaml_config.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

struct ClientSettings final {
    const std::vector<std::string> endpoints;
    const std::uint32_t retries;
    const std::chrono::microseconds request_timeout_ms;
};

}

namespace formats::parse {

storages::etcd::ClientSettings Parse(const yaml_config::YamlConfig& value, To<storages::etcd::ClientSettings>);

}

USERVER_NAMESPACE_END
