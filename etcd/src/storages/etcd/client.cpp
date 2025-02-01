#include <cstddef>
#include <memory>
#include <userver/storages/etcd/client.hpp>

#include <iostream>
#include <string>

#include <fmt/format.h>

#include <userver/clients/http/streamed_response.hpp>
#include <userver/http/status_code.hpp>
#include <userver/concurrent/queue.hpp>
#include <userver/crypto/base64.hpp>
#include <userver/dynamic_config/value.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/json/string_builder.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/logging/log.hpp>
#include <userver/utils/rand.hpp>
#include <userver/yaml_config/yaml_config.hpp>
#include <userver/utils/async.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

namespace {

std::string BuildPutUrl(const std::string& service_url) {
    return fmt::format("{}/v3/kv/put", service_url);
}

std::string BuildPutData(const std::string& key, const std::string& value) {
    formats::json::ValueBuilder builder;
    builder["key"] = crypto::base64::Base64Encode(key);
    builder["value"] = crypto::base64::Base64Encode(value);
    return formats::json::ToString(builder.ExtractValue());
}

std::string BuildRangeUrl(const std::string& service_url) {
    return fmt::format("{}/v3/kv/range", service_url);
}

std::string BuildRangeData(const std::string& key) {
    formats::json::ValueBuilder builder;
    builder["key"] = crypto::base64::Base64Encode(key);
    return formats::json::ToString(builder.ExtractValue());
}

std::string BuildDeleteRangeUrl(const std::string& service_url) {
    return fmt::format("{}/v3/kv/deleterange", service_url);
}

std::string BuildDeleteRangeData(const std::string& key) {
    formats::json::ValueBuilder builder;
    builder["key"] = crypto::base64::Base64Encode(key);
    return formats::json::ToString(builder.ExtractValue());
}


std::string BuildWatchUrl(const std::string& service_url) {
    return fmt::format("{}/v3/watch", service_url);
}

std::string BuildWatchData(const std::string& key) {
    formats::json::ValueBuilder builder;
    builder["create_request"]["key"] = crypto::base64::Base64Encode(key);
    return formats::json::ToString(builder.ExtractValue());
}

bool ShouldRetry(const http::StatusCode status_code) {
    return false;
}

}

namespace impl {

ClientImpl::ClientImpl(clients::http::Client& http_client, ClientSettings settings)
    : http_client_(http_client),
    settings_(settings) {
}

void ClientImpl::Put(const std::string& key, const std::string& value) {
    auto response = PerformEtcdRequest(BuildPutUrl, BuildPutData(key, value));
}

std::vector<std::string> ClientImpl::Range(const std::string& key) {
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

void ClientImpl::DeleteRange(const std::string& key) {
    auto response = PerformEtcdRequest(BuildDeleteRangeUrl, BuildDeleteRangeData(key));
}

clients::http::StreamedResponse ClientImpl::PerformStreamEtcdRequest(
    const std::function<std::string(const std::string&)>& url_builder, const std::string& data
){
    auto endpoints = settings_.endpoints;
    utils::Shuffle(endpoints);
    
    for (const auto& endpoint : endpoints) {
        const auto queue = concurrent::StringStreamQueue::Create();
        auto stream_response = http_client_
                .CreateRequest()
                .post(url_builder(endpoint), data)
                .retry(settings_.retries)
                .timeout(1'000'000'000)
                .async_perform_stream_body(queue);
        if (!ShouldRetry(stream_response.StatusCode())) {
            return stream_response;
        }
    }
}

void ClientImpl::StartWatch(const std::string& key) {

    const auto data = BuildWatchData(key);
    auto stream_response = PerformStreamEtcdRequest(BuildWatchUrl, BuildWatchData(key));

    watch_task_ = utils::Async(
        "watch task",
        [stream_response = std::move(stream_response)] mutable {
            std::string body_part;
            std::string result;
            const auto deadline = engine::Deadline::FromDuration(std::chrono::seconds{100'000'000});
            while (stream_response.ReadChunk(body_part, deadline)) {
                LOG_ERROR() << "Kek   " << body_part;
                result += body_part;
            } 
        }
    );
}

std::shared_ptr<clients::http::Response> ClientImpl::PerformEtcdRequest(
    const std::function<std::string(const std::string&)>& url_builder, const std::string& data
) {
    auto endpoints = settings_.endpoints;
    utils::Shuffle(endpoints);

    for (const auto& endpoint : endpoints) {
        auto response = http_client_
            .CreateRequest()
            .post(url_builder(endpoint), data)
            .retry(settings_.retries)
            .timeout(settings_.request_timeout_ms.count())
            .perform();
        LOG_DEBUG() << "Response: " << formats::json::FromString(response->body());
        if (!ShouldRetry(response->status_code())) {
            return response;
        }
    }
}

}

}  // namespace storages::etcd

USERVER_NAMESPACE_END
