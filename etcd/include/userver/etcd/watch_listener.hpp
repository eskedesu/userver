#pragma once

#include <string>

#include <userver/concurrent/queue.hpp>
#include <userver/formats/json/value.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

struct KeyValueEvent final {
    std::string key;
    std::string value;
    std::int32_t version;
};

struct WatchListener final {
    concurrent::SpscQueue<KeyValueEvent>::Consumer consumer;

    KeyValueEvent GetEvent();
};

}  // namespace etcd

namespace formats::parse {

etcd::KeyValueEvent Parse(const formats::json::Value& value, To<etcd::KeyValueEvent>);

}

USERVER_NAMESPACE_END
