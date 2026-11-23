/*
 * FPV Checklist — propeller sizes (mm) and typical builds.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "base_screen.h"

#include <array>

namespace screen {

class PropsScreen : public BaseScreen {
public:
    PropsScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets);

private:
    static constexpr int kVisibleRows = 5;
    static constexpr int kDetailRows = 4;

    struct ListRow {
        lv_obj_t* box{nullptr};
        lv_obj_t* size{nullptr};
        lv_obj_t* mm{nullptr};
    };

    void build_content(lv_obj_t* content) override;
    void refresh() override;

    std::array<ListRow, kVisibleRows> list_{};
    lv_obj_t* header_{nullptr};
    lv_obj_t* variant_{nullptr};
    lv_obj_t* pager_{nullptr};
    std::array<lv_obj_t*, kDetailRows> keys_{};
    std::array<lv_obj_t*, kDetailRows> values_{};
};

} // namespace screen
