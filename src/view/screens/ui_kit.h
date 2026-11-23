/*
 * FPV Checklist — tiny helpers for building screens.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "lvgl.h"

namespace ui {

inline lv_obj_t* label(lv_obj_t* parent, const lv_font_t* font, int32_t x, int32_t y, int32_t width = 0) {
    auto* l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_pos(l, x, y);
    if (width > 0) {
        lv_obj_set_width(l, width);
        lv_label_set_long_mode(l, LV_LABEL_LONG_DOT);
    }
    return l;
}

inline lv_obj_t* box(lv_obj_t* parent, int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius = 4) {
    auto* b = lv_obj_create(parent);
    lv_obj_remove_style_all(b);
    lv_obj_set_pos(b, x, y);
    lv_obj_set_size(b, w, h);
    lv_obj_set_style_radius(b, radius, 0);
    lv_obj_clear_flag(b, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(b, LV_OBJ_FLAG_CLICKABLE);
    return b;
}

inline void fill(lv_obj_t* obj, lv_color_t color, lv_opa_t opa) {
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, opa, 0);
}

inline void outline(lv_obj_t* obj, lv_color_t color, int32_t width) {
    lv_obj_set_style_border_color(obj, color, 0);
    lv_obj_set_style_border_width(obj, width, 0);
}

inline void text_color(lv_obj_t* obj, lv_color_t color) {
    lv_obj_set_style_text_color(obj, color, 0);
}

inline void show(lv_obj_t* obj, bool visible) {
    if (visible) {
        lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
    else {
        lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
    }
}

// Highlight a selectable row: tinted background + outline when selected.
inline void select_style(lv_obj_t* obj, lv_color_t accent, bool selected) {
    fill(obj, accent, selected ? LV_OPA_30 : LV_OPA_TRANSP);
    outline(obj, accent, selected ? 1 : 0);
}

} // namespace ui
