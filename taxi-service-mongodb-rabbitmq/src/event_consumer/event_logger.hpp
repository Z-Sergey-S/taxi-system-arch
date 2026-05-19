#pragma once

#include <userver/urabbitmq/consumer_component_base.hpp>

namespace event_consumer {

class EventLogger : public userver::urabbitmq::ConsumerComponentBase {
public:
    static constexpr std::string_view kName = "event-logger";

    EventLogger(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context);

    void Process(std::string body) override;
};

} // namespace event_consumer