/*
 * FPV Checklist — 5.8GHz channel spectrum with overlap and IMD check.
 *
 * Every system is one row; each channel is drawn as a pill whose width is the
 * channel bandwidth on a shared frequency axis (like Oscar Liang's chart).
 *
 * SPDX-License-Identifier: MIT
 */

#include "channels_screen.h"

#include "theme.h"
#include "ui_kit.h"

#include <algorithm>
#include <string>

namespace screen {
namespace {

using model::ChannelRef;
using model::kChannelSystems;
using model::tr;

constexpr int kNameW = 34;
constexpr int kPlotX = 38;
constexpr int kPlotW = 276;
constexpr int kFreqMin10 = 56300;  // 5630 MHz
constexpr int kFreqMax10 = 59600;  // 5960 MHz
constexpr int kRowsTop = 13;
constexpr int kRowH = 9;
constexpr int kPillH = 7;

int freq_x(int freq10) {
    return kPlotX + (freq10 - kFreqMin10) * kPlotW / (kFreqMax10 - kFreqMin10);
}

int bw_px(int bw_mhz) {
    return std::max(3, bw_mhz * 10 * kPlotW / (kFreqMax10 - kFreqMin10));
}

std::string channel_name(const ChannelRef& r) {
    const auto& s = kChannelSystems[r.sys];
    if (s.analog || r.sys == model::kSysR) {
        return s.labels[r.ch];
    }
    return std::string(s.name) + " " + s.labels[r.ch];
}

void fmt_freq(char* out, size_t size, int freq10) {
    if (freq10 % 10) lv_snprintf(out, size, "%d.%d", freq10 / 10, freq10 % 10);
    else lv_snprintf(out, size, "%d", freq10 / 10);
}

} // namespace

ChannelsScreen::ChannelsScreen(viewmodel::BaseViewModel& view_model, app::AssetManager& assets)
    : BaseScreen(view_model, assets) {
    init();
}

void ChannelsScreen::build_content(lv_obj_t* content) {
    // Frequency axis: 5650 / 5750 / 5850 / 5950
    for (int i = 0; i < 4; ++i) {
        const int f10 = 56500 + i * 1000;
        tick_lines_[i] = ui::box(content, freq_x(f10), kRowsTop - 1, 1, kVisibleRows * kRowH + 1, 0);
        ticks_[i] = ui::label(content, &lv_font_montserrat_10, freq_x(f10) - 12, 0);
        lv_label_set_text_fmt(ticks_[i], "%d", f10 / 10);
    }

    band_ = ui::box(content, 0, kRowsTop - 1, 10, kVisibleRows * kRowH + 1, 2);

    for (auto& row : rows_) {
        const int index = static_cast<int>(&row - rows_.data());
        const int y = kRowsTop + index * kRowH;
        row.bg = ui::box(content, 2, y, 316, kRowH, 2);
        row.name = ui::label(content, &lv_font_montserrat_10, 4, y - 2, kNameW);
        for (auto& pill : row.chans) {
            pill = ui::box(content, 0, y + 1, 10, kPillH, 3);
        }
    }

    const int info_y = kRowsTop + kVisibleRows * kRowH + 3;
    title_ = ui::label(content, font(14, true), 6, info_y);
    system_ = ui::label(content, font(10), 160, info_y + 3, 154);
    lv_obj_set_style_text_align(system_, LV_TEXT_ALIGN_RIGHT, 0);
    detail_ = ui::label(content, font(12), 6, info_y + 17, 308);

    observe_state(content);
}

void ChannelsScreen::refresh() {
    const auto colors = view::palette(view_model().is_dark_mode());
    const auto& st = view_model().model().channels;
    const ChannelRef selected{st.sys, st.ch};
    const auto& sel_sys = st.system();
    const int total_rows = static_cast<int>(kChannelSystems.size());
    const int first = std::clamp(st.sys - kVisibleRows / 2, 0, total_rows - kVisibleRows);

    for (int i = 0; i < 4; ++i) {
        ui::text_color(ticks_[i], colors.text_disabled);
        ui::fill(tick_lines_[i], colors.border, LV_OPA_COVER);
    }

    // Vertical band showing the selected channel's occupied spectrum across all rows.
    const int sel_f = st.freq10();
    const int sel_w = bw_px(sel_sys.bw);
    lv_obj_set_x(band_, freq_x(sel_f) - sel_w / 2);
    lv_obj_set_width(band_, sel_w);
    ui::fill(band_, colors.primary, LV_OPA_20);

    std::vector<std::string> overlaps;
    for (int r = 0; r < kVisibleRows; ++r) {
        auto& row = rows_[r];
        const int sys_index = first + r;
        const auto& sys = kChannelSystems[sys_index];
        const bool row_selected = sys_index == st.sys;

        ui::fill(row.bg, colors.primary, row_selected ? LV_OPA_10 : LV_OPA_TRANSP);
        lv_label_set_text(row.name, sys.name);
        ui::text_color(row.name, row_selected ? colors.primary : colors.text_disabled);

        for (int c = 0; c < kMaxChannels; ++c) {
            auto* pill = row.chans[c];
            ui::show(pill, c < sys.count);
            if (c >= sys.count) continue;
            const ChannelRef ref{sys_index, c};
            const int w = bw_px(sys.bw);
            lv_obj_set_x(pill, freq_x(sys.freq10[c]) - w / 2);
            lv_obj_set_width(pill, w);

            const bool is_sel = sys_index == st.sys && c == st.ch;
            const bool marked = st.is_marked(sys_index, c);
            const bool overlap = !is_sel && model::channels_overlap(ref, selected);
            lv_color_t color = sys.analog ? colors.text_disabled : colors.info;
            lv_opa_t opa = LV_OPA_50;
            if (overlap) { color = colors.warning; opa = LV_OPA_80; }
            if (marked) { color = colors.success; opa = LV_OPA_COVER; }
            if (is_sel) { color = colors.primary; opa = LV_OPA_COVER; }
            ui::fill(pill, color, opa);
            // Thin surface-coloured edge keeps neighbouring (overlapping) channels distinguishable.
            ui::outline(pill, is_sel ? colors.text : colors.surface, 1);
        }
    }

    // Overlaps across *all* systems (not only visible rows).
    for (int s = 0; s < total_rows; ++s) {
        for (int c = 0; c < kChannelSystems[s].count; ++c) {
            const ChannelRef ref{s, c};
            if ((s != st.sys || c != st.ch) && model::channels_overlap(ref, selected) &&
                kChannelSystems[s].freq10[c] != sel_f) {
                overlaps.push_back(channel_name(ref));
            }
        }
    }

    char freq[16];
    fmt_freq(freq, sizeof(freq), sel_f);
    lv_label_set_text_fmt(title_, "%s  %s MHz", channel_name(selected).c_str(), freq);
    ui::text_color(title_, colors.text);
    lv_label_set_text_fmt(system_, "%s · %d MHz", sel_sys.full, sel_sys.bw);
    ui::text_color(system_, colors.text_disabled);

    // Detail line priority: IMD of marked set > channel tips > overlaps.
    std::string detail;
    lv_color_t detail_color = colors.text_disabled;
    ChannelRef a{}, b{}, victim{};
    if (st.marked.size() >= 2) {
        const std::string prefix = st.preset >= 0 ? std::string(model::kGroupPresets[st.preset].name.get()) + " · "
                                                  : std::string(tr("Marked ", "Позначено ")) +
                                                        std::to_string(st.marked.size()) + " · ";
        if (st.marked.size() >= 3 && model::find_imd(st.marked, a, b, victim)) {
            detail = prefix + "IMD! " + channel_name(a) + "+" + channel_name(b) + " " LV_SYMBOL_RIGHT " " +
                     channel_name(victim);
            detail_color = colors.error;
        }
        else {
            bool clash = false;
            for (size_t i = 0; i < st.marked.size() && !clash; ++i)
                for (size_t j = i + 1; j < st.marked.size() && !clash; ++j)
                    clash = model::channels_overlap(st.marked[i], st.marked[j]);
            detail = prefix + (clash ? tr("channels overlap!", "канали перетинаються!") : tr("IMD OK", "IMD OK"));
            detail_color = clash ? colors.warning : colors.success;
        }
    }
    else if (st.sys == model::kSysR && st.ch == 5) {
        detail = tr("Analog: avoid R6 near DJI/Avatar (public ch)", "Аналог: уникай R6 поруч з DJI/Avatar");
        detail_color = colors.warning;
    }
    else if ((st.sys == model::kSysDjiV1 && st.ch == 7) || (st.sys == model::kSysDjiV1_50 && st.ch == 3)) {
        detail = tr("Public channel: move off after power-up", "Публічний канал: зійди з нього після старту");
        detail_color = colors.warning;
    }
    else if (sel_sys.name[0] == 'O' && sel_sys.name[1] == '4') {
        detail = tr("O4 manual mode. Racing: up to 8 ch (DJI)", "O4 ручний режим. Racing: до 8 каналів");
    }
    else if (overlaps.empty()) {
        detail = tr("No overlaps", "Без перетинів");
        detail_color = colors.success;
    }
    else {
        detail = tr("Overlaps: ", "Перетини: ");
        for (size_t i = 0; i < overlaps.size() && i < 4; ++i) {
            detail += (i ? ", " : "") + overlaps[i];
        }
        if (overlaps.size() > 4) detail += " +" + std::to_string(overlaps.size() - 4);
        detail_color = colors.warning;
    }
    lv_label_set_text(detail_, detail.c_str());
    ui::text_color(detail_, detail_color);
}

} // namespace screen
