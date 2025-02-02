#include <userver/storages/etcd/watch_listener.hpp>

#include <userver/storages/etcd/exceptions.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

KVEvent WatchListener::GetEvent() {
    KVEvent event;
    if (!consumer.Pop(event)) {
        throw EtcdError("Consumer pop failed");
    }
    return event;
}

}  // namespace formats::parse

USERVER_NAMESPACE_END
