#include "event_publisher_component.hpp"
#include "event_publisher.hpp"
#include <userver/components/component_config.hpp>
#include <userver/components/component_context.hpp>
#include <userver/yaml_config/merge_schemas.hpp>
#include <userver/logging/log.hpp>

namespace events {

EventPublisherComponent::EventPublisherComponent(
    const userver::components::ComponentConfig& config,
    const userver::components::ComponentContext& context)
    : userver::components::ComponentBase(config, context) {
    
    std::string exchange_name = config["exchange_name"].As<std::string>("taxi.events");
    
    auto& publisher = EventPublisher::GetInstance();
    publisher.Initialize(exchange_name);
    
    LOG_INFO() << "EventPublisherComponent initialized with exchange: " << exchange_name;
}

EventPublisherComponent::~EventPublisherComponent() = default;

userver::yaml_config::Schema EventPublisherComponent::GetStaticConfigSchema() {
    return userver::yaml_config::MergeSchemas<userver::components::ComponentBase>(R"(
type: object
description: Event Publisher Component
additionalProperties: false
properties:
    exchange_name:
        type: string
        description: RabbitMQ exchange name for events
        default: taxi.events
)");
}

} // namespace events