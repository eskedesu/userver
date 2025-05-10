#include <userver/etcd/settings.hpp>

#include <userver/dynamic_config/value.hpp>
#include <userver/formats/parse/common_containers.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

namespace {

constexpr std::uint32_t kDefaultAttempts{3};
constexpr std::chrono::milliseconds kDefaultRequestTimeout{1'000};
constexpr std::chrono::milliseconds kDefaultWatchTimeout{1'000'000};

}  // namespace

}  // namespace etcd

namespace formats::parse {

etcd::ClientSettings Parse(const yaml_config::YamlConfig& config, To<etcd::ClientSettings>) {
    return etcd::ClientSettings{
        /* .endpoints = */ config["endpoints"].As<std::vector<std::string>>(),
        /* .attempts = */ config["attempts"].As<std::uint32_t>(etcd::kDefaultAttempts),
        /* .request_timeout_ms = */ config["request_timeout_ms"].As<std::chrono::milliseconds>(etcd::kDefaultRequestTimeout),
        /* .watch_timeout_ms = */ config["watch_timeout_ms"].As<std::chrono::milliseconds>(etcd::kDefaultWatchTimeout),
    };
}

}  // namespace formats::parse

USERVER_NAMESPACE_END
