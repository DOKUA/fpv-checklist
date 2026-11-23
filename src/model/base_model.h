/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "fpv_data.h"

#include <array>
#include <cstdint>
#include <string>

namespace model {

enum class AppPage {
    Checklist = 0,
    Lipo      = 1,
};

enum class LipoField {
    Cells    = 0,
    Voltage  = 1,
    Capacity = 2,
    Current  = 3,
};
constexpr int kLipoFieldCount = 4;

enum class LipoStatus {
    Overcharged,
    Full,
    Good,
    Storage,
    Low,
    Critical,
};

class BaseModel {
public:
    BaseModel();

    const char* app_title() const;

    bool dark_mode() const;
    void set_dark_mode(bool enabled);
    void toggle_dark_mode();

    AppPage current_page() const;
    void set_current_page(AppPage page);
    void toggle_page();

    // ---- Checklists -------------------------------------------------------
    int active_list() const;
    void switch_list(int delta);
    const ChecklistDef& active_def() const;
    int selected_item() const;
    void move_selection(int delta);
    bool is_checked(int list, int item) const;
    void toggle_selected();
    void reset_active_list();
    int checked_count(int list) const;
    bool list_complete(int list) const;

    // ---- LiPo calculator --------------------------------------------------
    LipoField lipo_field() const;
    void move_lipo_field(int delta);
    void adjust_lipo_field(int delta);

    int cells() const;
    int pack_centivolts() const;        // 2310 == 23.10 V
    int cell_centivolts() const;        // per-cell voltage, rounded
    int capacity_mah() const;
    int current_amps() const;
    int charge_percent() const;         // resting-voltage estimate, 0..100
    int flight_time_seconds() const;    // 80 % usable capacity at avg current
    LipoStatus lipo_status() const;

    // ---- Persistence ------------------------------------------------------
    std::string serialize() const;
    void deserialize(const std::string& text);

private:
    bool dark_mode_ = false;
    AppPage current_page_ = AppPage::Checklist;

    int active_list_ = 0;
    std::array<int, kChecklists.size()> selected_{};
    std::array<uint32_t, kChecklists.size()> checked_{};  // bit mask per list

    LipoField lipo_field_ = LipoField::Voltage;
    int cells_ = 6;
    int pack_centivolts_ = 2310;
    int capacity_index_ = 6;  // 1300 mAh
    int current_amps_ = 25;
};

} // namespace model
