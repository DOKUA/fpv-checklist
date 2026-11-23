/*
 * FPV Checklist — checklist page.
 *
 * SPDX-License-Identifier: MIT
 */

#include "checklist_screen.h"

#include "asset_manager.h"
#include "bindings.h"
#include "theme.h"

#include <algorithm>

namespace screen {
namespace {

constexpr int kTabHeight = 20;
constexpr int kRowHeight = 20;
constexpr int kRowsTop = 28;
constexpr int kSidePad = 8;

const lv_font_t* font_or(const lv_font_t* font, const lv_font_t* fallback) {
    return font ? font : fallback;
}

} // namespace

ChecklistScreen::ChecklistScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets)
    : BaseScreen(view_model, assets) {
    init();
}

void ChecklistScreen::build_content(lv_obj_t* content) {
    const auto* tab_font = font_or(assets().load_standard_font(12), &lv_font_montserrat_12);
    const auto* row_font =
        font_or(assets().load_standard_font(14, app::StandardFontWeight::Regular), &lv_font_montserrat_14);

    // Tabs: Bench | Field | Post-flight, each with its own progress counter.
    for (size_t i = 0; i < tabs_.size(); ++i) {
        auto* tab = lv_label_create(content);
        lv_obj_set_style_text_font(tab, tab_font, 0);
        lv_obj_set_style_text_align(tab, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(tab, view::kScreenWidth / 3);
        lv_obj_set_pos(tab, static_cast<int32_t>(i) * (view::kScreenWidth / 3), 4);
        tabs_[i] = tab;
    }

    progress_ = lv_bar_create(content);
    lv_obj_set_size(progress_, view::kScreenWidth - 2 * kSidePad, 4);
    lv_obj_set_pos(progress_, kSidePad, kTabHeight + 1);
    lv_bar_set_range(progress_, 0, 100);

    for (auto& row : rows_) {
        const auto index = static_cast<int32_t>(&row - rows_.data());

        auto* line = lv_obj_create(content);
        lv_obj_remove_style_all(line);
        lv_obj_set_size(line, view::kScreenWidth - 2 * kSidePad, kRowHeight - 1);
        lv_obj_set_pos(line, kSidePad, kRowsTop + index * kRowHeight);
        lv_obj_set_style_radius(line, 4, 0);
        lv_obj_clear_flag(line, LV_OBJ_FLAG_SCROLLABLE);
        row.box = line;

        auto* mark = lv_label_create(line);
        lv_obj_set_style_text_font(mark, &lv_font_montserrat_14, 0);
        lv_obj_align(mark, LV_ALIGN_LEFT_MID, 4, 0);
        row.mark = mark;

        auto* text = lv_label_create(line);
        lv_obj_set_style_text_font(text, row_font, 0);
        lv_label_set_long_mode(text, LV_LABEL_LONG_DOT);
        lv_obj_set_width(text, view::kScreenWidth - 2 * kSidePad - 40);
        lv_obj_align(text, LV_ALIGN_LEFT_MID, 24, 0);
        row.text = text;
    }

    scroll_hint_ = lv_label_create(content);
    lv_obj_set_style_text_font(scroll_hint_, &lv_font_montserrat_10, 0);
    lv_obj_align(scroll_hint_, LV_ALIGN_TOP_RIGHT, -kSidePad - 4, kRowsTop + 4);

    reactive::observe_obj(content, view_model().checklist_subject(), changed_cb, this);
    reactive::observe_obj(content, view_model().dark_mode_subject(), changed_cb, this);
}

void ChecklistScreen::refresh() {
    const auto& m = view_model().model();
    const auto colors = view::palette(view_model().is_dark_mode());
    const int active = m.active_list();
    const auto& def = m.active_def();

    for (size_t i = 0; i < tabs_.size(); ++i) {
        const int list = static_cast<int>(i);
        const bool complete = m.list_complete(list);
        lv_label_set_text_fmt(tabs_[i],
                              "%s%s %d/%d",
                              complete ? LV_SYMBOL_OK " " : "",
                              model::kChecklists[i].title,
                              m.checked_count(list),
                              static_cast<int>(model::kChecklists[i].count));
        lv_color_t color = colors.text_disabled;
        if (list == active) {
            color = complete ? colors.success : colors.primary;
        }
        else if (complete) {
            color = colors.success;
        }
        lv_obj_set_style_text_color(tabs_[i], color, 0);
        lv_obj_set_style_text_decor(tabs_[i], list == active ? LV_TEXT_DECOR_UNDERLINE : LV_TEXT_DECOR_NONE, 0);
    }

    const int total = static_cast<int>(def.count);
    const int done = m.checked_count(active);
    const bool complete = done == total;
    lv_bar_set_value(progress_, total ? done * 100 / total : 0, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(progress_, colors.border, LV_PART_MAIN);
    lv_obj_set_style_bg_color(progress_, complete ? colors.success : colors.primary, LV_PART_INDICATOR);

    // Scroll window so the selected row is always visible.
    const int selected = m.selected_item();
    const int first = std::clamp(selected - kVisibleRows / 2, 0, std::max(0, total - kVisibleRows));

    for (int r = 0; r < kVisibleRows; ++r) {
        auto& row = rows_[r];
        const int item = first + r;
        if (item >= total) {
            lv_obj_add_flag(row.box, LV_OBJ_FLAG_HIDDEN);
            continue;
        }
        lv_obj_remove_flag(row.box, LV_OBJ_FLAG_HIDDEN);

        const bool checked = m.is_checked(active, item);
        const bool is_selected = item == selected;

        lv_label_set_text(row.text, def.items[item]);
        lv_label_set_text(row.mark, checked ? LV_SYMBOL_OK : "-");

        lv_obj_set_style_bg_color(row.box, colors.primary, 0);
        lv_obj_set_style_bg_opa(row.box, is_selected ? LV_OPA_30 : LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(row.box, colors.primary, 0);
        lv_obj_set_style_border_width(row.box, is_selected ? 1 : 0, 0);

        lv_obj_set_style_text_color(row.mark, checked ? colors.success : colors.text_disabled, 0);
        lv_obj_set_style_text_color(row.text, checked ? colors.text_disabled : colors.text, 0);
        lv_obj_set_style_text_decor(row.text, checked ? LV_TEXT_DECOR_STRIKETHROUGH : LV_TEXT_DECOR_NONE, 0);
    }

    const bool more_above = first > 0;
    const bool more_below = first + kVisibleRows < total;
    lv_label_set_text(scroll_hint_, more_above && more_below ? LV_SYMBOL_UP LV_SYMBOL_DOWN
                                    : more_above             ? LV_SYMBOL_UP
                                    : more_below             ? LV_SYMBOL_DOWN
                                                             : "");
    lv_obj_set_style_text_color(scroll_hint_, colors.text_disabled, 0);

}

void ChecklistScreen::changed_cb(lv_observer_t* observer, lv_subject_t* subject) {
    LV_UNUSED(subject);
    auto* screen = static_cast<ChecklistScreen*>(lv_observer_get_user_data(observer));
    if (screen) {
        screen->refresh();
    }
}

} // namespace screen
