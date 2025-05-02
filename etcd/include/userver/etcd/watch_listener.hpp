#pragma once

/// @file userver/etcd/watch_listener.hpp
/// @brief Queue with value change events in etcd

#include <string>

#include <userver/concurrent/queue.hpp>
#include <userver/formats/json/value.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

/// @brief Struct with key value pair from etcd. It represents current status of key value pair.
struct KeyValueState final {
    std::string key;
    std::string value;
    std::int32_t version;
};

/// @brief Struct that return value change events in etcd
struct WatchListener final {
    concurrent::SpscQueue<KeyValueState>::Consumer consumer;

    KeyValueState GetEvent();
};

}  // namespace etcd

namespace formats::parse {

etcd::KeyValueState Parse(const formats::json::Value& value, To<etcd::KeyValueState>);

}

USERVER_NAMESPACE_END
