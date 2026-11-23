/*
 * FPV Checklist — static reference data.
 *
 * Sources (checked Nov 2026):
 *  - Batteries / charging: oscarliang.com/lipo-battery-guide/,
 *    oscarliang.com/how-to-charge-lipo-batteries/, oscarliang.com/li-ion-battery-long-range/
 *  - Channels: oscarliang.com/fpv-channels/ (tables + chart, incl. DJI O3 rows)
 *  - DJI O4: DJI O4 Air Unit Series User Manual v1.0 (2025.01), section 4.4
 *  - Props: oscarliang.com/table-prop-motor-lipo-weight/
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "i18n.h"

#include <array>
#include <cstddef>

namespace model {

// ---------------------------------------------------------------- Checklists

struct ChecklistDef {
    Text title;
    const Text* items;
    size_t count;
};

namespace detail {

inline constexpr Text kBenchItems[] = {
    {"Batteries charged & balanced", "Батареї заряджені й збалансовані"},
    {"Goggles & radio charged", "Окуляри й пульт заряджені"},
    {"Props: no chips or cracks", "Пропи без сколів і тріщин"},
    {"Prop nuts tight", "Гайки пропів затягнуті"},
    {"Frame & arm screws tight", "Гвинти рами й променів затягнуті"},
    {"Antennas secured (VTX + RX)", "Антени закріплені (VTX + RX)"},
    {"Camera lens clean, angle set", "Лінза чиста, кут камери ок"},
    {"Correct model on radio", "Правильна модель на пульті"},
    {"Failsafe tested", "Failsafe перевірено"},
    {"SD card in goggles / camera", "SD-карта в окулярах / камері"},
    {"Spare props, tools, straps", "Запасні пропи, інструмент, стрепи"},
};

inline constexpr Text kFieldItems[] = {
    {"Area clear of people", "Поруч немає людей"},
    {"Wind & weather OK", "Вітер і погода ок"},
    {"Ask pilots which channels are used", "Спитай пілотів про зайняті канали"},
    {"Goggles ON, channel free", "Окуляри ввімкнені, канал вільний"},
    {"Arm switch OFF", "Тумблер ARM вимкнено"},
    {"Battery strapped tight", "Батарея туго закріплена"},
    {"Plug in, check voltage", "Підключи, перевір напругу"},
    {"Video & OSD OK", "Відео й OSD ок"},
    {"GPS lock / home set", "GPS-фікс / home point"},
    {"Beeper & turtle mode set", "Біпер і turtle mode"},
    {"Short hover test", "Коротке тестове зависання"},
};

inline constexpr Text kPostItems[] = {
    {"Disarm, unplug battery", "Disarm, відключи батарею"},
    {"Check motor temperature", "Перевір температуру моторів"},
    {"Inspect props & frame", "Огляд пропів і рами"},
    {"Let battery cool before charging", "Дай батареї охолонути"},
    {"Note battery voltage", "Запиши напругу батареї"},
    {"Save DVR / footage", "Збережи DVR / відео"},
    {"Not flying soon? Storage 3.8V/cell", "Не літаєш скоро? Storage 3.8V/банку"},
};

// Oscar: "Inspect your batteries after crashes".
inline constexpr Text kCrashItems[] = {
    {"Battery: no dents or punctures", "Батарея: без вм'ятин і проколів"},
    {"Battery: no swelling or heat", "Батарея: без здуття й нагріву"},
    {"Wires & connectors intact", "Дроти й роз'єми цілі"},
    {"Balance lead not damaged", "Балансувальний роз'єм цілий"},
    {"Bent props replaced", "Погнуті пропи замінено"},
    {"Motors spin freely", "Мотори крутяться вільно"},
    {"Camera & antennas OK", "Камера й антени ок"},
};

} // namespace detail

inline constexpr std::array<ChecklistDef, 4> kChecklists = {{
    {{"Bench", "Дім"}, detail::kBenchItems, std::size(detail::kBenchItems)},
    {{"Field", "Поле"}, detail::kFieldItems, std::size(detail::kFieldItems)},
    {{"Post", "Після"}, detail::kPostItems, std::size(detail::kPostItems)},
    {{"Crash", "Краш"}, detail::kCrashItems, std::size(detail::kCrashItems)},
}};

inline constexpr size_t kMaxChecklistItems = 16;

// ---------------------------------------------------------------- Batteries

enum class Chem {
    LiPo = 0,
    LiHV = 1,
    LiIon = 2,
};
inline constexpr int kChemCount = 3;

struct ChemInfo {
    const char* name;
    int full_cv;        // max charge voltage per cell, centivolts
    int storage_lo_cv;  // storage range per cell (0 = see datasheet)
    int storage_hi_cv;
    int land_lo_cv;     // typical landing guidance (resting / light load)
    int land_hi_cv;
    int min_cv;         // lower limit — not a landing target
    Text charge_tip;
};

inline constexpr std::array<ChemInfo, kChemCount> kChems = {{
    {"LiPo", 420, 380, 385, 350, 360, 300,
     {"Never LiHV mode for LiPo!", "Ніколи не LiHV для LiPo!"}},
    {"LiHV", 435, 380, 385, 350, 360, 300,
     {"Only LiHV-rated packs!", "Лише для паків LiHV!"}},
    {"Li-ion", 420, 0, 0, 300, 300, 280,
     {"See cell datasheet, 0.5-1C", "Див. datasheet, 0.5-1C"}},
}};

// ---------------------------------------------------------------- Channels

struct ChannelSystem {
    const char* name;   // row label
    const char* full;   // long name for info line
    bool analog;
    int bw;             // MHz
    const int* freq10;  // centre frequencies, tenths of MHz
    const char* const* labels;
    int count;
};

namespace detail {

inline constexpr const char* kBandLabelsA[] = {"A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8"};
inline constexpr const char* kBandLabelsB[] = {"B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8"};
inline constexpr const char* kBandLabelsE[] = {"E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8"};
inline constexpr const char* kBandLabelsF[] = {"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8"};
inline constexpr const char* kBandLabelsR[] = {"R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8"};
inline constexpr const char* kChLabels[] = {"CH1", "CH2", "CH3", "CH4", "CH5", "CH6", "CH7", "CH8"};

inline constexpr int kBandA[] = {58650, 58450, 58250, 58050, 57850, 57650, 57450, 57250};
inline constexpr int kBandB[] = {57330, 57520, 57710, 57900, 58090, 58280, 58470, 58660};
inline constexpr int kBandE[] = {57050, 56850, 56650, 56450, 58850, 59050, 59250, 59450};
inline constexpr int kBandF[] = {57400, 57600, 57800, 58000, 58200, 58400, 58600, 58800};
inline constexpr int kBandR[] = {56580, 56950, 57320, 57690, 58060, 58430, 58800, 59170};
// DJI V1 / Vista and Walksnail Avatar share these (FCC). CH8 = public channel.
inline constexpr int kDjiV1_25[] = {56600, 56950, 57350, 57700, 58050, 58780, 59140, 58390};
inline constexpr int kDjiV1_50[] = {56950, 57700, 58780, 58390};
inline constexpr const char* kDjiV1_50Labels[] = {"CH1", "CH2", "CH3", "CH8"};
inline constexpr int kDjiO3_20[] = {56690, 57050, 57680, 58040, 58390, 58760, 59120};
inline constexpr int kDjiO3_40[] = {56770, 57940, 59020};
inline constexpr int kDjiO4_20[] = {57685, 57895, 58145};
inline constexpr int kDjiO4_40[] = {57945};
inline constexpr int kHdZero[] = {56580, 56950, 57320, 57690, 58060, 58430, 58800, 59170};

} // namespace detail

inline constexpr std::array<ChannelSystem, 12> kChannelSystems = {{
    {"A", "Band A", true, 30, detail::kBandA, detail::kBandLabelsA, 8},
    {"B", "Band B", true, 30, detail::kBandB, detail::kBandLabelsB, 8},
    {"E", "Band E", true, 30, detail::kBandE, detail::kBandLabelsE, 8},
    {"F", "Band F", true, 30, detail::kBandF, detail::kBandLabelsF, 8},
    {"R", "Raceband", true, 30, detail::kBandR, detail::kBandLabelsR, 8},
    {"V1/AV", "DJI V1/Avatar 25M", false, 20, detail::kDjiV1_25, detail::kChLabels, 8},
    {"V1 50", "DJI V1/Avatar 50M", false, 40, detail::kDjiV1_50, detail::kDjiV1_50Labels, 4},
    {"O3 20", "DJI O3 10/20MHz", false, 20, detail::kDjiO3_20, detail::kChLabels, 7},
    {"O3 40", "DJI O3 40MHz", false, 40, detail::kDjiO3_40, detail::kChLabels, 3},
    {"O4 20", "DJI O4 10/20MHz", false, 20, detail::kDjiO4_20, detail::kChLabels, 3},
    {"O4 40", "DJI O4 40MHz", false, 40, detail::kDjiO4_40, detail::kChLabels, 1},
    {"HDZ", "HDZero", false, 27, detail::kHdZero, detail::kBandLabelsR, 8},
}};

inline constexpr int kSysA = 0, kSysB = 1, kSysE = 2, kSysF = 3, kSysR = 4, kSysDjiV1 = 5, kSysDjiV1_50 = 6;

struct ChannelRef {
    int sys;
    int ch;
};

struct GroupPreset {
    Text name;
    const ChannelRef* channels;
    int count;
};

namespace detail {
inline constexpr ChannelRef kGroup2[] = {{kSysR, 0}, {kSysR, 7}};
inline constexpr ChannelRef kGroup3[] = {{kSysR, 0}, {kSysR, 3}, {kSysR, 7}};
inline constexpr ChannelRef kGroup4[] = {{kSysR, 0}, {kSysR, 2}, {kSysR, 5}, {kSysR, 7}};
inline constexpr ChannelRef kGroup5[] = {{kSysR, 0}, {kSysR, 1}, {kSysF, 1}, {kSysF, 3}, {kSysE, 4}};
inline constexpr ChannelRef kGroup6[] = {{kSysE, 3}, {kSysE, 1}, {kSysF, 1}, {kSysA, 3}, {kSysE, 5}, {kSysE, 7}};
inline constexpr ChannelRef kGroupNarrow[] = {{kSysR, 2}, {kSysR, 3}, {kSysB, 5}, {kSysA, 0}};
} // namespace detail

// Oscar's recommended analog group sets (separation + IMD considered).
inline constexpr std::array<GroupPreset, 6> kGroupPresets = {{
    {{"2 pilots", "2 пілоти"}, detail::kGroup2, 2},
    {{"3 pilots", "3 пілоти"}, detail::kGroup3, 3},
    {{"4 pilots", "4 пілоти"}, detail::kGroup4, 4},
    {{"5 pilots", "5 пілотів"}, detail::kGroup5, 5},
    {{"6 pilots", "6 пілотів"}, detail::kGroup6, 6},
    {{"5725-5866 only", "Лише 5725-5866"}, detail::kGroupNarrow, 4},
}};

// ---------------------------------------------------------------- Props

struct PropVariant {
    Text cls;
    const char* blades;
    const char* cells;
    const char* motors;
    const char* kv;
    const char* battery;
    const char* dry;
};

struct PropSize {
    const char* label;   // e.g. 5" or 31mm
    int tenth_inch;      // 50 == 5.0"
    int mm;              // diameter in mm
    const PropVariant* variants;
    int count;
};

namespace detail {

inline constexpr Text kWhoop65{"Tiny whoop 65mm", "Тайні вуп 65мм"};
inline constexpr Text kWhoop75{"Tiny whoop 75mm", "Тайні вуп 75мм"};
inline constexpr Text kCine{"Cinewhoop", "Сіневуп"};
inline constexpr Text kUltra{"Ultralight", "Ультралайт"};
inline constexpr Text kFree{"Freestyle", "Фрістайл"};
inline constexpr Text kRace{"Racing", "Рейсинг"};
inline constexpr Text kLong{"Long range", "Лонг-рендж"};

inline constexpr PropVariant kP31[] = {
    {kWhoop65, "Tri", "1S", "0702, 0802", "23000-30000", "260-450", "16-22g"},
    {kWhoop65, "Two", "1S", "0702, 0802", "30000+", "300", "16-22g"},
};
inline constexpr PropVariant kP40[] = {
    {kWhoop75, "Tri", "1S", "0802, 1002", "20000-23000", "450-550", "20-30g"},
    {kWhoop75, "Tri", "2S", "0802, 1002, 1102", "12000-14000", "450-650", "30-50g"},
    {kWhoop75, "Tri", "3S", "1102, 1103", "8000-11000", "300-650", "40-70g"},
    {kWhoop75, "Two", "1S", "0802, 1002", "23000-25000", "450-550", "20-30g"},
    {kWhoop75, "Two", "2S", "0802, 1002", "14000+", "450-650", "30-50g"},
    {kUltra, "Tri", "1S", "1002", "21000-23000", "450-650", "30g"},
};
inline constexpr PropVariant kP2[] = {
    {kCine, "Tri", "2S", "1003, 1103", "11000-15000", "450-650", "70-90g"},
    {kCine, "Tri", "3S", "1103, 1104, 1203, 1303", "6000-7500", "450-650", "80-100g"},
    {kCine, "Tri", "4S", "1303, 1304", "5000-6000", "450-720", "120-140g"},
    {kUltra, "Tri", "1S", "1002, 1003, 1102", "20000-23000", "450-650", "30-40g"},
    {kUltra, "Tri", "2S", "1002, 1003, 1103", "10000-14000", "450-650", "30-50g"},
    {kUltra, "Tri", "3S", "1103, 1105", "6000-8000", "300-450", "60-100g"},
    {kUltra, "Tri", "4S", "1105, 1106", "5000-6000", "450-650", "70-130g"},
};
inline constexpr PropVariant kP25[] = {
    {kUltra, "Tri", "2S", "1203, 1204, 1303", "7500", "450-650", "55-70g"},
    {kUltra, "Tri", "3S", "1203, 1204, 1303", "6000", "450-650", "120-140g"},
    {kUltra, "Tri", "4S", "1404", "4500", "450-850", "140-180g"},
    {kCine, "Tri", "4S", "1404", "4500-5000", "650-850", "140-180g"},
    {kUltra, "Two", "1S", "1102, 1103", "13000-15000", "300-450", "25-35g"},
    {kUltra, "Two", "2S", "1203, 1204, 1303", "7500-10000", "450-550", "40-60g"},
    {kUltra, "Two", "3S", "1203, 1204, 1303", "6000-8000", "450-550", "45-65g"},
    {kUltra, "Two", "4S", "1303, 1404", "4500-6000", "450-650", "55-90g"},
};
inline constexpr PropVariant kP3[] = {
    {kFree, "Tri", "2S", "1303, 1404", "4500-5500", "550-650", "70-100g"},
    {kFree, "Tri", "3S", "1303, 1404, 1407", "4000-5000", "550-850", "90-200g"},
    {kFree, "Tri", "4S", "1404, 1407, 1506", "3000-4200", "450-850", "140-260g"},
    {kFree, "Tri", "6S", "1506, 1507", "2700-3000", "550-850", "150-280g"},
    {kLong, "Two", "1S", "1103, 1202, 1203", "10000-12000", "Li-ion 2500-3000", "60-80g"},
    {kUltra, "Two", "1S", "1202.5, 1203, 1303", "11000-14000", "450", "30-50g"},
    {kUltra, "Two", "2S", "1203, 1204, 1303", "6000-8000", "450", "40-60g"},
    {kUltra, "Two", "3S", "1203, 1204, 1303, 1304", "4500-6500", "300-450", "55-75g"},
    {kUltra, "Two", "4S", "1303, 1304, 1404", "3500-5000", "450-550", "60-80g"},
    {kCine, "-", "4S", "1404, 2004", "3000-4000", "850-1300", "180-300g"},
    {kCine, "-", "6S", "1404, 2004", "2500-3000", "850-1300", "180-300g"},
};
inline constexpr PropVariant kP35[] = {
    {kCine, "-", "4S", "2004, 2006, 2106", "2500-3500", "1300-1500", "250-350g"},
    {kCine, "-", "6S", "2004, 2006, 2106", "1800-2500", "1000-1300", "250-350g"},
    {kFree, "Tri", "4S", "1404, 1504, 1604, 2004", "3500-4000", "850-1000", "150-200g"},
    {kFree, "Tri", "6S", "1504, 1604, 2004, 2006", "2500-3000", "850-1000", "150-200g"},
};
inline constexpr PropVariant kP4[] = {
    {kFree, "Tri", "4S", "2004", "2400-3000", "850-1000", "120-200g"},
    {kFree, "Tri", "6S", "2004, 2106", "1800-2500", "650-850", "120-180g"},
    {kLong, "Two", "3S", "1404, 1504, 1604", "3000-4000", "Li-ion 2500-3000", "120-150g"},
    {kLong, "Two", "4S", "1404, 1504, 1604", "2500-3000", "Li-ion 2500-3000", "150-200g"},
};
inline constexpr PropVariant kP5[] = {
    {kFree, "Tri", "4S", "2306, 2207, 2208, 2308", "2300-2700", "1300-1500", "300-450g"},
    {kFree, "Tri", "6S", "2306, 2207, 2208, 2308", "1700-2100", "1000-1300", "300-450g"},
    {kFree, "Tri", "8S", "2306, 2207, 2208, 2308", "1500-1700", "1000-1100", "300-450g"},
    {kRace, "Tri", "4S", "2306, 2207, 2208, 2308", "2500-3000", "1300-1500", "250-300g"},
    {kRace, "Tri", "6S", "2306, 2207, 2208, 2308", "1900-2300", "1000-1300", "250-300g"},
    {kUltra, "Two", "4S", "2004, 2204, 2205", "2300-3000", "850-1000", "150-250g"},
    {kUltra, "Two", "6S", "2004, 2204, 2205", "1600-2300", "650-850", "150-250g"},
};
inline constexpr PropVariant kP6[] = {
    {kFree, "-", "4S", "2207, 2208, 2308, 2407", "2100-2500", "1300-1800", "300-450g"},
    {kFree, "-", "6S", "2207, 2208, 2308, 2407", "1500-1900", "1000-1500", "300-450g"},
};
inline constexpr PropVariant kP7[] = {
    {kFree, "-", "6S", "2806, 2807, 2808, 3106", "1200-1400", "2200-3000", "400-500g"},
    {kLong, "-", "4S", "2806, 2807, 2808, 3106", "1600-1900", "Li-ion 2500-4000", "350-500g"},
    {kLong, "-", "6S", "2806, 2807, 2808, 3106", "980-1300", "Li-ion 2500-4000", "350-500g"},
};
inline constexpr PropVariant kP8[] = {
    {kLong, "-", "6S", "2808, 2810, 2814", "900-1200", "-", "-"},
};
inline constexpr PropVariant kP10[] = {
    {kLong, "-", "6S", "2814, 3110, 3115, 3214", "900", "-", "-"},
    {kLong, "-", "12S", "2814, 3110, 3115, 3214", "450", "-", "-"},
};
inline constexpr PropVariant kP13[] = {
    {kLong, "-", "12S", "-", "330-360", "-", "-"},
};
inline constexpr PropVariant kP15[] = {
    {kLong, "-", "12S", "-", "250", "-", "-"},
};

} // namespace detail

inline constexpr std::array<PropSize, 26> kPropSizes = {{
    {"31mm", 12, 31, detail::kP31, 2},
    {"40mm", 16, 40, detail::kP40, 6},
    {"45mm", 18, 45, nullptr, 0},
    {"2\"", 20, 51, detail::kP2, 7},
    {"2.5\"", 25, 64, detail::kP25, 8},
    {"3\"", 30, 76, detail::kP3, 11},
    {"3.5\"", 35, 89, detail::kP35, 4},
    {"4\"", 40, 102, detail::kP4, 4},
    {"5\"", 50, 127, detail::kP5, 7},
    {"5.1\"", 51, 130, nullptr, 0},
    {"5.5\"", 55, 140, nullptr, 0},
    {"6\"", 60, 152, detail::kP6, 2},
    {"7\"", 70, 178, detail::kP7, 3},
    {"8\"", 80, 203, detail::kP8, 1},
    {"9\"", 90, 229, nullptr, 0},
    {"10\"", 100, 254, detail::kP10, 2},
    {"11\"", 110, 279, nullptr, 0},
    {"12\"", 120, 305, nullptr, 0},
    {"13\"", 130, 330, detail::kP13, 1},
    {"14\"", 140, 356, nullptr, 0},
    {"15\"", 150, 381, detail::kP15, 1},
    {"16\"", 160, 406, nullptr, 0},
    {"17\"", 170, 432, nullptr, 0},
    {"18\"", 180, 457, nullptr, 0},
    {"19\"", 190, 483, nullptr, 0},
    {"20\"", 200, 508, nullptr, 0},
}};

} // namespace model
