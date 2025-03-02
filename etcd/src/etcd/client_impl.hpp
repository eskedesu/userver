#pragma once

#include <userver/concurrent/queue.hpp>
#include <userver/engine/mutex.hpp>
#include <userver/etcd/client.hpp>
#include <userver/etcd/settings.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

namespace impl {

class ClientImpl : public Client {
public:
    ClientImpl(clients::http::Client& http_client, ClientSettings settings);

    void Put(const std::string& key, const std::string& value) override;

    [[nodiscard]] std::optional<std::string> Get(const std::string& key) override;

    [[nodiscard]] std::vector<std::string> Range(const std::string& key) override;

    void DeleteRange(const std::string& key) override;

    WatchListener StartWatch(const std::string& key) override;

private:
    [[nodiscard]] std::shared_ptr<clients::http::Response>
    PerformEtcdRequest(const std::function<std::string(const std::string&)>& url_builder, const std::string& data);

    [[nodiscard]] clients::http::StreamedResponse PerformStreamedEtcdRequest(
        const std::function<std::string(const std::string&)>& url_builder,
        const std::string& data
    );

    clients::http::Client& http_client_;
    engine::Mutex watch_queues_lock_;
    std::vector<std::shared_ptr<concurrent::SpscQueue<KeyValueEvent>>> watch_queues_;
    const ClientSettings settings_;
};

}  // namespace impl

}  // namespace etcd

USERVER_NAMESPACE_END
