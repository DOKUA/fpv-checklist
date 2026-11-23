/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "subjects.h"
#include "base_model.h"

#include "lvgl.h"

#include <string>

namespace viewmodel {

class BaseViewModel {
public:
    BaseViewModel();
    ~BaseViewModel();

    BaseViewModel(const BaseViewModel&) = delete;
    BaseViewModel& operator=(const BaseViewModel&) = delete;

    lv_subject_t* title_subject();
    lv_subject_t* dark_mode_subject();
    lv_subject_t* current_page_subject();
    // Revision counters: bumped whenever checklist / LiPo state changes,
    // screens observe them and redraw.
    lv_subject_t* checklist_subject();
    lv_subject_t* lipo_subject();
    lv_subject_t* quit_requested_subject();

    const model::BaseModel& model() const;

    bool is_dark_mode() const;
    void set_dark_mode(bool enabled);
    void toggle_dark_mode();

    model::AppPage current_page() const;
    void show_checklist_page();
    void show_lipo_page();
    void toggle_page();
    void request_quit();

    // Checklist actions
    void move_selection(int delta);
    void toggle_selected();
    void switch_list(int delta);
    void reset_active_list();

    // LiPo actions
    void move_lipo_field(int delta);
    void adjust_lipo_field(int delta);

    // Persistence
    std::string save_state() const;
    void load_state(const std::string& text);

private:
    void publish_all();
    void bump(reactive::IntSubject& subject);

    model::BaseModel model_;
    reactive::StringSubject<32> title_subject_;
    reactive::BoolSubject dark_mode_subject_;
    reactive::IntSubject current_page_subject_;
    reactive::IntSubject checklist_subject_;
    reactive::IntSubject lipo_subject_;
    reactive::BoolSubject quit_requested_subject_;
};

} // namespace viewmodel
