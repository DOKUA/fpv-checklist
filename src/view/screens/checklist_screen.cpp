/*
 * FPV Toolkit — checklist page.
 *
 * SPDX-License-Identifier: MIT
 */

#include "checklist_screen.h"

#include "theme.h"
#include "ui_kit.h"

#include <algorithm>

namespace screen {
namespace {

constexpr int kTabHeight = 20;
constexpr int kRowHeight = 20;
constexpr int kRowsTop = 28;
constexpr int kSidePad = 8;
constexpr int kTabWidth = view::kScreenWidth / static_cast<int>(model::kChecklists.size());

} // namespace

ChecklistScreen::ChecklistScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets)
    : BaseScreen(view_model, assets) {
    init();
}

void ChecklistScreen::build_content(lv_obj_t* content) {
    const auto* tab_font = font(12, true);
    const auto* row_font = font(14);

    for (size_t i = 0; i < tabs_.size(); ++i) {
        auto* tab = ui::label(content, tab_font, static_cast<int32_t>(i) * kTabWidth, 4, kTabWidth);
        lv_obj_set_style_text_align(tab, LV_TEXT_ALIGN_CENTER, 0);
        tabs_[i] = tab;
    }

    progress_ = lv_bar_create(content);
    lv_obj_set_size(progress_, view::kScreenWidth - 2 * kSidePad, 4);
    lv_obj_set_pos(progress_, kSidePad, kTabHeight + 1);
    lv_bar_set_range(progress_, 0, 100);

    for (int r = 0; r < kVisibleRows; ++r) {
        auto& row = rows_[r];
        row.box = ui::box(content, kSidePad, kRowsTop + r * kRowHeight, view::kScreenWidth - 2 * kSidePad, kRowHeight - 1);
        row.mark = lv_label_create(row.box);
        lv_obj_set_style_text_font(row.mark, &lv_font_montserrat_14, 0);
        lv_obj_align(row.mark, LV_ALIGN_LEFT_MID, 4, 0);
        row.text = ui::label(row.box, row_font, 24, 0, view::kScreenWidth - 2 * kSidePad - 44);
        lv_obj_align(row.text, LV_ALIGN_LEFT_MID, 24, 0);
    }

    scroll_hint_ = ui::label(content, &lv_font_montserrat_10, view::kScreenWidth - kSidePad - 16, kRowsTop + 4);

    observe_state(content);
}

void ChecklistScreen::refresh() {
    const auto& m = view_model().model();
    const auto colors = view::palette(view_model().is_dark_mode());
    const int active = m.active_list();
    const auto& def = m.active_def();

    for (size_t i = 0; i < tabs_.size(); ++i) {
        const int list = static_cast<int>(i);
        const bool complete = m.list_complete(list);
        lv_label_set_text_fmt(tabs_[i], "%s%s %d/%d", complete ? LV_SYMBOL_OK " " : "",
                              model::kChecklists[i].title.get(), m.checked_count(list),
                              static_cast<int>(model::kChecklists[i].count));
        lv_color_t color = colors.text_disabled;
        if (list == active) color = complete ? colors.success : colors.primary;
        else if (complete) color = colors.success;
        ui::text_color(tabs_[i], color);
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
        ui::show(row.box, item < total);
        if (item >= total) continue;

        const bool checked = m.is_checked(active, item);
        lv_label_set_text(row.text, def.items[item].get());
        lv_label_set_text(row.mark, checked ? LV_SYMBOL_OK : "-");
        ui::select_style(row.box, colors.primary, item == selected);
        ui::text_color(row.mark, checked ? colors.success : colors.text_disabled);
        ui::text_color(row.text, checked ? colors.text_disabled : colors.text);
        lv_obj_set_style_text_decor(row.text, checked ? LV_TEXT_DECOR_STRIKETHROUGH : LV_TEXT_DECOR_NONE, 0);
    }

    const bool more_above = first > 0;
    const bool more_below = first + kVisibleRows < total;
    lv_label_set_text(scroll_hint_, more_above && more_below ? LV_SYMBOL_UP "\n" LV_SYMBOL_DOWN
                                    : more_above             ? LV_SYMBOL_UP
                                    : more_below             ? "\n" LV_SYMBOL_DOWN
                                                             : "");
    ui::text_color(scroll_hint_, colors.text_disabled);
}

} // namespace screen
