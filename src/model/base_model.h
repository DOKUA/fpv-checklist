/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "fpv_data.h"
#include "i18n.h"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace model {

enum class AppPage {
    Menu      = 0,
    Checklist = 1,
    Battery   = 2,
    Channels  = 3,
    Props     = 4,
    Log       = 5,
};
inline constexpr int kMenuItemCount = 6;  // 5 tools + language tile

// ------------------------------------------------------------------ Battery

enum class BatteryView {
    Charge = 0,
    Check  = 1,
    Info   = 2,
};
inline constexpr int kBatteryViewCount = 3;

enum class BatteryStatus {
    Overcharged,
    Full,
    Ready,
    Storage,
    Low,
    Critical,
};

struct BatteryState {
    BatteryView view = BatteryView::Charge;
    int field = 0;
    Chem chem = Chem::LiPo;
    int cells = 6;
    int capacity_index = 6;  // 1300 mAh
    int rate_index = 1;      // 1C
    int pack_cv = 2310;      // centivolts
    int current_a = 25;

    static int field_count(BatteryView view);
    void move_field(int delta);
    void adjust(int delta);
    void switch_view(int delta);

    const ChemInfo& chem_info() const;
    int capacity_mah() const;
    int rate_tenths() const;      // 5 / 10 / 20 => 0.5C / 1C / 2C
    int charge_current_ma() const;
    int cell_cv() const;
    int charge_percent() const;
    int flight_time_s() const;
    BatteryStatus status() const;
    void clamp_voltage();
};

// ------------------------------------------------------------------ Channels

struct ChannelState {
    int sys = kSysR;
    int ch = 0;
    int preset = -1;
    std::vector<ChannelRef> marked;

    void move_system(int delta);
    void move_channel(int delta);
    void toggle_mark();
    void next_preset();
    void clear_marks();
    bool is_marked(int sys, int ch) const;
    int freq10() const;
    const ChannelSystem& system() const;
};

// Two channels overlap when centres are closer than half their bandwidths combined.
bool channels_overlap(const ChannelRef& a, const ChannelRef& b);
int channel_freq10(const ChannelRef& ref);
// Finds an IMD hit (2*f1 - f2 landing inside a third marked channel). Returns false if clean.
bool find_imd(const std::vector<ChannelRef>& marked, ChannelRef& f1, ChannelRef& f2, ChannelRef& victim);

// ------------------------------------------------------------------ Props

struct PropsState {
    int size = 8;  // 5"
    int variant = 0;

    void move_size(int delta);
    void move_variant(int delta);
};

// ------------------------------------------------------------------ Battery log

enum class PackStatus {
    Ok = 0,
    Watch = 1,
    Retire = 2,
};

inline constexpr size_t kMaxPackName = 8;

struct Pack {
    std::string name;  // label stuck on the battery, e.g. "6S-03" (A-Z 0-9 - #)
    Chem chem = Chem::LiPo;
    int cells = 6;
    int capacity_index = 6;
    int cycles = 0;
    PackStatus status = PackStatus::Ok;
};

inline constexpr int kMaxPacks = 24;
inline constexpr int kPackFieldCount = 5;

struct LogState {
    std::vector<Pack> packs;
    int selected = 0;
    bool editing = false;
    int field = 0;
    bool delete_armed = false;
    bool renaming = false;
    std::string draft;

    void move(int delta);
    void start_rename();
    void rename_input(char c);
    void rename_backspace();
    void commit_rename();
    void cancel_rename();
    void add_pack();
    void delete_selected();
    void adjust_cycles(int delta);
    void adjust_field(int delta);
    Pack* current();
};

int capacity_at(int index);
int capacity_count();

// ------------------------------------------------------------------ Model

class BaseModel {
public:
    BaseModel();

    const char* app_title() const;

    bool dark_mode() const { return dark_mode_; }
    void set_dark_mode(bool enabled) { dark_mode_ = enabled; }
    void toggle_dark_mode() { dark_mode_ = !dark_mode_; }

    AppPage current_page() const { return current_page_; }
    void set_current_page(AppPage page) { current_page_ = page; }

    void toggle_language();

    // Menu
    int menu_index = 0;

    // Checklists
    int active_list() const { return active_list_; }
    void switch_list(int delta);
    const ChecklistDef& active_def() const;
    int selected_item() const;
    void move_selection(int delta);
    bool is_checked(int list, int item) const;
    void toggle_selected();
    void reset_active_list();
    int checked_count(int list) const;
    bool list_complete(int list) const;

    BatteryState battery;
    ChannelState channels;
    PropsState props;
    LogState log;

    std::string serialize() const;
    void deserialize(const std::string& text);

private:
    bool dark_mode_ = false;
    AppPage current_page_ = AppPage::Menu;

    int active_list_ = 0;
    std::array<int, kChecklists.size()> selected_{};
    std::array<uint32_t, kChecklists.size()> checked_{};
};

} // namespace model
