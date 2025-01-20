#include <userver/storages/etcd/client.hpp>

#include <iostream>
#include <string>

#include <fmt/format.h>

#include <userver/crypto/base64.hpp>
#include <userver/dynamic_config/value.hpp>
#include <userver/formats/json/string_builder.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/logging/log.hpp>
#include <userver/utils/rand.hpp>
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

std::string BuildDeleteRangeUrl(const std::string& service_url) {
    return fmt::format("{}/v3/kv/deleterange", service_url);
}

std::string BuildDeleteRangeData(const std::string& key) {
    formats::json::StringBuilder sb;
    {
        formats::json::StringBuilder::ObjectGuard guard{sb};
        sb.Key("key");
        sb.WriteString(crypto::base64::Base64Encode(key));
    }
    return sb.GetString();
}

bool ShouldRetry(std::shared_ptr<clients::http::Response> response) {
    return false;
}

}

ClientV2::ClientV2(clients::http::Client& http_client, ClientV2Settings settings)
    : http_client_(http_client),
    settings_(settings) {
}

void ClientV2::Put(const std::string& key, const std::string& value) {
    auto response = PerformEtcdRequest(BuildPutUrl, BuildPutData(key, value));
}

std::vector<std::string> ClientV2::Range(const std::string& key) {
    auto response = PerformEtcdRequest(BuildRangeUrl, BuildRangeData(key));

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

void ClientV2::DeleteRange(const std::string& key) {
    auto response = PerformEtcdRequest(BuildDeleteRangeUrl, BuildDeleteRangeData(key));
}

std::shared_ptr<clients::http::Response> ClientV2::PerformEtcdRequest(
    const std::function<std::string(const std::string&)>& url_builder, const std::string& data
) {
    endpoints_shared_mutex_.lock_shared();
    auto endpoints = settings_.endpoints;
    endpoints_shared_mutex_.unlock_shared();
    utils::Shuffle(endpoints);

    for (const auto& endpoint : endpoints) {
        auto response = http_client_
            .CreateRequest()
            .post(url_builder(endpoint), data)
            .retry(settings_.retries)
            .timeout(settings_.request_timeout_ms.count())
            .perform();
        LOG_DEBUG() << "Response: " << formats::json::FromString(response->body());
        if (!ShouldRetry(response)) {
            return response;
        }
    }
}

}  // namespace storages::etcd

USERVER_NAMESPACE_END
