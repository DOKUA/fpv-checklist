/*
 * FPV Checklist — 5.8GHz channel spectrum with overlap and IMD check.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "base_screen.h"

#include <array>

namespace screen {

class ChannelsScreen : public BaseScreen {
public:
    ChannelsScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets);

private:
    static constexpr int kVisibleRows = 7;
    static constexpr int kMaxChannels = 8;

    struct Row {
        lv_obj_t* bg{nullptr};
        lv_obj_t* name{nullptr};
        std::array<lv_obj_t*, kMaxChannels> chans{};
    };

    void build_content(lv_obj_t* content) override;
    void refresh() override;

    std::array<lv_obj_t*, 4> ticks_{};
    std::array<lv_obj_t*, 4> tick_lines_{};
    lv_obj_t* band_{nullptr};
    std::array<Row, kVisibleRows> rows_{};
    lv_obj_t* title_{nullptr};
    lv_obj_t* system_{nullptr};
    lv_obj_t* detail_{nullptr};
};

} // namespace screen
