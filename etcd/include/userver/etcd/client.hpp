#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <userver/clients/http/component.hpp>
#include <userver/concurrent/queue.hpp>
#include <userver/engine/task/task_with_result.hpp>
#include <userver/etcd/settings.hpp>
#include <userver/etcd/watch_listener.hpp>
#include <userver/yaml_config/fwd.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

class Client {
public:
    virtual ~Client() = default;

    virtual void Put(const std::string& key, const std::string& value) = 0;

    [[nodiscard]] virtual std::optional<std::string> Get(const std::string& key) = 0;

    [[nodiscard]] virtual std::vector<std::string> Range(const std::string& key) = 0;

    virtual void Delete(const std::string& key) = 0;

    virtual WatchListener StartWatch(const std::string& key) = 0;
};

using ClientPtr = std::shared_ptr<Client>;

}  // namespace etcd

USERVER_NAMESPACE_END
