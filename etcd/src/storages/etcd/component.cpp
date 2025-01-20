#include <userver/storages/etcd/component.hpp>

#include <userver/clients/http/component.hpp>
#include <userver/storages/etcd/settings.hpp>
#include <userver/yaml_config/merge_schemas.hpp>


USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

Component::Component(const components::ComponentConfig& config, const components::ComponentContext& context)
    :
    ComponentBase(config, context),
    etcd_client_v2_ptr_(std::make_shared<ClientV2>(
        context.FindComponent<components::HttpClient>().GetHttpClient(),
        config.As<ClientV2Settings>()
    )) {}

yaml_config::Schema Component::GetStaticConfigSchema() {
    return yaml_config::MergeSchemas<ComponentBase>(R"(
type: object
description: Etcd cluster component
additionalProperties: false
properties:
    endpoints:
        type: array
        description: Etcd endpoints
        items:
            type: string
            description: host
    retries:
        type: integer
        description: >
            Number of retries per one endpoints, total number of retries is number of endpoints times retries
        minimum: 1
    request_timeout_ms:
        type: integer
        description: Number of miliseconds to timeout request
        minimum: 1
)");
}

ClientV2Ptr Component::GetClientV2() {
    return etcd_client_v2_ptr_;
}

}  // namespace storages::etcd

USERVER_NAMESPACE_END
