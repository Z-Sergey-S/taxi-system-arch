#pragma once

#include <userver/components/component_base.hpp>
#include <memory>

namespace events {

class EventPublisherComponent final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "event-publisher";

    EventPublisherComponent(const userver::components::ComponentConfig& config,
                            const userver::components::ComponentContext& context);
    ~EventPublisherComponent() override;

    static userver::yaml_config::Schema GetStaticConfigSchema();
};

} // namespace events