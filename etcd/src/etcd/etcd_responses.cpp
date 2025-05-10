#include <etcd/etcd_responses.hpp>

#include <userver/etcd/exceptions.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/logging/log.hpp>

USERVER_NAMESPACE_BEGIN

namespace formats::parse {

etcd::EtcdRangeResponse Parse(const formats::json::Value& value, To<etcd::EtcdRangeResponse>) {
    if (!value.HasMember("kvs")) {
        return etcd::EtcdRangeResponse{};
    }
    return etcd::EtcdRangeResponse{
        /* .key_value_states = */ value["kvs"].As<std::vector<etcd::KeyValueState>>(),
    };
}

etcd::EtcdWatchResponse Parse(const formats::json::Value& value, To<etcd::EtcdWatchResponse>) {
    if (!value.HasMember("result")) {
        throw etcd::EtcdWatchResponseParseError(
            fmt::format("No result in watch response: {}", formats::json::ToString(value))
        );
    }
    if (!value["result"].HasMember("events")) {
        throw etcd::EtcdWatchResponseParseError(
            fmt::format("No events in watch response: {}", formats::json::ToString(value))
        );
    }
    etcd::EtcdWatchResponse etcd_watch_response;
    for (const auto& event : value["result"]["events"]) {
        if (!event.HasMember("kv")) {
            LOG_DEBUG() << "Event is not key value change, skipping";
            continue;
        }
        etcd_watch_response.events.push_back(event["kv"].As<etcd::KeyValueState>());
    }
    return etcd_watch_response;
}

}  // namespace formats::parse

USERVER_NAMESPACE_END
