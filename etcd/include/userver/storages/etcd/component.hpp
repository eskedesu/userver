#pragma once

#include <userver/components/component_base.hpp>
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/storages/etcd/client.hpp>

USERVER_NAMESPACE_BEGIN

namespace storages::etcd {

class Component final : public components::ComponentBase {
public:
    static constexpr std::string_view kName = "etcd";

    Component(const components::ComponentConfig&, const components::ComponentContext&);

    ~Component() = default;

    static yaml_config::Schema GetStaticConfigSchema();

    ClientV2Ptr GetClientV2();

private:
    const ClientV2Ptr etcd_client_v2_ptr_;
};

}

USERVER_NAMESPACE_END
