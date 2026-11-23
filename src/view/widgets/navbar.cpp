/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include "navbar.h"

#include "asset_manager.h"
#include "bindings.h"
#include "linux_input.h"
#include "theme.h"
#include "ui_const.h"

namespace view::widgets {
namespace {

// Icon centres must line up with keys 4..8 (x = 49, 105, 161, 217, 273).
constexpr std::array<int32_t, 5> kNavButtonXOffsets = {30, 17, 2, -15, -30};

} // namespace

NavBar::NavBar(lv_obj_t* parent, viewmodel::BaseViewModel& view_model, app::AssetManager& assets)
    : BaseWidgets(parent), view_model_(view_model), assets_(assets) {}

NavBar::~NavBar() {
    for (size_t i = 0; i < icon_buttons_.size(); ++i) {
        if (icon_buttons_[i]) {
            platform::unregister_nav_button(i, icon_buttons_[i]->root());
        }
    }
}

void NavBar::build() {
    if (core_obj_) {
        return;
    }

    core_obj_ = lv_obj_create(parent_);
    lv_obj_remove_style_all(core_obj_);
    lv_obj_set_size(core_obj_, LV_PCT(100), kNavBarHeight);
    lv_obj_align(core_obj_, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_clear_flag(core_obj_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(core_obj_, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(core_obj_, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(core_obj_, 8, 0);
    lv_obj_set_style_pad_right(core_obj_, 8, 0);
    reactive::bind_theme(core_obj_, view_model_.dark_mode_subject(), reactive::ThemeRole::Bar);

    create_icon_buttons();
    // Observers are bound to core_obj_ and removed automatically with it.
    reactive::observe_obj(core_obj_, view_model_.current_page_subject(), update_icons_cb, this);
    reactive::observe_obj(core_obj_, view_model_.dark_mode_subject(), update_icons_cb, this);
}

void NavBar::create_icon_buttons() {
    const auto light_color = view::palette(false).text;
    const auto dark_color = view::palette(true).text;
    icon_font_ = assets_.load_font("Phosphor-Fill.ttf", 22);

    platform::set_nav_shortcut_mode(false);
    for (size_t i = 0; i < icon_buttons_.size(); ++i) {
        icon_buttons_[i] = std::make_unique<IconButton>(core_obj_,
                                                        view_model_,
                                                        32,
                                                        24,
                                                        "",
                                                        icon_font_ ? icon_font_ : &lv_font_montserrat_14,
                                                        light_color,
                                                        dark_color,
                                                        nullptr,
                                                        &view_model_,
                                                        true);
        icon_buttons_[i]->build();
        lv_obj_set_style_translate_x(icon_buttons_[i]->root(), kNavButtonXOffsets[i], 0);
        // Keys are routed explicitly; keep buttons out of the focus group so Enter
        // does not "click" whichever button happens to be focused.
        lv_group_remove_obj(icon_buttons_[i]->root());
        lv_obj_add_event_cb(icon_buttons_[i]->root(), button_clicked_cb, LV_EVENT_CLICKED, this);
        platform::register_nav_button(i, icon_buttons_[i]->root());
    }
}

void NavBar::update_icon_buttons() {
    const bool lipo = view_model_.current_page() == model::AppPage::Lipo;
    const char* theme_icon = view_model_.is_dark_mode() ? ICON_SUN : ICON_MOON;

    //                                       key 4          5                  6               7                8
    const std::array<const char*, 5> checklist = {ICON_SIGN_OUT, ICON_CARET_LEFT, ICON_RESET, ICON_CARET_RIGHT, ICON_BATTERY};
    const std::array<const char*, 5> lipo_page = {ICON_LIST_CHECKS, ICON_MINUS, ICON_CARET_UP_DOWN, ICON_PLUS, theme_icon};

    const auto& icons = lipo ? lipo_page : checklist;
    for (size_t i = 0; i < icon_buttons_.size(); ++i) {
        if (icon_buttons_[i]) {
            icon_buttons_[i]->set_text(icons[i]);
            icon_buttons_[i]->set_enabled(true);
        }
    }
}

void NavBar::on_button(size_t index) {
    auto& vm = view_model_;
    if (vm.current_page() == model::AppPage::Checklist) {
        switch (index) {
            case 0: vm.request_quit(); break;
            case 1: vm.switch_list(-1); break;
            case 2: vm.reset_active_list(); break;
            case 3: vm.switch_list(+1); break;
            case 4: vm.show_lipo_page(); break;
            default: break;
        }
        return;
    }

    switch (index) {
        case 0: vm.show_checklist_page(); break;
        case 1: vm.adjust_lipo_field(-1); break;
        case 2: vm.move_lipo_field(+1); break;
        case 3: vm.adjust_lipo_field(+1); break;
        case 4: vm.toggle_dark_mode(); break;
        default: break;
    }
}

void NavBar::button_clicked_cb(lv_event_t* event) {
    auto* nav_bar = static_cast<NavBar*>(lv_event_get_user_data(event));
    auto* target = static_cast<lv_obj_t*>(lv_event_get_current_target(event));
    if (!nav_bar) {
        return;
    }
    for (size_t i = 0; i < nav_bar->icon_buttons_.size(); ++i) {
        if (nav_bar->icon_buttons_[i] && nav_bar->icon_buttons_[i]->root() == target) {
            nav_bar->on_button(i);
            return;
        }
    }
}

void NavBar::update_icons_cb(lv_observer_t* observer, lv_subject_t* subject) {
    LV_UNUSED(subject);

    auto* nav_bar = static_cast<NavBar*>(lv_observer_get_user_data(observer));
    if (nav_bar) {
        nav_bar->update_icon_buttons();
    }
}

} // namespace view::widgets
