/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include "base_model.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>

namespace model {
namespace {

constexpr std::array<int, 16> kCapacities = {
    450, 550, 650, 850, 1000, 1100, 1300, 1500, 1800, 2200, 3000, 4000, 5000, 6000, 8000, 10000,
};

constexpr int kMinCells = 1;
constexpr int kMaxCells = 8;
constexpr int kVoltageStep = 5;  // 0.05 V
constexpr int kMaxCellCentivolts = 440;
constexpr int kMinCellCentivolts = 250;

// Resting LiPo voltage (centivolts per cell) -> state of charge.
struct SocPoint {
    int centivolts;
    int percent;
};
constexpr std::array<SocPoint, 21> kSocCurve = {{
    {327, 0},   {361, 5},   {369, 10},  {371, 15},  {373, 20},  {375, 25},  {377, 30},
    {379, 35},  {380, 40},  {382, 45},  {384, 50},  {385, 55},  {387, 60},  {391, 65},
    {395, 70},  {398, 75},  {402, 80},  {408, 85},  {411, 90},  {415, 95},  {420, 100},
}};

int wrap(int value, int count) {
    return ((value % count) + count) % count;
}

} // namespace

BaseModel::BaseModel() = default;

const char* BaseModel::app_title() const {
    return current_page_ == AppPage::Lipo ? "LiPo calculator" : "FPV Checklist";
}

bool BaseModel::dark_mode() const {
    return dark_mode_;
}

void BaseModel::set_dark_mode(bool enabled) {
    dark_mode_ = enabled;
}

void BaseModel::toggle_dark_mode() {
    dark_mode_ = !dark_mode_;
}

AppPage BaseModel::current_page() const {
    return current_page_;
}

void BaseModel::set_current_page(AppPage page) {
    current_page_ = page;
}

void BaseModel::toggle_page() {
    current_page_ = current_page_ == AppPage::Checklist ? AppPage::Lipo : AppPage::Checklist;
}

// ---- Checklists -----------------------------------------------------------

int BaseModel::active_list() const {
    return active_list_;
}

void BaseModel::switch_list(int delta) {
    active_list_ = wrap(active_list_ + delta, static_cast<int>(kChecklists.size()));
}

const ChecklistDef& BaseModel::active_def() const {
    return kChecklists[active_list_];
}

int BaseModel::selected_item() const {
    return selected_[active_list_];
}

void BaseModel::move_selection(int delta) {
    const int count = static_cast<int>(active_def().count);
    selected_[active_list_] = wrap(selected_[active_list_] + delta, count);
}

bool BaseModel::is_checked(int list, int item) const {
    return (checked_[list] >> item) & 1U;
}

void BaseModel::toggle_selected() {
    checked_[active_list_] ^= 1U << selected_[active_list_];
}

void BaseModel::reset_active_list() {
    checked_[active_list_] = 0;
    selected_[active_list_] = 0;
}

int BaseModel::checked_count(int list) const {
    int count = 0;
    for (size_t i = 0; i < kChecklists[list].count; ++i) {
        count += is_checked(list, static_cast<int>(i)) ? 1 : 0;
    }
    return count;
}

bool BaseModel::list_complete(int list) const {
    return checked_count(list) == static_cast<int>(kChecklists[list].count);
}

// ---- LiPo calculator ------------------------------------------------------

LipoField BaseModel::lipo_field() const {
    return lipo_field_;
}

void BaseModel::move_lipo_field(int delta) {
    lipo_field_ = static_cast<LipoField>(wrap(static_cast<int>(lipo_field_) + delta, kLipoFieldCount));
}

void BaseModel::adjust_lipo_field(int delta) {
    switch (lipo_field_) {
        case LipoField::Cells: {
            const int per_cell = cell_centivolts();
            cells_ = std::clamp(cells_ + delta, kMinCells, kMaxCells);
            pack_centivolts_ = per_cell * cells_;  // keep per-cell voltage when changing S
            break;
        }
        case LipoField::Voltage:
            pack_centivolts_ = std::clamp(pack_centivolts_ + delta * kVoltageStep,
                                          kMinCellCentivolts * cells_,
                                          kMaxCellCentivolts * cells_);
            break;
        case LipoField::Capacity:
            capacity_index_ = std::clamp(capacity_index_ + delta, 0, static_cast<int>(kCapacities.size()) - 1);
            break;
        case LipoField::Current: {
            const int step = (current_amps_ + (delta > 0 ? 0 : -1)) >= 20 ? 5 : 1;
            current_amps_ = std::clamp(current_amps_ + delta * step, 1, 200);
            break;
        }
    }
}

int BaseModel::cells() const {
    return cells_;
}

int BaseModel::pack_centivolts() const {
    return pack_centivolts_;
}

int BaseModel::cell_centivolts() const {
    return (pack_centivolts_ + cells_ / 2) / cells_;
}

int BaseModel::capacity_mah() const {
    return kCapacities[capacity_index_];
}

int BaseModel::current_amps() const {
    return current_amps_;
}

int BaseModel::charge_percent() const {
    const int v = cell_centivolts();
    if (v <= kSocCurve.front().centivolts) {
        return 0;
    }
    if (v >= kSocCurve.back().centivolts) {
        return 100;
    }
    for (size_t i = 1; i < kSocCurve.size(); ++i) {
        const auto& hi = kSocCurve[i];
        if (v <= hi.centivolts) {
            const auto& lo = kSocCurve[i - 1];
            return lo.percent + (v - lo.centivolts) * (hi.percent - lo.percent) / (hi.centivolts - lo.centivolts);
        }
    }
    return 100;
}

int BaseModel::flight_time_seconds() const {
    // usable mAh = 80 %, mAh / (A * 1000) h  ->  seconds
    const long usable_mah = capacity_mah() * 80L / 100L;
    return static_cast<int>(usable_mah * 3600L / (current_amps_ * 1000L));
}

LipoStatus BaseModel::lipo_status() const {
    const int v = cell_centivolts();
    if (v > 425) return LipoStatus::Overcharged;
    if (v >= 415) return LipoStatus::Full;
    if (v > 390) return LipoStatus::Good;
    if (v >= 375) return LipoStatus::Storage;
    if (v >= 350) return LipoStatus::Low;
    return LipoStatus::Critical;
}

// ---- Persistence ----------------------------------------------------------

std::string BaseModel::serialize() const {
    std::ostringstream out;
    out << "active_list=" << active_list_ << '\n';
    for (size_t i = 0; i < kChecklists.size(); ++i) {
        out << "checked" << i << '=' << checked_[i] << '\n';
        out << "selected" << i << '=' << selected_[i] << '\n';
    }
    out << "cells=" << cells_ << '\n';
    out << "pack_cv=" << pack_centivolts_ << '\n';
    out << "capacity_index=" << capacity_index_ << '\n';
    out << "current_a=" << current_amps_ << '\n';
    return out.str();
}

void BaseModel::deserialize(const std::string& text) {
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        const auto eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        const std::string key = line.substr(0, eq);
        const long value = std::strtol(line.c_str() + eq + 1, nullptr, 10);

        for (size_t i = 0; i < kChecklists.size(); ++i) {
            const uint32_t mask = (1U << kChecklists[i].count) - 1U;
            if (key == "checked" + std::to_string(i)) {
                checked_[i] = static_cast<uint32_t>(value) & mask;
            }
            else if (key == "selected" + std::to_string(i)) {
                selected_[i] = std::clamp(static_cast<int>(value), 0, static_cast<int>(kChecklists[i].count) - 1);
            }
        }
        if (key == "active_list") {
            active_list_ = std::clamp(static_cast<int>(value), 0, static_cast<int>(kChecklists.size()) - 1);
        }
        else if (key == "cells") {
            cells_ = std::clamp(static_cast<int>(value), kMinCells, kMaxCells);
        }
        else if (key == "pack_cv") {
            pack_centivolts_ = static_cast<int>(value);
        }
        else if (key == "capacity_index") {
            capacity_index_ = std::clamp(static_cast<int>(value), 0, static_cast<int>(kCapacities.size()) - 1);
        }
        else if (key == "current_a") {
            current_amps_ = std::clamp(static_cast<int>(value), 1, 200);
        }
    }
    pack_centivolts_ = std::clamp(pack_centivolts_, kMinCellCentivolts * cells_, kMaxCellCentivolts * cells_);
}

} // namespace model
