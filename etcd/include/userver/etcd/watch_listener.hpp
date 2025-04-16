#pragma once

#include <string>

#include <userver/concurrent/queue.hpp>
#include <userver/formats/json/value.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

struct KeyValueState final {
    std::string key;
    std::string value;
    std::int32_t version;
};

struct WatchListener final {
    concurrent::SpscQueue<KeyValueState>::Consumer consumer;

    KeyValueState GetEvent();
};

}  // namespace etcd

namespace formats::parse {

etcd::KeyValueState Parse(const formats::json::Value& value, To<etcd::KeyValueState>);

}

USERVER_NAMESPACE_END
