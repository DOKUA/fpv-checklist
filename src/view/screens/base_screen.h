/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "navbar.h"
#include "base_viewmodel.h"
#include "titlebar.h"
#include "lvgl.h"

#include <memory>

namespace app {
class AssetManager;
}

namespace screen {

class BaseScreen {
public:
    BaseScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets);
    virtual ~BaseScreen();

    BaseScreen(const BaseScreen&) = delete;
    BaseScreen& operator=(const BaseScreen&) = delete;

    void init();
    lv_obj_t* root() const;
    void refresh_now();

protected:
    virtual void build_content(lv_obj_t* content) = 0;

    viewmodel::BaseViewModel& view_model();
    app::AssetManager& assets();
    // Standard UI font (Cyrillic-capable) with built-in Montserrat as fallback.
    const lv_font_t* font(uint32_t size, bool bold = false);
    // Redraw content whenever app state or theme changes.
    void observe_state(lv_obj_t* obj);
    virtual void refresh() {}

private:
    viewmodel::BaseViewModel& view_model_;
    app::AssetManager& assets_;
    lv_obj_t* root_{nullptr};
    lv_obj_t* content_{nullptr};
    std::unique_ptr<view::widgets::TitleBar> title_bar_;
    std::unique_ptr<view::widgets::NavBar> nav_bar_;
};

} // namespace screen
