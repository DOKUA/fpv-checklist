/*
 * FPV Checklist — battery log: packs, cycles, condition.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "base_screen.h"

#include <array>

namespace screen {

class LogScreen : public BaseScreen {
public:
    LogScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets);

private:
    static constexpr int kVisibleRows = 4;

    struct PackRow {
        lv_obj_t* box{nullptr};
        lv_obj_t* num{nullptr};
        lv_obj_t* spec{nullptr};
        lv_obj_t* cycles{nullptr};
        lv_obj_t* status{nullptr};
    };

    struct FieldRow {
        lv_obj_t* box{nullptr};
        lv_obj_t* name{nullptr};
        lv_obj_t* value{nullptr};
    };

    void build_content(lv_obj_t* content) override;
    void refresh() override;
    void refresh_list();
    void refresh_edit();

    lv_obj_t* header_{nullptr};
    lv_obj_t* empty_{nullptr};
    std::array<PackRow, kVisibleRows> rows_{};
    std::array<FieldRow, model::kPackFieldCount> fields_{};
    lv_obj_t* hint_{nullptr};
};

} // namespace screen
