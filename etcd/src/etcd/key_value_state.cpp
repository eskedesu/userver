#include <userver/etcd/key_value_state.hpp>

#include <userver/crypto/base64.hpp>
#include <userver/etcd/exceptions.hpp>

USERVER_NAMESPACE_BEGIN

namespace {
const std::string kKeyPrefix = "/etcd/";
}

namespace formats::parse {

etcd::KeyValueState Parse(const formats::json::Value& value, To<etcd::KeyValueState>) {
    return etcd::KeyValueState{
        /* .key = */ crypto::base64::Base64Decode(value["key"].As<std::string>()).substr(kKeyPrefix.size()),
        /* .value = */ crypto::base64::Base64Decode(value["value"].As<std::string>()),
        /* .version = */ std::stoi(value["version"].As<std::string>()),
    };
}

}  // namespace formats::parse

USERVER_NAMESPACE_END
