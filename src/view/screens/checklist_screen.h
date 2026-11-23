/*
 * FPV Checklist — checklist page.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "base_screen.h"

#include <array>

namespace screen {

class ChecklistScreen : public BaseScreen {
public:
    ChecklistScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets);

private:
    static constexpr int kVisibleRows = 4;

    struct Row {
        lv_obj_t* box{nullptr};
        lv_obj_t* mark{nullptr};
        lv_obj_t* text{nullptr};
    };

    void build_content(lv_obj_t* content) override;
    void refresh();
    static void changed_cb(lv_observer_t* observer, lv_subject_t* subject);

    std::array<lv_obj_t*, 3> tabs_{};
    lv_obj_t* progress_{nullptr};
    std::array<Row, kVisibleRows> rows_{};
    lv_obj_t* scroll_hint_{nullptr};
};

} // namespace screen
