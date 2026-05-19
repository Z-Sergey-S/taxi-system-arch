#include "event_logger.hpp"
#include <userver/logging/log.hpp>
#include <userver/formats/json/serialize.hpp>

namespace event_consumer {

EventLogger::EventLogger(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : ConsumerComponentBase(config, context) {}

void EventLogger::Process(std::string body) {
    try {
        auto json = userver::formats::json::FromString(body);
        LOG_INFO() << "Received event: body=" << json;
    } catch (const std::exception& e) {
        LOG_ERROR() << "Failed to parse event JSON: " << e.what();
    }
}

} // namespace event_consumer