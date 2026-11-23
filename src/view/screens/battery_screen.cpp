/*
 * FPV Checklist — battery page: charger settings, voltage check, reference.
 *
 * SPDX-License-Identifier: MIT
 */

#include "battery_screen.h"

#include "theme.h"
#include "ui_kit.h"

namespace screen {
namespace {

using model::BatteryView;
using model::tr;

constexpr int kSide = 8;
constexpr int kBodyTop = 22;
constexpr int kFieldW = 150;
constexpr int kCardX = kSide + kFieldW + 6;
constexpr int kCardW = 320 - kCardX - kSide;
constexpr int kCardH = 86;

void fmt_volts(char* out, size_t size, int cv) {
    lv_snprintf(out, size, "%d.%02d V", cv / 100, cv % 100);
}

const char* view_title(int view) {
    switch (view) {
        case 0: return tr("CHARGE", "ЗАРЯД");
        case 1: return tr("CHECK", "ПЕРЕВІРКА");
        default: return tr("INFO", "ІНФО");
    }
}

} // namespace

BatteryScreen::BatteryScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets)
    : BaseScreen(view_model, assets) {
    init();
}

void BatteryScreen::build_content(lv_obj_t* content) {
    for (int i = 0; i < model::kBatteryViewCount; ++i) {
        tabs_[i] = ui::label(content, font(12, true), i * 106 + 2, 3, 104);
        lv_obj_set_style_text_align(tabs_[i], LV_TEXT_ALIGN_CENTER, 0);
    }

    for (auto& f : fields_) {
        f.box = ui::box(content, kSide, kBodyTop, kFieldW, 20);
        f.name = ui::label(f.box, font(12), 5, 0);
        lv_obj_align(f.name, LV_ALIGN_LEFT_MID, 5, 0);
        f.value = ui::label(f.box, font(12, true), 0, 0);
        lv_obj_align(f.value, LV_ALIGN_RIGHT_MID, -5, 0);
    }

    note_ = ui::label(content, font(10), kSide + 2, kBodyTop + 48, kFieldW - 4);
    lv_label_set_long_mode(note_, LV_LABEL_LONG_WRAP);

    card_ = ui::box(content, kCardX, kBodyTop, kCardW, kCardH, 6);
    for (auto& line : lines_) {
        line = ui::label(card_, font(12), 6, 0, kCardW - 12);
    }
    for (auto& v : vals_) {
        v = ui::label(card_, font(12, true), 0, 0);
    }
    big_ = ui::label(card_, font(20, true), 6, 0);
    bar_ = lv_bar_create(card_);
    lv_bar_set_range(bar_, 0, 100);

    observe_state(content);
}

void BatteryScreen::refresh() {
    const auto colors = view::palette(view_model().is_dark_mode());
    const auto& b = view_model().model().battery;
    const int view = static_cast<int>(b.view);

    for (int i = 0; i < model::kBatteryViewCount; ++i) {
        lv_label_set_text(tabs_[i], view_title(i));
        ui::text_color(tabs_[i], i == view ? colors.primary : colors.text_disabled);
        lv_obj_set_style_text_decor(tabs_[i], i == view ? LV_TEXT_DECOR_UNDERLINE : LV_TEXT_DECOR_NONE, 0);
    }

    ui::fill(card_, colors.button, LV_OPA_COVER);
    ui::outline(card_, colors.border, 1);
    for (auto* line : lines_) {
        ui::show(line, false);
        ui::text_color(line, colors.text);
        lv_obj_set_style_text_font(line, font(12), 0);
    }
    for (auto* v : vals_) {
        ui::show(v, false);
        ui::text_color(v, colors.text);
    }
    ui::show(big_, false);
    ui::show(bar_, false);
    ui::show(note_, false);
    ui::text_color(big_, colors.text);

    refresh_fields();
    switch (b.view) {
        case BatteryView::Charge: refresh_charge(); break;
        case BatteryView::Check: refresh_check(); break;
        case BatteryView::Info: refresh_info(); break;
    }
}

void BatteryScreen::refresh_fields() {
    const auto colors = view::palette(view_model().is_dark_mode());
    const auto& b = view_model().model().battery;
    const int count = model::BatteryState::field_count(b.view);
    const int row_h = count > 4 ? 17 : 21;

    for (int i = 0; i < kMaxFields; ++i) {
        auto& f = fields_[i];
        ui::show(f.box, i < count);
        if (i >= count) continue;
        lv_obj_set_pos(f.box, kSide, kBodyTop + i * row_h);
        lv_obj_set_height(f.box, row_h - 1);

        // Field order must match model::BatteryState (kChargeFields / kCheckFields / kInfoFields).
        const char* name = "";
        char value[32] = "";
        auto field_type = [&] { name = tr("Type", "Тип"); lv_snprintf(value, sizeof(value), "%s", b.chem_info().name); };
        auto field_cells = [&] { name = tr("Cells", "Банки"); lv_snprintf(value, sizeof(value), "%dS", b.cells); };
        auto field_capacity = [&] { name = tr("Capacity", "Ємність"); lv_snprintf(value, sizeof(value), "%d mAh", b.capacity_mah()); };
        switch (b.view) {
            case BatteryView::Charge:
                if (i == 0) field_type();
                if (i == 1) field_cells();
                if (i == 2) field_capacity();
                if (i == 3) {
                    name = tr("Rate", "Швидкість");
                    const int r = b.rate_tenths();
                    if (r % 10) lv_snprintf(value, sizeof(value), "%d.%dC", r / 10, r % 10);
                    else lv_snprintf(value, sizeof(value), "%dC", r / 10);
                }
                break;
            case BatteryView::Check:
                if (i == 0) field_type();
                if (i == 1) field_cells();
                if (i == 2) { name = tr("Pack", "Пак"); fmt_volts(value, sizeof(value), b.pack_cv); }
                if (i == 3) field_capacity();
                if (i == 4) { name = tr("Avg current", "Сер. струм"); lv_snprintf(value, sizeof(value), "%d A", b.current_a); }
                break;
            case BatteryView::Info:
                if (i == 0) field_type();
                if (i == 1) field_cells();
                break;
        }

        const bool selected = i == b.field;
        lv_label_set_text(f.name, name);
        if (selected) lv_label_set_text_fmt(f.value, LV_SYMBOL_LEFT " %s " LV_SYMBOL_RIGHT, value);
        else lv_label_set_text(f.value, value);
        ui::select_style(f.box, colors.primary, selected);
        ui::text_color(f.name, selected ? colors.text : colors.text_disabled);
        ui::text_color(f.value, selected ? colors.primary : colors.text);
    }
}

void BatteryScreen::refresh_charge() {
    const auto colors = view::palette(view_model().is_dark_mode());
    const auto& b = view_model().model().battery;
    const auto& chem = b.chem_info();
    char buf[64];
    char v1[16];
    char v2[16];

    ui::show(lines_[0], true);
    lv_obj_set_style_text_font(lines_[0], font(12, true), 0);
    lv_obj_set_pos(lines_[0], 6, 3);
    lv_label_set_text(lines_[0], b.cells == 1 ? tr("1S charger", "1S зарядка") : tr("Balance charge", "Баланс-заряд"));

    ui::show(lines_[1], true);
    lv_obj_set_pos(lines_[1], 6, 18);
    fmt_volts(v1, sizeof(v1), chem.full_cv);
    lv_snprintf(buf, sizeof(buf), "%s  %s/%s", chem.name, v1, tr("cell", "банку"));
    lv_label_set_text(lines_[1], buf);

    // Charge current: capacity(mAh) * C / 1000  (Oscar: 1C = mAh / 1000)
    const int ma = b.charge_current_ma();
    ui::show(big_, true);
    lv_obj_set_pos(big_, 6, 32);
    const int amps_tenths = (ma + 50) / 100;
    lv_label_set_text_fmt(big_, "%d.%d A", amps_tenths / 10, amps_tenths % 10);
    ui::text_color(big_, colors.primary);

    const int r = b.rate_tenths();
    ui::show(vals_[0], true);
    if (r % 10) lv_label_set_text_fmt(vals_[0], "%d.%dC", r / 10, r % 10);
    else lv_label_set_text_fmt(vals_[0], "%dC", r / 10);
    lv_obj_align(vals_[0], LV_ALIGN_TOP_RIGHT, -6, 38);
    ui::text_color(vals_[0], colors.text_disabled);

    ui::show(lines_[2], true);
    lv_obj_set_pos(lines_[2], 6, 57);
    fmt_volts(v2, sizeof(v2), chem.full_cv * b.cells);
    lv_snprintf(buf, sizeof(buf), "%dS = %s  ~%d %s", b.cells, v2, 600 / r, tr("min", "хв"));
    lv_label_set_text(lines_[2], buf);

    ui::show(lines_[3], true);
    lv_obj_set_style_text_font(lines_[3], font(10, true), 0);
    lv_obj_set_pos(lines_[3], 6, 72);
    if (r > 10) {
        lv_label_set_text(lines_[3], tr(">1C: more heat & wear", ">1C: більше нагріву й зносу"));
        ui::text_color(lines_[3], colors.warning);
    }
    else if (b.cells == 1) {
        lv_label_set_text(lines_[3], tr("Use a dedicated 1S charger", "Краще окрема 1S зарядка"));
        ui::text_color(lines_[3], colors.info);
    }
    else {
        lv_label_set_text(lines_[3], chem.charge_tip.get());
        ui::text_color(lines_[3], b.chem == model::Chem::LiPo ? colors.error : colors.warning);
    }
}

void BatteryScreen::refresh_check() {
    const auto colors = view::palette(view_model().is_dark_mode());
    const auto& b = view_model().model().battery;
    const auto& chem = b.chem_info();

    struct StatusView {
        const char* text;
        lv_color_t color;
    };
    StatusView status{"", colors.text};
    switch (b.status()) {
        case model::BatteryStatus::Overcharged: status = {tr("OVERCHARGED!", "ПЕРЕЗАРЯД!"), colors.error}; break;
        case model::BatteryStatus::Full: status = {tr("FULL - fly", "ПОВНА - лети"), colors.success}; break;
        case model::BatteryStatus::Ready: status = {tr("OK to fly", "Можна літати"), colors.success}; break;
        case model::BatteryStatus::Storage: status = {tr("STORAGE", "STORAGE"), colors.info}; break;
        case model::BatteryStatus::Low: status = {tr("LOW - land", "МАЛО - сідай"), colors.warning}; break;
        case model::BatteryStatus::Critical: status = {tr("CRITICAL!", "КРИТИЧНО!"), colors.error}; break;
    }

    char buf[48];
    const int cv = b.cell_cv();
    ui::show(big_, true);
    lv_obj_set_pos(big_, 6, 3);
    lv_label_set_text_fmt(big_, "%d.%02d V", cv / 100, cv % 100);

    ui::show(vals_[0], true);
    lv_label_set_text(vals_[0], tr("/cell", "/банку"));
    ui::text_color(vals_[0], colors.text_disabled);
    lv_obj_align(vals_[0], LV_ALIGN_TOP_RIGHT, -6, 9);

    const int percent = b.charge_percent();
    ui::show(bar_, true);
    lv_obj_set_size(bar_, kCardW - 56, 8);
    lv_obj_set_pos(bar_, 6, 32);
    lv_bar_set_value(bar_, percent, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar_, colors.border, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar_, status.color, LV_PART_INDICATOR);
    ui::show(vals_[1], true);
    lv_label_set_text_fmt(vals_[1], b.chem == model::Chem::LiPo ? "%d%%" : "~%d%%", percent);
    lv_obj_align(vals_[1], LV_ALIGN_TOP_RIGHT, -6, 28);

    ui::show(lines_[0], true);
    lv_obj_set_style_text_font(lines_[0], font(14, true), 0);
    lv_obj_set_pos(lines_[0], 6, 44);
    lv_label_set_text(lines_[0], status.text);
    ui::text_color(lines_[0], status.color);

    const int seconds = b.flight_time_s();
    ui::show(lines_[1], true);
    lv_obj_set_pos(lines_[1], 6, 61);
    lv_snprintf(buf, sizeof(buf), "%s ~%d:%02d", tr("Flight", "Політ"), seconds / 60, seconds % 60);
    lv_label_set_text(lines_[1], buf);
    ui::text_color(lines_[1], colors.text_disabled);

    ui::show(lines_[2], true);
    lv_obj_set_style_text_font(lines_[2], font(10), 0);
    lv_obj_set_pos(lines_[2], 6, 74);
    if (chem.land_lo_cv == chem.land_hi_cv) {
        lv_snprintf(buf, sizeof(buf), "%s %d.%02dV", tr("Lower limit ~", "Нижня межа ~"), chem.land_lo_cv / 100,
                    chem.land_lo_cv % 100);
    }
    else {
        lv_snprintf(buf, sizeof(buf), "%s %d.%d-%d.%dV", tr("Land at", "Сідай при"), chem.land_lo_cv / 100,
                    chem.land_lo_cv / 10 % 10, chem.land_hi_cv / 100, chem.land_hi_cv / 10 % 10);
    }
    lv_label_set_text(lines_[2], buf);
    ui::text_color(lines_[2], colors.text_disabled);
}

void BatteryScreen::refresh_info() {
    const auto colors = view::palette(view_model().is_dark_mode());
    const auto& b = view_model().model().battery;
    const auto& chem = b.chem_info();
    char buf[48];
    char v[16];

    // Header
    ui::show(lines_[4], true);
    lv_obj_set_style_text_font(lines_[4], font(10), 0);
    lv_obj_set_pos(lines_[4], 6, 2);
    lv_snprintf(buf, sizeof(buf), "%s / %dS", tr("per cell", "на банку"), b.cells);
    lv_label_set_text(lines_[4], buf);
    ui::text_color(lines_[4], colors.text_disabled);

    const char* names[4] = {tr("Full", "Повна"), tr("Storage", "Storage"), tr("Land", "Посадка"),
                            tr("Min", "Мінімум")};
    for (int i = 0; i < 4; ++i) {
        ui::show(lines_[i], true);
        lv_obj_set_pos(lines_[i], 6, 16 + i * 17);
        lv_label_set_text(lines_[i], names[i]);
        ui::text_color(lines_[i], colors.text_disabled);

        switch (i) {
            case 0:
                fmt_volts(v, sizeof(v), chem.full_cv * b.cells);
                lv_snprintf(buf, sizeof(buf), "%d.%02d / %s", chem.full_cv / 100, chem.full_cv % 100, v);
                break;
            case 1:
                if (chem.storage_lo_cv == 0) lv_snprintf(buf, sizeof(buf), "%s", tr("datasheet", "datasheet"));
                else lv_snprintf(buf, sizeof(buf), "%d.%02d-%d.%02d", chem.storage_lo_cv / 100, chem.storage_lo_cv % 100,
                                 chem.storage_hi_cv / 100, chem.storage_hi_cv % 100);
                break;
            case 2:
                if (chem.land_lo_cv == chem.land_hi_cv) lv_snprintf(buf, sizeof(buf), "~%d.%02d", chem.land_lo_cv / 100, chem.land_lo_cv % 100);
                else lv_snprintf(buf, sizeof(buf), "%d.%d-%d.%d", chem.land_lo_cv / 100, chem.land_lo_cv / 10 % 10,
                                 chem.land_hi_cv / 100, chem.land_hi_cv / 10 % 10);
                break;
            default:
                lv_snprintf(buf, sizeof(buf), "%d.%02d", chem.min_cv / 100, chem.min_cv % 100);
                break;
        }
        ui::show(vals_[i], true);
        lv_label_set_text(vals_[i], buf);
        lv_obj_align(vals_[i], LV_ALIGN_TOP_RIGHT, -6, 16 + i * 17);
        ui::text_color(vals_[i], i == 3 ? colors.error : i == 1 ? colors.info : colors.text);
    }

    ui::show(note_, true);
    lv_obj_set_pos(note_, kSide + 2, kBodyTop + 46);
    ui::text_color(note_, colors.text_disabled);
    if (b.chem == model::Chem::LiIon) {
        lv_label_set_text(note_, tr("Li-ion: long range, ~5C. Storage per cell datasheet.",
                                    "Li-ion: лонг-рендж, ~5C. Storage за datasheet банок."));
    }
    else {
        lv_label_set_text(note_, tr("Not flying for >1 week? Storage mode. Min is a limit, not a target.",
                                    "Не літаєш >1 тижня? Режим Storage. Мінімум - межа, не ціль."));
    }
}

} // namespace screen
