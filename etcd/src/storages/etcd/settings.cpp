#include <userver/storages/etcd/settings.hpp>

#include <userver/dynamic_config/value.hpp>
#include <userver/formats/parse/common_containers.hpp>

USERVER_NAMESPACE_BEGIN


namespace storages::etcd {

namespace {

constexpr std::uint32_t kDefaultRetries{3};
constexpr std::chrono::milliseconds kDefaultRequestTimeout{1'000};

}

}

namespace formats::parse {

storages::etcd::ClientSettings Parse(const yaml_config::YamlConfig& cofig, To<storages::etcd::ClientSettings>) {
    return storages::etcd::ClientSettings {
        .endpoints = cofig["endpoints"].As<std::vector<std::string>>(),
        .retries = cofig["retries"].As<std::uint32_t>(storages::etcd::kDefaultRetries),
        .request_timeout_ms = cofig["request_timeout_ms"].As<std::chrono::milliseconds>(storages::etcd::kDefaultRequestTimeout),
    };
}

}  // namespace formats::parse

USERVER_NAMESPACE_END
