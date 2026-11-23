/*
 * FPV Toolkit — home menu (tile grid).
 *
 * SPDX-License-Identifier: MIT
 */

#include "menu_screen.h"

#include "asset_manager.h"
#include "theme.h"
#include "ui_const.h"
#include "ui_kit.h"

namespace screen {
namespace {

using model::tr;

constexpr int kTileW = 96;
constexpr int kTileH = 50;
constexpr int kGap = 8;

const char* tile_icon(int index) {
    switch (index) {
        case 0: return view::ICON_LIST_CHECKS;
        case 1: return view::ICON_BATTERY;
        case 2: return view::ICON_BROADCAST;
        case 3: return view::ICON_FAN;
        case 4: return view::ICON_NOTEBOOK;
        default: return view::ICON_TRANSLATE;
    }
}

const char* tile_text(int index) {
    switch (index) {
        case 0: return tr("Checklist", "Чекліст");
        case 1: return tr("Battery", "Батарея");
        case 2: return tr("Channels", "Канали");
        case 3: return tr("Props", "Пропелери");
        case 4: return tr("Battery log", "Журнал");
        default: return tr("Language: EN", "Мова: UA");
    }
}

} // namespace

MenuScreen::MenuScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets)
    : BaseScreen(view_model, assets) {
    init();
}

void MenuScreen::build_content(lv_obj_t* content) {
    const lv_font_t* icon_font = assets().load_font("Phosphor-Fill.ttf", 22);
    const lv_font_t* text_font = font(12, true);

    for (int i = 0; i < model::kMenuItemCount; ++i) {
        const int col = i % 3;
        const int row = i / 3;
        auto& tile = tiles_[i];
        tile.box = ui::box(content, kGap + col * (kTileW + kGap), 4 + row * (kTileH + 4), kTileW, kTileH, 8);

        tile.icon = lv_label_create(tile.box);
        lv_label_set_text(tile.icon, icon_font ? tile_icon(i) : LV_SYMBOL_LIST);
        lv_obj_set_style_text_font(tile.icon, icon_font ? icon_font : &lv_font_montserrat_20, 0);
        lv_obj_align(tile.icon, LV_ALIGN_TOP_MID, 0, 4);

        tile.text = lv_label_create(tile.box);
        lv_obj_set_style_text_font(tile.text, text_font, 0);
        lv_obj_align(tile.text, LV_ALIGN_BOTTOM_MID, 0, -4);
    }
    observe_state(content);
}

void MenuScreen::refresh() {
    const auto colors = view::palette(view_model().is_dark_mode());
    const int selected = view_model().model().menu_index;
    for (int i = 0; i < model::kMenuItemCount; ++i) {
        auto& tile = tiles_[i];
        const bool is_selected = i == selected;
        ui::fill(tile.box, is_selected ? colors.primary : colors.button, is_selected ? LV_OPA_30 : LV_OPA_COVER);
        ui::outline(tile.box, is_selected ? colors.primary : colors.border, 1);
        lv_label_set_text(tile.text, tile_text(i));
        ui::text_color(tile.icon, is_selected ? colors.primary : colors.text);
        ui::text_color(tile.text, colors.text);
    }
}

} // namespace screen
