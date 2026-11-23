/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include "base_viewmodel.h"

namespace viewmodel {
namespace {

int page_to_int(model::AppPage page) {
    return static_cast<int>(page);
}

} // namespace

BaseViewModel::BaseViewModel()
    : title_subject_(model_.app_title()),
      dark_mode_subject_(model_.dark_mode()),
      current_page_subject_(page_to_int(model_.current_page())),
      checklist_subject_(0),
      lipo_subject_(0),
      quit_requested_subject_(false) {}

BaseViewModel::~BaseViewModel() = default;

lv_subject_t* BaseViewModel::title_subject() {
    return title_subject_.native();
}

lv_subject_t* BaseViewModel::dark_mode_subject() {
    return dark_mode_subject_.native();
}

lv_subject_t* BaseViewModel::current_page_subject() {
    return current_page_subject_.native();
}

lv_subject_t* BaseViewModel::checklist_subject() {
    return checklist_subject_.native();
}

lv_subject_t* BaseViewModel::lipo_subject() {
    return lipo_subject_.native();
}

lv_subject_t* BaseViewModel::quit_requested_subject() {
    return quit_requested_subject_.native();
}

const model::BaseModel& BaseViewModel::model() const {
    return model_;
}

bool BaseViewModel::is_dark_mode() const {
    return model_.dark_mode();
}

void BaseViewModel::set_dark_mode(bool enabled) {
    model_.set_dark_mode(enabled);
    publish_all();
}

void BaseViewModel::toggle_dark_mode() {
    model_.toggle_dark_mode();
    publish_all();
}

model::AppPage BaseViewModel::current_page() const {
    return model_.current_page();
}

void BaseViewModel::show_checklist_page() {
    model_.set_current_page(model::AppPage::Checklist);
    publish_all();
}

void BaseViewModel::show_lipo_page() {
    model_.set_current_page(model::AppPage::Lipo);
    publish_all();
}

void BaseViewModel::toggle_page() {
    model_.toggle_page();
    publish_all();
}

void BaseViewModel::request_quit() {
    quit_requested_subject_.set(true);
}

void BaseViewModel::move_selection(int delta) {
    model_.move_selection(delta);
    bump(checklist_subject_);
}

void BaseViewModel::toggle_selected() {
    model_.toggle_selected();
    bump(checklist_subject_);
}

void BaseViewModel::switch_list(int delta) {
    model_.switch_list(delta);
    bump(checklist_subject_);
}

void BaseViewModel::reset_active_list() {
    model_.reset_active_list();
    bump(checklist_subject_);
}

void BaseViewModel::move_lipo_field(int delta) {
    model_.move_lipo_field(delta);
    bump(lipo_subject_);
}

void BaseViewModel::adjust_lipo_field(int delta) {
    model_.adjust_lipo_field(delta);
    bump(lipo_subject_);
}

std::string BaseViewModel::save_state() const {
    return model_.serialize();
}

void BaseViewModel::load_state(const std::string& text) {
    model_.deserialize(text);
    bump(checklist_subject_);
    bump(lipo_subject_);
}

void BaseViewModel::bump(reactive::IntSubject& subject) {
    subject.set(subject.value() + 1);
}

void BaseViewModel::publish_all() {
    title_subject_.set(model_.app_title());
    dark_mode_subject_.set(model_.dark_mode());
    current_page_subject_.set(page_to_int(model_.current_page()));
}

} // namespace viewmodel
