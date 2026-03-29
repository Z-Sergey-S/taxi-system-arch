#pragma once
#include <userver/components/component_base.hpp>
#include <userver/components/component_list.hpp>
#include "taxi_storage.hpp"

namespace storage {

class StorageComponent final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "taxi-storage";

    StorageComponent(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context)
        : ComponentBase(config, context) {}

    TaxiStorage& GetStorage() { return storage_; }

private:
    TaxiStorage storage_;
};

} // namespace storage