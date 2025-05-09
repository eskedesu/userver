#pragma once

/// @file userver/etcd/watch_listener.hpp
/// @brief Queue with value change events in etcd

#include <string>

#include <userver/concurrent/queue.hpp>
#include <userver/etcd/key_value_state.hpp>
#include <userver/formats/json/value.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

/// @brief Struct that return value change events in etcd
struct WatchListener final {
    concurrent::SpscQueue<KeyValueState>::Consumer consumer;

    /// Get an event from etcd if there was one, otherwise waits asynchronously until a next event occurs.
    /// If the coroutine, that was spawned by StartWatch method of etcd client, is finished or failed, GetEvent raises
    /// EtcdError exception Get Event uses Consumer::Pop method for getting the event.
    KeyValueState GetEvent();
};

}  // namespace etcd

USERVER_NAMESPACE_END
