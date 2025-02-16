#include <userver/etcd/settings.hpp>

#include <userver/dynamic_config/value.hpp>
#include <userver/formats/parse/common_containers.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

namespace {

constexpr std::uint32_t kDefaultAttempts{3};
constexpr std::chrono::milliseconds kDefaultRequestTimeout{1'000};

}  // namespace

}  // namespace etcd

namespace formats::parse {

etcd::ClientSettings Parse(const yaml_config::YamlConfig& cofig, To<etcd::ClientSettings>) {
    return etcd::ClientSettings{
        .endpoints = cofig["endpoints"].As<std::vector<std::string>>(),
        .attempts = cofig["attempts"].As<std::uint32_t>(etcd::kDefaultAttempts),
        .request_timeout_ms = cofig["request_timeout_ms"].As<std::chrono::milliseconds>(etcd::kDefaultRequestTimeout),
    };
}

}  // namespace formats::parse

USERVER_NAMESPACE_END
