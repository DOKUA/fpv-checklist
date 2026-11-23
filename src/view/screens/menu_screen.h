/*
 * FPV Checklist — home menu (tile grid).
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "base_screen.h"

#include <array>

namespace screen {

class MenuScreen : public BaseScreen {
public:
    MenuScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets);

private:
    struct Tile {
        lv_obj_t* box{nullptr};
        lv_obj_t* icon{nullptr};
        lv_obj_t* text{nullptr};
    };

    void build_content(lv_obj_t* content) override;
    void refresh() override;

    std::array<Tile, model::kMenuItemCount> tiles_{};
};

} // namespace screen
