#pragma once

#include <chrono>
#include <string>
#include <vector>

#include <userver/yaml_config/yaml_config.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

struct ClientV2Settings final {
    std::vector<std::string> endpoints;
    std::uint32_t retries;
    std::chrono::microseconds request_timeout_ms;
};

}

namespace formats::parse {

storages::etcd::ClientV2Settings Parse(const yaml_config::YamlConfig& value, To<storages::etcd::ClientV2Settings>);

}

USERVER_NAMESPACE_END
