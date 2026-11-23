/*
 * FPV Checklist — battery log: packs, cycles, condition.
 *
 * SPDX-License-Identifier: MIT
 */

#include "log_screen.h"

#include "theme.h"
#include "ui_kit.h"

#include <algorithm>

namespace screen {
namespace {

using model::PackStatus;
using model::tr;

constexpr int kSide = 6;
constexpr int kRowH = 21;
constexpr int kRowsTop = 18;
constexpr int kFieldW = 150;

const char* status_text(PackStatus s) {
    switch (s) {
        case PackStatus::Ok: return tr("OK", "OK");
        case PackStatus::Watch: return tr("Watch", "Стежити");
        case PackStatus::Retire: return tr("Retire", "Списати");
    }
    return "";
}

lv_color_t status_color(PackStatus s, const view::ThemePalette& c) {
    switch (s) {
        case PackStatus::Ok: return c.success;
        case PackStatus::Watch: return c.warning;
        case PackStatus::Retire: return c.error;
    }
    return c.text;
}

} // namespace

LogScreen::LogScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets)
    : BaseScreen(view_model, assets) {
    init();
}

void LogScreen::build_content(lv_obj_t* content) {
    header_ = ui::label(content, font(10), kSide + 2, 3, 308);

    empty_ = ui::label(content, font(14), kSide + 2, 40, 300);
    ui::wrap(empty_);

    for (int r = 0; r < kVisibleRows; ++r) {
        auto& row = rows_[r];
        row.box = ui::box(content, kSide, kRowsTop + r * kRowH, 320 - 2 * kSide, kRowH - 1);
        row.num = ui::label(row.box, font(12, true), 5, 3, 74);
        row.spec = ui::label(row.box, font(12), 80, 3, 100);
        row.cycles = ui::label(row.box, font(12, true), 182, 3, 56);
        row.status = ui::label(row.box, font(12, true), 238, 3, 68);
    }

    for (int i = 0; i < model::kPackFieldCount; ++i) {
        auto& f = fields_[i];
        f.box = ui::box(content, kSide, kRowsTop + i * 18, kFieldW, 17);
        f.name = ui::label(f.box, font(12), 5, 1);
        f.value = ui::label(f.box, font(12, true), 0, 1);
        lv_obj_align(f.value, LV_ALIGN_RIGHT_MID, -5, 0);
    }

    hint_ = ui::label(content, font(10), kSide + kFieldW + 8, kRowsTop, 320 - kFieldW - 2 * kSide - 8);
    ui::wrap(hint_);

    rename_box_ = ui::box(content, 30, 14, 260, 92, 8);
    rename_title_ = ui::label(rename_box_, font(12, true), 12, 8, 236);
    rename_text_ = ui::label(rename_box_, font(24, true), 12, 30, 236);
    rename_hint_ = ui::label(rename_box_, font(10), 12, 64, 236);
    ui::wrap(rename_hint_);

    observe_state(content);
}

void LogScreen::refresh() {
    const auto& log = view_model().model().log;
    for (auto& row : rows_) ui::show(row.box, false);
    for (auto& f : fields_) ui::show(f.box, false);
    ui::show(hint_, false);
    ui::show(empty_, false);
    ui::show(rename_box_, false);
    if (log.editing && !log.packs.empty()) refresh_edit();
    else refresh_list();
    if (log.renaming && !log.packs.empty()) refresh_rename();
}

void LogScreen::refresh_list() {
    const auto colors = view::palette(view_model().is_dark_mode());
    const auto& log = view_model().model().log;
    const int total = static_cast<int>(log.packs.size());

    if (log.delete_armed) {
        lv_label_set_text_fmt(header_, "%s #%d", tr("Press 8 again to delete", "Натисни 8 ще раз, щоб видалити"),
                              log.selected + 1);
        ui::text_color(header_, colors.error);
    }
    else {
        lv_label_set_text_fmt(header_, "%s: %d  ·  Z/C %s  ·  6 ID  ·  ENTER %s", tr("Packs", "Паки"), total,
                              tr("cycles", "цикли"), tr("edit", "ред."));
        ui::text_color(header_, colors.text_disabled);
    }

    if (total == 0) {
        ui::show(empty_, true);
        lv_label_set_text(empty_, tr("No packs yet.\nPress 5 to add your first battery.",
                                     "Ще немає паків.\nНатисни 5, щоб додати батарею."));
        ui::text_color(empty_, colors.text_disabled);
        return;
    }

    const int first = std::clamp(log.selected - kVisibleRows / 2, 0, std::max(0, total - kVisibleRows));
    for (int r = 0; r < kVisibleRows; ++r) {
        const int i = first + r;
        auto& row = rows_[r];
        if (i >= total) continue;
        ui::show(row.box, true);
        const auto& p = log.packs[i];
        const bool selected = i == log.selected;
        if (p.name.empty()) lv_label_set_text_fmt(row.num, "#%d", i + 1);
        else lv_label_set_text(row.num, p.name.c_str());
        lv_label_set_text_fmt(row.spec, "%s %dS %d", model::kChems[static_cast<int>(p.chem)].name, p.cells,
                              model::capacity_at(p.capacity_index));
        lv_label_set_text_fmt(row.cycles, "%d %s", p.cycles, tr("cyc", "цикл"));
        lv_label_set_text_fmt(row.status, LV_SYMBOL_BULLET " %s", status_text(p.status));
        ui::select_style(row.box, colors.primary, selected);
        ui::text_color(row.num, selected ? colors.primary : p.name.empty() ? colors.text_disabled : colors.text);
        ui::text_color(row.spec, colors.text);
        ui::text_color(row.cycles, colors.text);
        ui::text_color(row.status, status_color(p.status, colors));
    }
}

void LogScreen::refresh_edit() {
    const auto colors = view::palette(view_model().is_dark_mode());
    const auto& log = view_model().model().log;
    const auto& p = log.packs[log.selected];

    if (p.name.empty()) {
        lv_label_set_text_fmt(header_, "%s #%d   ·   6 ID   ·   ENTER %s", tr("Edit pack", "Пак"), log.selected + 1,
                              tr("done", "готово"));
    }
    else {
        lv_label_set_text_fmt(header_, "%s %s   ·   6 ID   ·   ENTER %s", tr("Edit pack", "Пак"), p.name.c_str(),
                              tr("done", "готово"));
    }
    ui::text_color(header_, colors.primary);

    const char* names[model::kPackFieldCount] = {tr("Type", "Тип"), tr("Cells", "Банки"), tr("Capacity", "Ємність"),
                                                 tr("Cycles", "Цикли"), tr("State", "Стан")};
    for (int i = 0; i < model::kPackFieldCount; ++i) {
        auto& f = fields_[i];
        ui::show(f.box, true);
        char value[32];
        switch (i) {
            case 0: lv_snprintf(value, sizeof(value), "%s", model::kChems[static_cast<int>(p.chem)].name); break;
            case 1: lv_snprintf(value, sizeof(value), "%dS", p.cells); break;
            case 2: lv_snprintf(value, sizeof(value), "%d mAh", model::capacity_at(p.capacity_index)); break;
            case 3: lv_snprintf(value, sizeof(value), "%d", p.cycles); break;
            default: lv_snprintf(value, sizeof(value), "%s", status_text(p.status)); break;
        }
        const bool selected = i == log.field;
        lv_label_set_text(f.name, names[i]);
        if (selected) lv_label_set_text_fmt(f.value, LV_SYMBOL_LEFT " %s " LV_SYMBOL_RIGHT, value);
        else lv_label_set_text(f.value, value);
        ui::select_style(f.box, colors.primary, selected);
        ui::text_color(f.name, selected ? colors.text : colors.text_disabled);
        ui::text_color(f.value, i == 4 ? status_color(p.status, colors) : selected ? colors.primary : colors.text);
    }

    // Oscar Liang: when to retire a LiPo.
    ui::show(hint_, true);
    lv_label_set_text(hint_, tr("Retire if:\n- swollen, punctured, dented\n- hot in normal use\n- one cell differs\n- big performance drop",
                                "Списати, якщо:\n- здутий, пробитий, м'ятий\n- гріється у звичайному режимі\n- одна банка відрізняється\n- сильно впала віддача"));
    ui::text_color(hint_, colors.text_disabled);
}

void LogScreen::refresh_rename() {
    const auto colors = view::palette(view_model().is_dark_mode());
    const auto& log = view_model().model().log;

    ui::show(rename_box_, true);
    ui::fill(rename_box_, colors.button, LV_OPA_COVER);
    ui::outline(rename_box_, colors.primary, 2);
    lv_obj_move_foreground(rename_box_);

    lv_label_set_text_fmt(rename_title_, "%s #%d", tr("Pack ID (label on battery)", "ID пака (наліпка)"), log.selected + 1);
    ui::text_color(rename_title_, colors.text_disabled);

    lv_label_set_text_fmt(rename_text_, "%s_", log.draft.c_str());
    ui::text_color(rename_text_, colors.primary);

    lv_label_set_text(rename_hint_, tr("A-Z 0-9 -   max 8   ·   ENTER save   ·   ESC cancel",
                                       "A-Z 0-9 -   макс 8   ·   ENTER зберегти   ·   ESC скасувати"));
    ui::text_color(rename_hint_, colors.text_disabled);
}

} // namespace screen
