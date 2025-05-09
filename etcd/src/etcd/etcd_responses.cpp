#include <etcd/etcd_responses.hpp>

USERVER_NAMESPACE_BEGIN

namespace formats::parse {

etcd::EtcdRangeResponse Parse(const formats::json::Value& value, To<etcd::EtcdRangeResponse>) {
    return etcd::EtcdRangeResponse{
        /* .key_value_states = */ value["kvs"].As<std::vector<etcd::KeyValueState>>(),
    };
}

}  // namespace formats::parse

USERVER_NAMESPACE_END
