#include <userver/storages/etcd/client.hpp>

#include <iostream>

#include <fmt/format.h>

#include <userver/crypto/base64.hpp>
#include <userver/dynamic_config/value.hpp>
#include <userver/formats/json/string_builder.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/logging/log.hpp>
#include <userver/yaml_config/yaml_config.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

namespace {

std::string BuildPutUrl(const std::string& service_url) {
    return fmt::format("{}/v3/kv/put", service_url);
}

std::string BuildPutData(const std::string& key, const std::string& value) {
    formats::json::StringBuilder sb;
    {
        formats::json::StringBuilder::ObjectGuard guard{sb};
        sb.Key("key");
        sb.WriteString(crypto::base64::Base64Encode(key));
        sb.Key("value");
        sb.WriteString(crypto::base64::Base64Encode(value));
    }
    return sb.GetString();
}

std::string BuildRangeUrl(const std::string& service_url) {
    return fmt::format("{}/v3/kv/range", service_url);
}

std::string BuildRangeData(const std::string& key) {
    formats::json::StringBuilder sb;
    {
        formats::json::StringBuilder::ObjectGuard guard{sb};
        sb.Key("key");
        sb.WriteString(crypto::base64::Base64Encode(key));
    }
    return sb.GetString();
}

}

ClientV2Settings Parse(const yaml_config::YamlConfig& value, formats::parse::To<ClientV2Settings>) {
    ClientV2Settings result;
    result.endpoints = value["endpoints"].As<std::vector<std::string>>(result.endpoints);
    return result;
}

ClientV2::ClientV2(clients::http::Client& http_client, ClientV2Settings settings)
    : http_client_(http_client),
    settings_(settings) {
}

void ClientV2::Put(const std::string& key, const std::string& value) {
    const auto service_url = settings_.endpoints.at(0);
    auto request = http_client_
        .CreateRequest()
        .post(BuildPutUrl(service_url), BuildPutData(key, value));
    auto response = request.perform();
}

std::vector<std::string> ClientV2::Range(const std::string& key) {
    const auto service_url = settings_.endpoints.at(0);
    auto request = http_client_
        .CreateRequest()
        .post(BuildRangeUrl(service_url), BuildRangeData(key));
    auto response = request.perform();
    const auto json_body = formats::json::FromString(response->body());
    const auto& key_value_list = json_body["kvs"];
    std::vector<std::string> values;
    values.reserve(key_value_list.GetSize());
    for (const auto& key_value : key_value_list) {
        values.push_back(
            crypto::base64::Base64Decode(key_value["value"].As<std::string>())
        );
    }
    return values;
}

}  // namespace storages::etcd

USERVER_NAMESPACE_END
