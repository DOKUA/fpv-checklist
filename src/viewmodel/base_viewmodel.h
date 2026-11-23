/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "subjects.h"
#include "base_model.h"

#include "lvgl.h"

#include <array>
#include <string>

namespace viewmodel {

struct HelpRow {
    const char* key;
    const char* action;
};

class BaseViewModel {
public:
    BaseViewModel();
    ~BaseViewModel();

    BaseViewModel(const BaseViewModel&) = delete;
    BaseViewModel& operator=(const BaseViewModel&) = delete;

    lv_subject_t* title_subject();
    lv_subject_t* dark_mode_subject();
    lv_subject_t* current_page_subject();
    // Revision counter: bumped on every state change; screens observe it and redraw.
    lv_subject_t* state_subject();
    lv_subject_t* quit_requested_subject();

    const model::BaseModel& model() const;

    bool is_dark_mode() const;
    void set_dark_mode(bool enabled);
    void toggle_dark_mode();
    void toggle_language();

    model::AppPage current_page() const;
    void show_page(model::AppPage page);
    void show_menu();
    void request_quit();

    // Keyboard: letters/arrows/Enter for the current page. Returns true if handled.
    bool on_key(uint32_t key);
    // Bottom bar (keys 4..8).
    std::array<const char*, 5> nav_icons() const;
    void nav_press(size_t index);
    std::array<HelpRow, 5> help_rows() const;

    std::string save_state() const;
    void load_state(const std::string& text);

private:
    void changed();
    bool menu_key(uint32_t key, int dx, int dy, bool select);

    model::BaseModel model_;
    reactive::StringSubject<48> title_subject_;
    reactive::BoolSubject dark_mode_subject_;
    reactive::IntSubject current_page_subject_;
    reactive::IntSubject state_subject_;
    reactive::BoolSubject quit_requested_subject_;
};

} // namespace viewmodel
