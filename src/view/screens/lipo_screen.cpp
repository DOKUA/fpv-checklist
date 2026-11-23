/*
 * FPV Checklist — LiPo calculator page.
 *
 * SPDX-License-Identifier: MIT
 */

#include "lipo_screen.h"

#include "asset_manager.h"
#include "bindings.h"
#include "theme.h"

namespace screen {
namespace {

constexpr int kSidePad = 8;
constexpr int kFieldWidth = 172;
constexpr int kFieldHeight = 24;
constexpr int kFieldsTop = 6;
constexpr int kResultsLeft = kSidePad + kFieldWidth + 10;
constexpr int kResultsWidth = 320 - kResultsLeft - kSidePad;

constexpr const char* kFieldNames[model::kLipoFieldCount] = {
    "Cells",
    "Pack",
    "Capacity",
    "Avg current",
};

const lv_font_t* font_or(const lv_font_t* font, const lv_font_t* fallback) {
    return font ? font : fallback;
}

struct StatusView {
    const char* text;
    bool warn;
    bool error;
};

StatusView status_view(model::LipoStatus status) {
    switch (status) {
        case model::LipoStatus::Overcharged: return {"OVERCHARGED!", false, true};
        case model::LipoStatus::Full:        return {"FULL - fly", false, false};
        case model::LipoStatus::Good:        return {"OK to fly", false, false};
        case model::LipoStatus::Storage:     return {"STORAGE", true, false};
        case model::LipoStatus::Low:         return {"LOW - land", true, true};
        case model::LipoStatus::Critical:    return {"CRITICAL!", false, true};
    }
    return {"", false, false};
}

} // namespace

LipoScreen::LipoScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets)
    : BaseScreen(view_model, assets) {
    init();
}

void LipoScreen::build_content(lv_obj_t* content) {
    const auto* name_font =
        font_or(assets().load_standard_font(12, app::StandardFontWeight::Regular), &lv_font_montserrat_12);
    const auto* value_font = font_or(assets().load_standard_font(14), &lv_font_montserrat_14);
    const auto* big_font = font_or(assets().load_standard_font(24), &lv_font_montserrat_24);
    const auto* small_font = font_or(assets().load_standard_font(12), &lv_font_montserrat_12);

    for (int i = 0; i < model::kLipoFieldCount; ++i) {
        auto& field = fields_[i];

        field.box = lv_obj_create(content);
        lv_obj_remove_style_all(field.box);
        lv_obj_set_size(field.box, kFieldWidth, kFieldHeight - 2);
        lv_obj_set_pos(field.box, kSidePad, kFieldsTop + i * kFieldHeight);
        lv_obj_set_style_radius(field.box, 4, 0);
        lv_obj_clear_flag(field.box, LV_OBJ_FLAG_SCROLLABLE);

        field.name = lv_label_create(field.box);
        lv_label_set_text(field.name, kFieldNames[i]);
        lv_obj_set_style_text_font(field.name, name_font, 0);
        lv_obj_align(field.name, LV_ALIGN_LEFT_MID, 6, 0);

        field.value = lv_label_create(field.box);
        lv_obj_set_style_text_font(field.value, value_font, 0);
        lv_obj_align(field.value, LV_ALIGN_RIGHT_MID, -6, 0);
    }

    cell_voltage_ = lv_label_create(content);
    lv_obj_set_style_text_font(cell_voltage_, big_font, 0);
    lv_obj_set_pos(cell_voltage_, kResultsLeft, 2);

    cell_caption_ = lv_label_create(content);
    lv_label_set_text(cell_caption_, "per cell");
    lv_obj_set_style_text_font(cell_caption_, &lv_font_montserrat_10, 0);
    lv_obj_set_pos(cell_caption_, kResultsLeft, 30);

    charge_bar_ = lv_bar_create(content);
    lv_obj_set_size(charge_bar_, kResultsWidth - 40, 10);
    lv_obj_set_pos(charge_bar_, kResultsLeft, 48);
    lv_bar_set_range(charge_bar_, 0, 100);

    charge_label_ = lv_label_create(content);
    lv_obj_set_style_text_font(charge_label_, small_font, 0);
    lv_obj_set_pos(charge_label_, kResultsLeft + kResultsWidth - 36, 45);

    status_ = lv_label_create(content);
    lv_obj_set_style_text_font(status_, value_font, 0);
    lv_obj_set_pos(status_, kResultsLeft, 64);

    flight_time_ = lv_label_create(content);
    lv_obj_set_style_text_font(flight_time_, small_font, 0);
    lv_obj_set_pos(flight_time_, kResultsLeft, 86);

    reactive::observe_obj(content, view_model().lipo_subject(), changed_cb, this);
    reactive::observe_obj(content, view_model().dark_mode_subject(), changed_cb, this);
}

void LipoScreen::refresh() {
    const auto& m = view_model().model();
    const auto colors = view::palette(view_model().is_dark_mode());
    const int selected = static_cast<int>(m.lipo_field());

    for (int i = 0; i < model::kLipoFieldCount; ++i) {
        auto& field = fields_[i];
        const bool is_selected = i == selected;

        char value[32];
        switch (static_cast<model::LipoField>(i)) {
            case model::LipoField::Cells:
                lv_snprintf(value, sizeof(value), "%dS", m.cells());
                break;
            case model::LipoField::Voltage:
                lv_snprintf(value, sizeof(value), "%d.%02d V", m.pack_centivolts() / 100, m.pack_centivolts() % 100);
                break;
            case model::LipoField::Capacity:
                lv_snprintf(value, sizeof(value), "%d mAh", m.capacity_mah());
                break;
            case model::LipoField::Current:
                lv_snprintf(value, sizeof(value), "%d A", m.current_amps());
                break;
        }
        if (is_selected) {
            lv_label_set_text_fmt(field.value, LV_SYMBOL_LEFT " %s " LV_SYMBOL_RIGHT, value);
        }
        else {
            lv_label_set_text(field.value, value);
        }

        lv_obj_set_style_bg_color(field.box, colors.primary, 0);
        lv_obj_set_style_bg_opa(field.box, is_selected ? LV_OPA_30 : LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_color(field.box, colors.primary, 0);
        lv_obj_set_style_border_width(field.box, is_selected ? 1 : 0, 0);
        lv_obj_set_style_text_color(field.name, colors.text_disabled, 0);
        lv_obj_set_style_text_color(field.value, is_selected ? colors.primary : colors.text, 0);
    }

    const auto status = status_view(m.lipo_status());
    const lv_color_t status_color = status.error ? colors.error : status.warn ? colors.warning : colors.success;

    const int cv = m.cell_centivolts();
    lv_label_set_text_fmt(cell_voltage_, "%d.%02d V", cv / 100, cv % 100);
    lv_obj_set_style_text_color(cell_voltage_, colors.text, 0);
    lv_obj_set_style_text_color(cell_caption_, colors.text_disabled, 0);

    const int percent = m.charge_percent();
    lv_bar_set_value(charge_bar_, percent, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(charge_bar_, colors.border, LV_PART_MAIN);
    lv_obj_set_style_bg_color(charge_bar_, status_color, LV_PART_INDICATOR);
    lv_label_set_text_fmt(charge_label_, "%d%%", percent);
    lv_obj_set_style_text_color(charge_label_, colors.text, 0);

    lv_label_set_text(status_, status.text);
    lv_obj_set_style_text_color(status_, status_color, 0);

    const int seconds = m.flight_time_seconds();
    lv_label_set_text_fmt(flight_time_, "Flight ~%d:%02d min", seconds / 60, seconds % 60);
    lv_obj_set_style_text_color(flight_time_, colors.text_disabled, 0);
}

void LipoScreen::changed_cb(lv_observer_t* observer, lv_subject_t* subject) {
    LV_UNUSED(subject);
    auto* screen = static_cast<LipoScreen*>(lv_observer_get_user_data(observer));
    if (screen) {
        screen->refresh();
    }
}

} // namespace screen
