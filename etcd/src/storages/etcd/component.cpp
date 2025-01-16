#include <userver/storages/etcd/component.hpp>

#include <userver/clients/http/component.hpp>
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
description: Etcd client component
additionalProperties: false
properties:
    endpoints:
        type: array
        description: etcd hosts
        items:
            type: string
            description: host
)");
}

ClientV2Ptr Component::GetClientV2() {
    return etcd_client_v2_ptr_;
}

}  // namespace storages::etcd

USERVER_NAMESPACE_END
