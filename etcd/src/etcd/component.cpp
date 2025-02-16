#include <userver/etcd/component.hpp>

#include <etcd/client_impl.hpp>
#include <userver/clients/http/component.hpp>
#include <userver/etcd/settings.hpp>
#include <userver/yaml_config/merge_schemas.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

Component::Component(const components::ComponentConfig& config, const components::ComponentContext& context)
    : ComponentBase(config, context),
      etcd_client_ptr_(std::make_shared<impl::ClientImpl>(
          context.FindComponent<components::HttpClient>().GetHttpClient(),
          config.As<ClientSettings>()
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
    attempts:
        type: integer
        description: >
            Number of attempts per one endpoints, total number of attempts is number of endpoints times attempts
        minimum: 1
    request_timeout_ms:
        type: integer
        description: Number of miliseconds between request attempts
        minimum: 1
)");
}

ClientPtr Component::GetClient() { return etcd_client_ptr_; }

}  // namespace etcd

USERVER_NAMESPACE_END
