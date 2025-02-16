#include <userver/etcd/watch_listener.hpp>

#include <userver/crypto/base64.hpp>
#include <userver/etcd/exceptions.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

KeyValueEvent WatchListener::GetEvent() {
    KeyValueEvent event;
    if (!consumer.Pop(event)) {
        throw EtcdError("Consumer pop failed");
    }
    return event;
}

}  // namespace etcd

namespace formats::parse {

etcd::KeyValueEvent Parse(const formats::json::Value& value, To<etcd::KeyValueEvent>) {
    return etcd::KeyValueEvent{
        .key = crypto::base64::Base64Decode(value["key"].As<std::string>()),
        .value = crypto::base64::Base64Decode(value["value"].As<std::string>()),
        .version = std::stoi(value["version"].As<std::string>()),
    };
}

}  // namespace formats::parse

USERVER_NAMESPACE_END
