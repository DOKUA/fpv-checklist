/*
 * FPV Toolkit — battery page: charger settings, voltage check, reference.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "base_screen.h"

#include <array>

namespace screen {

class BatteryScreen : public BaseScreen {
public:
    BatteryScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets);

private:
    static constexpr int kMaxFields = 5;
    static constexpr int kCardLines = 6;

    struct FieldRow {
        lv_obj_t* box{nullptr};
        lv_obj_t* name{nullptr};
        lv_obj_t* value{nullptr};
    };

    void build_content(lv_obj_t* content) override;
    void refresh() override;
    void refresh_fields();
    void refresh_charge();
    void refresh_check();
    void refresh_info();

    std::array<lv_obj_t*, model::kBatteryViewCount> tabs_{};
    std::array<FieldRow, kMaxFields> fields_{};
    lv_obj_t* card_{nullptr};
    std::array<lv_obj_t*, kCardLines> lines_{};
    std::array<lv_obj_t*, 4> vals_{};
    lv_obj_t* big_{nullptr};
    lv_obj_t* bar_{nullptr};
    lv_obj_t* note_{nullptr};
};

} // namespace screen
