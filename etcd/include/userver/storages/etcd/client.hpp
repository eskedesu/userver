#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <vector>

#include <userver/clients/http/component.hpp>
#include <userver/engine/shared_mutex.hpp>
#include <userver/storages/etcd/settings.hpp>
#include <userver/yaml_config/fwd.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

class Client final {
public:
    Client(clients::http::Client& http_client, ClientSettings settings);
    void Put(const std::string& key, const std::string& value);
    [[nodiscard]] std::vector<std::string> Range(const std::string& key);
    void DeleteRange(const std::string& key);
private:
    [[nodiscard]] std::shared_ptr<clients::http::Response>
     PerformEtcdRequest(const std::function<std::string(const std::string&)>& url_builder, const std::string& data);

    clients::http::Client& http_client_;
    engine::SharedMutex endpoints_shared_mutex_;
    ClientSettings settings_;
};

using ClientPtr = std::shared_ptr<Client>;

}

USERVER_NAMESPACE_END
