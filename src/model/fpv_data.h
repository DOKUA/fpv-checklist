/*
 * FPV Checklist — static checklist content.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <array>
#include <cstddef>

namespace model {

struct ChecklistDef {
    const char* title;
    const char* const* items;
    size_t count;
};

namespace detail {

inline constexpr const char* kBenchItems[] = {
    "Batteries charged & balanced",
    "Goggles & radio charged",
    "Props: no chips or cracks",
    "Prop nuts tight",
    "Frame & arm screws tight",
    "Antennas secured (VTX + RX)",
    "Camera lens clean, angle set",
    "Correct model on radio",
    "Failsafe tested",
    "SD card in goggles / camera",
    "Spare props, tools, straps",
};

inline constexpr const char* kFieldItems[] = {
    "Area clear of people",
    "Wind & weather OK",
    "Goggles ON, channel free",
    "Arm switch OFF",
    "Battery strapped tight",
    "Plug in, check voltage",
    "Video & OSD OK",
    "GPS lock / home set",
    "Beeper & turtle mode set",
    "Short hover test",
};

inline constexpr const char* kPostItems[] = {
    "Disarm, unplug battery",
    "Check motor temperature",
    "Inspect props & frame",
    "Note battery voltage",
    "Save DVR / footage",
    "Batteries to storage (3.8V/cell)",
};

} // namespace detail

inline constexpr std::array<ChecklistDef, 3> kChecklists = {{
    {"Bench", detail::kBenchItems, std::size(detail::kBenchItems)},
    {"Field", detail::kFieldItems, std::size(detail::kFieldItems)},
    {"Post-flight", detail::kPostItems, std::size(detail::kPostItems)},
}};

inline constexpr size_t kMaxChecklistItems = 16;

} // namespace model
