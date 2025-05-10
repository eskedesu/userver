#pragma once

#include <userver/etcd/key_value_state.hpp>
#include <userver/formats/parse/common_containers.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

struct EtcdRangeResponse final {
    std::vector<KeyValueState> key_value_states;
};

struct EtcdWatchResponse final {
    std::vector<KeyValueState> events;
};

}  // namespace etcd

namespace formats::parse {

etcd::EtcdRangeResponse Parse(const formats::json::Value& value, To<etcd::EtcdRangeResponse>);

etcd::EtcdWatchResponse Parse(const formats::json::Value& value, To<etcd::EtcdWatchResponse>);

}  // namespace formats::parse

USERVER_NAMESPACE_END
