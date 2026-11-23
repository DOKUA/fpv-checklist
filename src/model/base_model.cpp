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

constexpr std::array<int, 18> kCapacities = {
    300, 450, 550, 650, 850, 1000, 1300, 1500, 1800, 2200, 2500, 3000, 4000, 4500, 5000, 6000, 8000, 10000,
};
constexpr std::array<int, 3> kRates = {5, 10, 20};  // tenths of C

constexpr int kMinCells = 1;
constexpr int kMaxCells = 12;
constexpr int kVoltageStep = 5;  // 0.05 V

// Resting LiPo voltage (centivolts per cell) -> state of charge.
struct SocPoint {
    int cv;
    int percent;
};
constexpr std::array<SocPoint, 21> kLipoCurve = {{
    {327, 0},  {361, 5},  {369, 10}, {371, 15}, {373, 20}, {375, 25}, {377, 30},
    {379, 35}, {380, 40}, {382, 45}, {384, 50}, {385, 55}, {387, 60}, {391, 65},
    {395, 70}, {398, 75}, {402, 80}, {408, 85}, {411, 90}, {415, 95}, {420, 100},
}};

int wrap(int value, int count) {
    return count <= 0 ? 0 : ((value % count) + count) % count;
}

int lipo_percent(int cv) {
    if (cv <= kLipoCurve.front().cv) return 0;
    if (cv >= kLipoCurve.back().cv) return 100;
    for (size_t i = 1; i < kLipoCurve.size(); ++i) {
        const auto& hi = kLipoCurve[i];
        if (cv <= hi.cv) {
            const auto& lo = kLipoCurve[i - 1];
            return lo.percent + (cv - lo.cv) * (hi.percent - lo.percent) / (hi.cv - lo.cv);
        }
    }
    return 100;
}

// Field ids used by the battery views.
enum BField { FType, FCells, FCapacity, FRate, FVoltage, FCurrent };
constexpr BField kChargeFields[] = {FType, FCells, FCapacity, FRate};
constexpr BField kCheckFields[] = {FType, FCells, FVoltage, FCapacity, FCurrent};
constexpr BField kInfoFields[] = {FType, FCells};

BField field_id(BatteryView view, int field) {
    switch (view) {
        case BatteryView::Charge: return kChargeFields[field];
        case BatteryView::Check: return kCheckFields[field];
        case BatteryView::Info: return kInfoFields[field];
    }
    return FType;
}

} // namespace

int capacity_at(int index) {
    return kCapacities[std::clamp(index, 0, static_cast<int>(kCapacities.size()) - 1)];
}

int capacity_count() {
    return static_cast<int>(kCapacities.size());
}

// ------------------------------------------------------------------ Battery

int BatteryState::field_count(BatteryView view) {
    switch (view) {
        case BatteryView::Charge: return static_cast<int>(std::size(kChargeFields));
        case BatteryView::Check: return static_cast<int>(std::size(kCheckFields));
        case BatteryView::Info: return static_cast<int>(std::size(kInfoFields));
    }
    return 1;
}

void BatteryState::move_field(int delta) {
    field = wrap(field + delta, field_count(view));
}

void BatteryState::switch_view(int delta) {
    view = static_cast<BatteryView>(wrap(static_cast<int>(view) + delta, kBatteryViewCount));
    field = std::min(field, field_count(view) - 1);
}

void BatteryState::adjust(int delta) {
    switch (field_id(view, field)) {
        case FType:
            chem = static_cast<Chem>(wrap(static_cast<int>(chem) + delta, kChemCount));
            clamp_voltage();
            break;
        case FCells: {
            const int per_cell = cell_cv();
            cells = std::clamp(cells + delta, kMinCells, kMaxCells);
            pack_cv = per_cell * cells;
            clamp_voltage();
            break;
        }
        case FCapacity:
            capacity_index = std::clamp(capacity_index + delta, 0, capacity_count() - 1);
            break;
        case FRate:
            rate_index = std::clamp(rate_index + delta, 0, static_cast<int>(kRates.size()) - 1);
            break;
        case FVoltage:
            pack_cv += delta * kVoltageStep;
            clamp_voltage();
            break;
        case FCurrent: {
            const int step = (current_a + (delta > 0 ? 0 : -1)) >= 20 ? 5 : 1;
            current_a = std::clamp(current_a + delta * step, 1, 200);
            break;
        }
    }
}

void BatteryState::clamp_voltage() {
    pack_cv = std::clamp(pack_cv, 250 * cells, 445 * cells);
}

const ChemInfo& BatteryState::chem_info() const {
    return kChems[static_cast<int>(chem)];
}

int BatteryState::capacity_mah() const {
    return capacity_at(capacity_index);
}

int BatteryState::rate_tenths() const {
    return kRates[rate_index];
}

int BatteryState::charge_current_ma() const {
    return capacity_mah() * rate_tenths() / 10;
}

int BatteryState::cell_cv() const {
    return (pack_cv + cells / 2) / cells;
}

int BatteryState::charge_percent() const {
    const int cv = cell_cv();
    switch (chem) {
        case Chem::LiPo:
            return lipo_percent(cv);
        case Chem::LiHV:
            // Stretch the LiPo curve so 4.35 V maps to 100 % (estimate).
            return lipo_percent(327 + (cv - 327) * (420 - 327) / (435 - 327));
        case Chem::LiIon:
            // Li-ion discharge is flatter/wider: rough linear estimate 2.8 V..4.2 V.
            return std::clamp((cv - 280) * 100 / (420 - 280), 0, 100);
    }
    return 0;
}

int BatteryState::flight_time_s() const {
    const long usable_mah = capacity_mah() * 80L / 100L;
    return static_cast<int>(usable_mah * 3600L / (current_a * 1000L));
}

BatteryStatus BatteryState::status() const {
    const int v = cell_cv();
    const auto& c = chem_info();
    if (v > c.full_cv + 5) return BatteryStatus::Overcharged;
    if (v >= c.full_cv - 5) return BatteryStatus::Full;
    if (chem == Chem::LiIon) {
        if (v > 330) return BatteryStatus::Ready;
        if (v >= c.land_lo_cv) return BatteryStatus::Low;
        return BatteryStatus::Critical;
    }
    if (v > 390) return BatteryStatus::Ready;
    if (v >= 375) return BatteryStatus::Storage;
    if (v >= c.land_lo_cv) return BatteryStatus::Low;
    return BatteryStatus::Critical;
}

// ------------------------------------------------------------------ Channels

int channel_freq10(const ChannelRef& ref) {
    return kChannelSystems[ref.sys].freq10[ref.ch];
}

bool channels_overlap(const ChannelRef& a, const ChannelRef& b) {
    const int distance = std::abs(channel_freq10(a) - channel_freq10(b));
    const int half_bw10 = (kChannelSystems[a.sys].bw + kChannelSystems[b.sys].bw) * 10 / 2;
    return distance < half_bw10;
}

bool find_imd(const std::vector<ChannelRef>& marked, ChannelRef& f1, ChannelRef& f2, ChannelRef& victim) {
    for (size_t i = 0; i < marked.size(); ++i) {
        for (size_t j = 0; j < marked.size(); ++j) {
            if (i == j) continue;
            const int product = 2 * channel_freq10(marked[i]) - channel_freq10(marked[j]);
            for (size_t k = 0; k < marked.size(); ++k) {
                if (k == i || k == j) continue;
                const int half_bw10 = kChannelSystems[marked[k].sys].bw * 10 / 2;
                if (std::abs(product - channel_freq10(marked[k])) < half_bw10) {
                    f1 = marked[i];
                    f2 = marked[j];
                    victim = marked[k];
                    return true;
                }
            }
        }
    }
    return false;
}

const ChannelSystem& ChannelState::system() const {
    return kChannelSystems[sys];
}

int ChannelState::freq10() const {
    return channel_freq10({sys, ch});
}

void ChannelState::move_system(int delta) {
    // Keep roughly the same frequency when jumping between rows.
    const int target = freq10();
    sys = wrap(sys + delta, static_cast<int>(kChannelSystems.size()));
    int best = 0;
    for (int i = 0; i < system().count; ++i) {
        if (std::abs(system().freq10[i] - target) < std::abs(system().freq10[best] - target)) {
            best = i;
        }
    }
    ch = best;
}

void ChannelState::move_channel(int delta) {
    // Move by frequency order (band tables are not always sorted, e.g. band A / E).
    const auto& s = system();
    const int current = s.freq10[ch];
    int best = -1;
    for (int i = 0; i < s.count; ++i) {
        const int f = s.freq10[i];
        const bool ahead = delta > 0 ? f > current : f < current;
        if (ahead && (best < 0 || std::abs(f - current) < std::abs(s.freq10[best] - current))) {
            best = i;
        }
    }
    if (best >= 0) {
        ch = best;
    }
}

bool ChannelState::is_marked(int s, int c) const {
    return std::any_of(marked.begin(), marked.end(), [&](const ChannelRef& r) { return r.sys == s && r.ch == c; });
}

void ChannelState::toggle_mark() {
    preset = -1;
    auto it = std::find_if(marked.begin(), marked.end(), [&](const ChannelRef& r) { return r.sys == sys && r.ch == ch; });
    if (it != marked.end()) {
        marked.erase(it);
    }
    else if (marked.size() < 8) {
        marked.push_back({sys, ch});
    }
}

void ChannelState::next_preset() {
    preset = preset + 1 >= static_cast<int>(kGroupPresets.size()) ? -1 : preset + 1;
    marked.clear();
    if (preset >= 0) {
        const auto& p = kGroupPresets[preset];
        marked.assign(p.channels, p.channels + p.count);
        sys = p.channels[0].sys;
        ch = p.channels[0].ch;
    }
}

void ChannelState::clear_marks() {
    marked.clear();
    preset = -1;
}

// ------------------------------------------------------------------ Props

void PropsState::move_size(int delta) {
    size = std::clamp(size + delta, 0, static_cast<int>(kPropSizes.size()) - 1);
    variant = 0;
}

void PropsState::move_variant(int delta) {
    const int count = kPropSizes[size].count;
    variant = count ? wrap(variant + delta, count) : 0;
}

// ------------------------------------------------------------------ Log

Pack* LogState::current() {
    return packs.empty() ? nullptr : &packs[selected];
}

void LogState::move(int delta) {
    delete_armed = false;
    if (!packs.empty()) {
        selected = std::clamp(selected + delta, 0, static_cast<int>(packs.size()) - 1);
    }
}

void LogState::add_pack() {
    delete_armed = false;
    if (static_cast<int>(packs.size()) >= kMaxPacks) {
        return;
    }
    Pack pack = packs.empty() ? Pack{} : packs[selected];  // copy spec of the selected pack
    pack.cycles = 0;
    pack.status = PackStatus::Ok;
    packs.push_back(pack);
    selected = static_cast<int>(packs.size()) - 1;
}

void LogState::delete_selected() {
    if (packs.empty()) {
        return;
    }
    if (!delete_armed) {
        delete_armed = true;  // second press confirms
        return;
    }
    packs.erase(packs.begin() + selected);
    selected = std::clamp(selected, 0, std::max(0, static_cast<int>(packs.size()) - 1));
    delete_armed = false;
    editing = false;
}

void LogState::adjust_cycles(int delta) {
    delete_armed = false;
    if (auto* p = current()) {
        p->cycles = std::clamp(p->cycles + delta, 0, 9999);
    }
}

void LogState::adjust_field(int delta) {
    auto* p = current();
    if (!p) return;
    switch (field) {
        case 0: p->chem = static_cast<Chem>(wrap(static_cast<int>(p->chem) + delta, kChemCount)); break;
        case 1: p->cells = std::clamp(p->cells + delta, kMinCells, kMaxCells); break;
        case 2: p->capacity_index = std::clamp(p->capacity_index + delta, 0, capacity_count() - 1); break;
        case 3: p->cycles = std::clamp(p->cycles + delta, 0, 9999); break;
        case 4: p->status = static_cast<PackStatus>(wrap(static_cast<int>(p->status) + delta, 3)); break;
        default: break;
    }
}

// ------------------------------------------------------------------ Model

BaseModel::BaseModel() = default;

const char* BaseModel::app_title() const {
    switch (current_page_) {
        case AppPage::Menu: return "FPV Toolkit";
        case AppPage::Checklist: return tr("Checklist", "Чекліст");
        case AppPage::Battery: return tr("Battery", "Батарея");
        case AppPage::Channels: return tr("Channels", "Канали");
        case AppPage::Props: return tr("Props", "Пропелери");
        case AppPage::Log: return tr("Battery log", "Журнал батарей");
    }
    return "FPV Toolkit";
}

void BaseModel::toggle_language() {
    set_language(language() == Lang::En ? Lang::Ua : Lang::En);
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
    selected_[active_list_] = wrap(selected_[active_list_] + delta, static_cast<int>(active_def().count));
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

// ------------------------------------------------------------------ Persistence

std::string BaseModel::serialize() const {
    std::ostringstream out;
    out << "lang=" << static_cast<int>(language()) << '\n';
    out << "menu=" << menu_index << '\n';
    out << "active_list=" << active_list_ << '\n';
    for (size_t i = 0; i < kChecklists.size(); ++i) {
        out << "checked" << i << '=' << checked_[i] << '\n';
        out << "selected" << i << '=' << selected_[i] << '\n';
    }
    out << "bat_view=" << static_cast<int>(battery.view) << '\n';
    out << "bat_chem=" << static_cast<int>(battery.chem) << '\n';
    out << "cells=" << battery.cells << '\n';
    out << "pack_cv=" << battery.pack_cv << '\n';
    out << "capacity_index=" << battery.capacity_index << '\n';
    out << "rate_index=" << battery.rate_index << '\n';
    out << "current_a=" << battery.current_a << '\n';
    out << "ch_sys=" << channels.sys << '\n';
    out << "ch_ch=" << channels.ch << '\n';
    out << "ch_marks=";
    for (const auto& m : channels.marked) out << m.sys << ':' << m.ch << ',';
    out << '\n';
    out << "prop_size=" << props.size << '\n';
    out << "log_sel=" << log.selected << '\n';
    for (const auto& p : log.packs) {
        out << "pack=" << static_cast<int>(p.chem) << ',' << p.cells << ',' << p.capacity_index << ',' << p.cycles
            << ',' << static_cast<int>(p.status) << '\n';
    }
    return out.str();
}

void BaseModel::deserialize(const std::string& text) {
    std::istringstream in(text);
    std::string line;
    log.packs.clear();
    while (std::getline(in, line)) {
        const auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        const std::string key = line.substr(0, eq);
        const std::string val = line.substr(eq + 1);
        const long v = std::strtol(val.c_str(), nullptr, 10);
        const auto clampi = [&](int lo, int hi) { return std::clamp(static_cast<int>(v), lo, hi); };

        for (size_t i = 0; i < kChecklists.size(); ++i) {
            const uint32_t mask = (1U << kChecklists[i].count) - 1U;
            if (key == "checked" + std::to_string(i)) checked_[i] = static_cast<uint32_t>(v) & mask;
            if (key == "selected" + std::to_string(i)) selected_[i] = clampi(0, static_cast<int>(kChecklists[i].count) - 1);
        }
        if (key == "lang") set_language(clampi(0, 1) ? Lang::Ua : Lang::En);
        else if (key == "menu") menu_index = clampi(0, kMenuItemCount - 1);
        else if (key == "active_list") active_list_ = clampi(0, static_cast<int>(kChecklists.size()) - 1);
        else if (key == "bat_view") battery.view = static_cast<BatteryView>(clampi(0, kBatteryViewCount - 1));
        else if (key == "bat_chem") battery.chem = static_cast<Chem>(clampi(0, kChemCount - 1));
        else if (key == "cells") battery.cells = clampi(kMinCells, kMaxCells);
        else if (key == "pack_cv") battery.pack_cv = static_cast<int>(v);
        else if (key == "capacity_index") battery.capacity_index = clampi(0, capacity_count() - 1);
        else if (key == "rate_index") battery.rate_index = clampi(0, static_cast<int>(kRates.size()) - 1);
        else if (key == "current_a") battery.current_a = clampi(1, 200);
        else if (key == "ch_sys") channels.sys = clampi(0, static_cast<int>(kChannelSystems.size()) - 1);
        else if (key == "ch_ch") channels.ch = static_cast<int>(v);
        else if (key == "prop_size") props.size = clampi(0, static_cast<int>(kPropSizes.size()) - 1);
        else if (key == "log_sel") log.selected = static_cast<int>(v);
        else if (key == "ch_marks") {
            channels.marked.clear();
            std::istringstream marks(val);
            std::string item;
            while (std::getline(marks, item, ',')) {
                const auto colon = item.find(':');
                if (colon == std::string::npos) continue;
                const int s = std::atoi(item.substr(0, colon).c_str());
                const int c = std::atoi(item.substr(colon + 1).c_str());
                if (s >= 0 && s < static_cast<int>(kChannelSystems.size()) && c >= 0 && c < kChannelSystems[s].count) {
                    channels.marked.push_back({s, c});
                }
            }
        }
        else if (key == "pack" && static_cast<int>(log.packs.size()) < kMaxPacks) {
            Pack p;
            int f[5] = {0, 6, 6, 0, 0};
            std::istringstream parts(val);
            std::string item;
            for (int i = 0; i < 5 && std::getline(parts, item, ','); ++i) f[i] = std::atoi(item.c_str());
            p.chem = static_cast<Chem>(std::clamp(f[0], 0, kChemCount - 1));
            p.cells = std::clamp(f[1], kMinCells, kMaxCells);
            p.capacity_index = std::clamp(f[2], 0, capacity_count() - 1);
            p.cycles = std::clamp(f[3], 0, 9999);
            p.status = static_cast<PackStatus>(std::clamp(f[4], 0, 2));
            log.packs.push_back(p);
        }
    }
    battery.clamp_voltage();
    channels.ch = std::clamp(channels.ch, 0, channels.system().count - 1);
    log.selected = std::clamp(log.selected, 0, std::max(0, static_cast<int>(log.packs.size()) - 1));
}

} // namespace model
