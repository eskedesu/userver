#include <etcd/client_impl.hpp>

#include <iostream>
#include <memory>
#include <string>

#include <fmt/format.h>

#include <userver/clients/http/streamed_response.hpp>
#include <userver/crypto/base64.hpp>
#include <userver/dynamic_config/value.hpp>
#include <userver/engine/sleep.hpp>
#include <userver/etcd/exceptions.hpp>
#include <userver/formats/json/inline.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/formats/parse/common_containers.hpp>
#include <userver/http/common_headers.hpp>
#include <userver/http/status_code.hpp>
#include <userver/logging/log.hpp>
#include <userver/utils/async.hpp>
#include <userver/utils/rand.hpp>
#include <userver/yaml_config/yaml_config.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

namespace {

const std::uint32_t kMinRetryStatusCode = 500;
const std::uint32_t kMaxRetryStatusCode = 599;

const std::uint32_t kMinGoodStatusCode = 200;
const std::uint32_t kMaxGoodStatusCode = 299;

const std::string kKeyPrefix = "/etcd/";
const std::string kLastPossibleKeyPrefix = "/etcd0";

std::string BuildPutUrl(const std::string& service_url) { return fmt::format("{}/v3/kv/put", service_url); }

std::string BuildPutData(const std::string& key, const std::string& value) {
    const auto etcd_key = kKeyPrefix + key;
    return formats::json::ToString(formats::json::MakeObject(
        "key", crypto::base64::Base64Encode(etcd_key), "value", crypto::base64::Base64Encode(value)
    ));
}

std::string BuildRangeUrl(const std::string& service_url) { return fmt::format("{}/v3/kv/range", service_url); }

std::string BuildRangeData(const std::string& key, const std::optional<std::string>& maybe_range_end = std::nullopt) {
    const auto etcd_key = kKeyPrefix + key;
    if (!maybe_range_end.has_value()) {
        return formats::json::ToString(formats::json::MakeObject(
            "key", crypto::base64::Base64Encode(etcd_key)
        ));    
    }
    return formats::json::ToString(formats::json::MakeObject(
        "key", crypto::base64::Base64Encode(etcd_key), "range_end", crypto::base64::Base64Encode(maybe_range_end)
    ));
}

std::string BuildDeleteUrl(const std::string& service_url) { return fmt::format("{}/v3/kv/deleterange", service_url); }

std::string BuildDeleteData(const std::string& key) {
    const auto etcd_key = kKeyPrefix + key;
    return formats::json::ToString(formats::json::MakeObject("key", crypto::base64::Base64Encode(etcd_key)));
}

std::string BuildWatchUrl(const std::string& service_url) { return fmt::format("{}/v3/watch", service_url); }

std::string BuildWatchData(const std::string& key) {
    const auto etcd_key = kKeyPrefix + key;
    return formats::json::ToString(formats::json::MakeObject(
        "create_request", formats::json::MakeObject("key", crypto::base64::Base64Encode(etcd_key))
    ));
}

bool ShouldRetry(const http::StatusCode status_code) {
    return kMinRetryStatusCode <= status_code && status_code <= kMaxRetryStatusCode;
}

void CheckResponseStatusCode(const http::StatusCode status_code) {
    if (status_code < kMinGoodStatusCode || kMaxGoodStatusCode < status_code) {
        throw EtcdError(fmt::format("Got bad status code from etcd: {}", status_code));
    }
}

}  // namespace

namespace impl {

ClientImpl::ClientImpl(clients::http::Client& http_client, ClientSettings settings)
    : http_client_(http_client), settings_(settings) {}

void ClientImpl::Put(const std::string& key, const std::string& value) {
    try {
        PerformEtcdRequest(BuildPutUrl, BuildPutData(key, value));
    } catch (const clients::http::HttpClientException& exception) {
        throw EtcdRequestError(fmt::format("Request to etcd was unsuccessful: {}", exception.what()));
    }
}

void ClientImpl::Delete(const std::string& key) {
    try {
        PerformEtcdRequest(BuildDeleteUrl, BuildDeleteData(key));
    } catch (const clients::http::HttpClientException& exception) {
        throw EtcdRequestError(fmt::format("Request to etcd was unsuccessful: {}", exception.what()));
    }
}

std::optional<std::string> ClientImpl::Get(const std::string& key) {
    auto response = PerformEtcdRequest(BuildRangeUrl, BuildRangeData(key));

    const auto json_body = formats::json::FromString(response->body());
    if (!json_body.HasMember("kvs")) {
        return std::nullopt;
    }
    const auto& key_value_list = json_body["kvs"];
    const auto etcd_key = kKeyPrefix + key;
    for (const auto& key_value : key_value_list) {
        if (crypto::base64::Base64Decode(key_value["key"].As<std::string>()) == etcd_key) {
            return crypto::base64::Base64Decode(key_value["value"].As<std::string>());
        }
    }
    return std::nullopt;
}

std::vector<std::string> ClientImpl::Range(const std::string& key_prefix) {
    auto response = PerformEtcdRequest(BuildRangeUrl, BuildRangeData(key_prefix, kLastPossibleKeyPrefix));

    const auto json_body = formats::json::FromString(response->body());
    if (!json_body.HasMember("kvs")) {
        return {};
    }
    const auto& key_value_list = json_body["kvs"];
    std::vector<std::string> values;
    values.reserve(key_value_list.GetSize());
    for (const auto& key_value : key_value_list) {
        values.push_back(crypto::base64::Base64Decode(key_value["value"].As<std::string>()));
    }
    return values;
}

WatchListener ClientImpl::StartWatch(const std::string& key) {
    auto queue = concurrent::SpscQueue<KeyValueState>::Create();

    auto watch_queues_ptr = watch_queues_.Lock();
    watch_queues_ptr->push_back(queue);

    utils::Async("watch task", [&key, producer = queue->GetProducer(), this]() mutable {
        this->WatchKeyChanges(key, std::move(producer));
    }).Detach();

    return WatchListener{queue->GetConsumer()};
}

std::shared_ptr<clients::http::Response> ClientImpl::PerformEtcdRequest(
    const std::function<std::string(const std::string&)>& url_builder,
    const std::string& data
) {
    auto endpoints = settings_.endpoints;
    utils::Shuffle(endpoints);

    std::shared_ptr<clients::http::Response> response_ptr;
    for (const auto& endpoint : endpoints) {
        response_ptr = http_client_.CreateRequest()
                           .post(url_builder(endpoint), data)
                           .retry(settings_.attempts)
                           .timeout(settings_.request_timeout_ms.count())
                           .perform();
        if (!ShouldRetry(response_ptr->status_code())) {
            CheckResponseStatusCode(response_ptr->status_code());
            return response_ptr;
        }
    }
    throw EtcdError("Failed to get Ok response from etcd with error: " + response_ptr->body());
}

clients::http::StreamedResponse ClientImpl::PerformStreamedEtcdRequest(
    const std::function<std::string(const std::string&)>& url_builder,
    const std::string& data
) {
    auto endpoints = settings_.endpoints;
    utils::Shuffle(endpoints);

    std::optional<clients::http::StreamedResponse> maybe_streamed_response;
    for (const auto& endpoint : endpoints) {
        const auto queue = concurrent::StringStreamQueue::Create();
        maybe_streamed_response = http_client_.CreateRequest()
                                      .post(url_builder(endpoint), data)
                                      .retry(settings_.attempts)
                                      .timeout(settings_.watch_timeout_ms.count())
                                      .async_perform_stream_body(queue);
        auto& streamed_response = maybe_streamed_response.value();
        if (!ShouldRetry(streamed_response.StatusCode())) {
            CheckResponseStatusCode(streamed_response.StatusCode());
            return std::move(streamed_response);
        }
    }
    if (maybe_streamed_response.has_value()) {
        throw EtcdError(
            "Failed to get Ok response from etcd with status code: " + maybe_streamed_response.value().StatusCode()
        );
    } else {
        throw EtcdError("Failed to get streamed response, number of etcd endpoints: " + endpoints.size());
    }
}

void ClientImpl::WatchKeyChanges(const std::string& key, concurrent::SpscQueue<KeyValueState>::Producer producer) {
    auto stream_response = PerformStreamedEtcdRequest(BuildWatchUrl, BuildWatchData(key));

    std::string body_part;
    while (stream_response.ReadChunk(body_part, engine::Deadline())) {
        const auto watch_response = formats::json::FromString(body_part);
        LOG_DEBUG() << "Got folowing chunk from etcd watch handler: " << watch_response;
        if (!watch_response["result"].HasMember("events")) {
            LOG_DEBUG() << "No events in watch part response, skipping";
            continue;
        }
        for (const auto& event : watch_response["result"]["events"]) {
            if (!event.HasMember("kv")) {
                LOG_DEBUG() << "Event is not key value change, skipping";
                continue;
            }
            LOG_DEBUG() << "Got event with kv: " << event["kv"];
            if (!producer.Push(event["kv"].As<KeyValueState>())) {
                LOG_ERROR() << "Could not push to queue, aborting task";
                return;
            };
        }
    }
}

}  // namespace impl

}  // namespace etcd

USERVER_NAMESPACE_END
