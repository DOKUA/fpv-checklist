/*
 * FPV Checklist — LiPo calculator page.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "base_screen.h"

#include <array>

namespace screen {

class LipoScreen : public BaseScreen {
public:
    LipoScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets);

private:
    struct FieldRow {
        lv_obj_t* box{nullptr};
        lv_obj_t* name{nullptr};
        lv_obj_t* value{nullptr};
    };

    void build_content(lv_obj_t* content) override;
    void refresh();
    static void changed_cb(lv_observer_t* observer, lv_subject_t* subject);

    std::array<FieldRow, model::kLipoFieldCount> fields_{};
    lv_obj_t* cell_voltage_{nullptr};
    lv_obj_t* cell_caption_{nullptr};
    lv_obj_t* charge_bar_{nullptr};
    lv_obj_t* charge_label_{nullptr};
    lv_obj_t* status_{nullptr};
    lv_obj_t* flight_time_{nullptr};
};

} // namespace screen
