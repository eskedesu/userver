#pragma once

#include <userver/etcd/key_value_state.hpp>
#include <userver/formats/parse/common_containers.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

struct EtcdRangeResponse {
    std::vector<KeyValueState> key_value_states;
};

struct EtcdWatchResponse {
    /* data */
};

}  // namespace etcd

namespace formats::parse {

etcd::EtcdRangeResponse Parse(const formats::json::Value& value, To<etcd::EtcdRangeResponse>);

}

USERVER_NAMESPACE_END
