#pragma once

#include <memory>
#include <vector>

#include <userver/clients/http/component.hpp>
#include <userver/yaml_config/fwd.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

struct ClientV2Settings final {
    std::vector<std::string> endpoints;
};

ClientV2Settings Parse(const yaml_config::YamlConfig& value, formats::parse::To<ClientV2Settings>);

class ClientV2 final {
public:
    ClientV2(clients::http::Client& http_client, ClientV2Settings settings);
    void Put(const std::string& key, const std::string& value);
    [[nodiscard]] std::vector<std::string> Range(const std::string& key);
private:
    clients::http::Client& http_client_;
    const ClientV2Settings settings_;
};

using ClientV2Ptr = std::shared_ptr<ClientV2>;

}

USERVER_NAMESPACE_END
