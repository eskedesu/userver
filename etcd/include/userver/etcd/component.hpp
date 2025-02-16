#pragma once

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/etcd/client.hpp>

USERVER_NAMESPACE_BEGIN

namespace etcd {

class Component final : public components::ComponentBase {
public:
    static constexpr std::string_view kName = "etcd-сlient";

    Component(const components::ComponentConfig&, const components::ComponentContext&);

    static yaml_config::Schema GetStaticConfigSchema();

    ClientPtr GetClient();

private:
    const ClientPtr etcd_client_ptr_;
};

}  // namespace etcd

USERVER_NAMESPACE_END
