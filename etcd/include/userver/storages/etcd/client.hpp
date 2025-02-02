#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <userver/clients/http/component.hpp>
#include <userver/concurrent/queue.hpp>
#include <userver/engine/task/task_with_result.hpp>
#include <userver/storages/etcd/settings.hpp>
#include <userver/storages/etcd/watch_listener.hpp>
#include <userver/yaml_config/fwd.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

namespace impl {

class ClientImpl;

}

class Client {
public:
    virtual ~Client() = default;
    virtual void Put(const std::string& key, const std::string& value) = 0;
    [[nodiscard]] virtual std::vector<std::string> Range(const std::string& key) = 0;
    virtual void DeleteRange(const std::string& key) = 0;
    virtual WatchListener StartWatch(const std::string& key) = 0;
};

namespace impl {

class ClientImpl : public Client {
    public:
    ~ClientImpl() override = default;
    ClientImpl(clients::http::Client& http_client, ClientSettings settings);
    void Put(const std::string& key, const std::string& value) override;
    [[nodiscard]] std::vector<std::string> Range(const std::string& key) override;
    void DeleteRange(const std::string& key) override;
    WatchListener StartWatch(const std::string& key) override;  
private:
    [[nodiscard]] std::shared_ptr<clients::http::Response>
     PerformEtcdRequest(const std::function<std::string(const std::string&)>& url_builder, const std::string& data);
    [[nodiscard]] clients::http::StreamedResponse PerformStreamEtcdRequest(
    const std::function<std::string(const std::string&)>& url_builder, const std::string& data);

    clients::http::Client& http_client_;
    std::vector<userver::engine::TaskWithResult<void>> watch_tasks_;
    std::vector<std::shared_ptr<concurrent::SpscQueue<KVEvent>>> watch_queues_;
    const ClientSettings settings_;
};

}

using ClientPtr = std::shared_ptr<Client>;

}

USERVER_NAMESPACE_END
