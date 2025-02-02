#pragma once

#include <string>

#include <userver/concurrent/queue.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

struct KVEvent final {
    std::string key;
    std::string value;
    std::size_t version;
};

struct WatchListener final {
    concurrent::SpscQueue<KVEvent>::Consumer consumer;

    KVEvent GetEvent();
};

}

USERVER_NAMESPACE_END
