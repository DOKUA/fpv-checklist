/*
 * FPV Checklist — propeller sizes (mm) and typical builds.
 *
 * SPDX-License-Identifier: MIT
 */

#include "props_screen.h"

#include "theme.h"
#include "ui_kit.h"

#include <algorithm>
#include <cstring>

namespace screen {
namespace {

using model::kPropSizes;
using model::tr;

constexpr int kListX = 6;
constexpr int kListW = 82;
constexpr int kRowH = 21;
constexpr int kPanelX = kListX + kListW + 8;
constexpr int kPanelW = 320 - kPanelX - 6;

} // namespace

PropsScreen::PropsScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets)
    : BaseScreen(view_model, assets) {
    init();
}

void PropsScreen::build_content(lv_obj_t* content) {
    for (int r = 0; r < kVisibleRows; ++r) {
        auto& row = list_[r];
        row.box = ui::box(content, kListX, 2 + r * kRowH, kListW, kRowH - 1);
        row.size = ui::label(row.box, font(14, true), 6, 0);
        lv_obj_align(row.size, LV_ALIGN_LEFT_MID, 6, 0);
        row.mm = ui::label(row.box, font(10), 0, 0);
        lv_obj_align(row.mm, LV_ALIGN_RIGHT_MID, -5, 0);
    }

    header_ = ui::label(content, font(20, true), kPanelX, 0, kPanelW);
    variant_ = ui::label(content, font(12, true), kPanelX, 27, kPanelW - 46);
    pager_ = ui::label(content, font(10), kPanelX + kPanelW - 46, 29, 46);
    lv_obj_set_style_text_align(pager_, LV_TEXT_ALIGN_RIGHT, 0);
    for (int i = 0; i < kDetailRows; ++i) {
        keys_[i] = ui::label(content, font(10), kPanelX, 47 + i * 15, 66);
        values_[i] = ui::label(content, font(12, true), kPanelX + 66, 45 + i * 15, kPanelW - 66);
    }

    observe_state(content);
}

void PropsScreen::refresh() {
    const auto colors = view::palette(view_model().is_dark_mode());
    const auto& st = view_model().model().props;
    const int total = static_cast<int>(kPropSizes.size());
    const int first = std::clamp(st.size - kVisibleRows / 2, 0, total - kVisibleRows);

    for (int r = 0; r < kVisibleRows; ++r) {
        auto& row = list_[r];
        const auto& p = kPropSizes[first + r];
        const bool selected = first + r == st.size;
        lv_label_set_text(row.size, p.label);
        if (p.label[std::strlen(p.label) - 1] == 'm') lv_label_set_text_fmt(row.mm, "%d.%d\"", p.tenth_inch / 10, p.tenth_inch % 10);
        else lv_label_set_text_fmt(row.mm, "%d", p.mm);
        ui::select_style(row.box, colors.primary, selected);
        ui::text_color(row.size, selected ? colors.primary : colors.text);
        ui::text_color(row.mm, selected ? colors.primary : colors.text_disabled);
    }

    const auto& p = kPropSizes[st.size];
    const bool whoop = p.label[std::strlen(p.label) - 1] == 'm';
    if (whoop) lv_label_set_text_fmt(header_, "%d mm = %d.%d\"", p.mm, p.tenth_inch / 10, p.tenth_inch % 10);
    else lv_label_set_text_fmt(header_, "%s = %d mm", p.label, p.mm);
    ui::text_color(header_, colors.primary);

    const char* keys[kDetailRows] = {tr("Motor", "Мотор"), "KV", tr("Battery", "Батарея"), tr("Dry weight", "Вага")};
    if (p.count == 0) {
        lv_label_set_text(variant_, tr("No build data", "Немає даних збірки"));
        ui::text_color(variant_, colors.text_disabled);
        lv_label_set_text(pager_, "");
        for (int i = 0; i < kDetailRows; ++i) {
            ui::show(keys_[i], i == 0);
            ui::show(values_[i], false);
        }
        lv_label_set_text(keys_[0], tr("Not in Oscar's lookup table", "Немає в таблиці Оскара"));
        lv_obj_set_width(keys_[0], kPanelW);
        ui::text_color(keys_[0], colors.text_disabled);
        return;
    }

    lv_obj_set_width(keys_[0], 66);
    const auto& v = p.variants[std::min(st.variant, p.count - 1)];
    if (v.blades[0] == '-') lv_label_set_text_fmt(variant_, "%s · %s", v.cls.get(), v.cells);
    else lv_label_set_text_fmt(variant_, "%s · %s · %s", v.cls.get(), v.cells, v.blades);
    ui::text_color(variant_, colors.text);
    lv_label_set_text_fmt(pager_, LV_SYMBOL_LEFT "%d/%d" LV_SYMBOL_RIGHT, st.variant + 1, p.count);
    ui::text_color(pager_, p.count > 1 ? colors.primary : colors.text_disabled);

    const char* values[kDetailRows] = {v.motors, v.kv, v.battery, v.dry};
    for (int i = 0; i < kDetailRows; ++i) {
        ui::show(keys_[i], true);
        ui::show(values_[i], true);
        lv_label_set_text(keys_[i], keys[i]);
        if (i == 2 && v.battery[0] != '-' && v.battery[0] != 'L') lv_label_set_text_fmt(values_[i], "%s mAh", v.battery);
        else lv_label_set_text(values_[i], values[i]);
        ui::marquee(values_[i], i == 0);  // motor lists can be long
        ui::text_color(keys_[i], colors.text_disabled);
        ui::text_color(values_[i], colors.text);
    }
}

} // namespace screen
