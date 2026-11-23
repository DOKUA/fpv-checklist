/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include "base_viewmodel.h"

#include "ui_const.h"

namespace viewmodel {
namespace {

using model::AppPage;
using model::tr;

int page_to_int(AppPage page) {
    return static_cast<int>(page);
}

constexpr int kMenuColumns = 3;

constexpr AppPage kMenuPages[model::kMenuItemCount] = {
    AppPage::Checklist, AppPage::Battery, AppPage::Channels,
    AppPage::Props,     AppPage::Log,     AppPage::Menu,  // last tile = language toggle
};

} // namespace

BaseViewModel::BaseViewModel()
    : title_subject_(model_.app_title()),
      dark_mode_subject_(model_.dark_mode()),
      current_page_subject_(page_to_int(model_.current_page())),
      state_subject_(0),
      quit_requested_subject_(false) {}

BaseViewModel::~BaseViewModel() = default;

lv_subject_t* BaseViewModel::title_subject() { return title_subject_.native(); }
lv_subject_t* BaseViewModel::dark_mode_subject() { return dark_mode_subject_.native(); }
lv_subject_t* BaseViewModel::current_page_subject() { return current_page_subject_.native(); }
lv_subject_t* BaseViewModel::state_subject() { return state_subject_.native(); }
lv_subject_t* BaseViewModel::quit_requested_subject() { return quit_requested_subject_.native(); }

const model::BaseModel& BaseViewModel::model() const {
    return model_;
}

bool BaseViewModel::is_dark_mode() const {
    return model_.dark_mode();
}

void BaseViewModel::set_dark_mode(bool enabled) {
    model_.set_dark_mode(enabled);
    dark_mode_subject_.set(enabled);
}

void BaseViewModel::toggle_dark_mode() {
    set_dark_mode(!model_.dark_mode());
}

void BaseViewModel::toggle_language() {
    model_.toggle_language();
    changed();
}

AppPage BaseViewModel::current_page() const {
    return model_.current_page();
}

void BaseViewModel::show_page(AppPage page) {
    model_.set_current_page(page);
    model_.log.editing = false;
    model_.log.renaming = false;
    current_page_subject_.set(page_to_int(page));
    changed();
}

void BaseViewModel::show_menu() {
    show_page(AppPage::Menu);
}

void BaseViewModel::request_quit() {
    quit_requested_subject_.set(true);
}

void BaseViewModel::changed() {
    title_subject_.set(model_.app_title());
    state_subject_.set(state_subject_.value() + 1);
}

bool BaseViewModel::menu_key(uint32_t, int dx, int dy, bool select) {
    int& index = model_.menu_index;
    if (dx || dy) {
        const int col = (index % kMenuColumns + dx + kMenuColumns) % kMenuColumns;
        const int rows = model::kMenuItemCount / kMenuColumns;
        const int row = (index / kMenuColumns + dy + rows) % rows;
        index = row * kMenuColumns + col;
        changed();
        return true;
    }
    if (select) {
        if (index == model::kMenuItemCount - 1) {
            toggle_language();
        }
        else {
            show_page(kMenuPages[index]);
        }
        return true;
    }
    return false;
}

bool BaseViewModel::on_key(uint32_t key) {
    const bool up = key == 'f' || key == 'F' || key == LV_KEY_UP;
    const bool down = key == 'x' || key == 'X' || key == LV_KEY_DOWN;
    const bool left = key == 'z' || key == 'Z' || key == LV_KEY_LEFT;
    const bool right = key == 'c' || key == 'C' || key == LV_KEY_RIGHT;
    const bool select = key == LV_KEY_ENTER || key == ' ';
    const int dx = left ? -1 : right ? 1 : 0;
    const int dy = up ? -1 : down ? 1 : 0;

    // Typing a pack ID in the battery log swallows every key.
    if (model_.current_page() == AppPage::Log && model_.log.renaming) {
        auto& log = model_.log;
        if (key == LV_KEY_ENTER) log.commit_rename();
        else if (key == LV_KEY_ESC) log.cancel_rename();
        else if (key == LV_KEY_BACKSPACE || key == LV_KEY_DEL) log.rename_backspace();
        else if (key >= 32 && key < 127) log.rename_input(static_cast<char>(key));
        changed();
        return true;
    }

    if (key == 't' || key == 'T') {
        toggle_dark_mode();
        return true;
    }
    if (key == 'l' || key == 'L') {
        toggle_language();
        return true;
    }

    auto& m = model_;
    switch (m.current_page()) {
        case AppPage::Menu:
            return menu_key(key, dx, dy, select);

        case AppPage::Checklist:
            if (dy) m.move_selection(dy);
            else if (dx) m.switch_list(dx);
            else if (select) m.toggle_selected();
            else return false;
            break;

        case AppPage::Battery:
            if (dy) m.battery.move_field(dy);
            else if (dx) m.battery.adjust(dx);
            else if (select) m.battery.move_field(1);
            else return false;
            break;

        case AppPage::Channels:
            if (dy) m.channels.move_system(dy);
            else if (dx) m.channels.move_channel(dx);
            else if (select) m.channels.toggle_mark();
            else return false;
            break;

        case AppPage::Props:
            if (dy) m.props.move_size(dy);
            else if (dx) m.props.move_variant(dx);
            else return false;
            break;

        case AppPage::Log: {
            auto& log = m.log;
            if (log.editing) {
                if (dy) log.field = (log.field + dy + model::kPackFieldCount) % model::kPackFieldCount;
                else if (dx) log.adjust_field(dx);
                else if (select || key == LV_KEY_ESC) log.editing = false;
                else return false;
            }
            else {
                if (dy) log.move(dy);
                else if (dx) log.adjust_cycles(dx);
                else if (select && !log.packs.empty()) { log.editing = true; log.field = 0; }
                else return false;
            }
            break;
        }
    }
    changed();
    return true;
}

std::array<const char*, 5> BaseViewModel::nav_icons() const {
    using namespace view;
    const char* theme_icon = model_.dark_mode() ? ICON_SUN : ICON_MOON;
    switch (model_.current_page()) {
        case AppPage::Menu:
            return {ICON_SIGN_OUT, "", theme_icon, ICON_TRANSLATE, ""};
        case AppPage::Checklist:
            return {ICON_ARROW_LEFT, ICON_CARET_LEFT, ICON_RESET, ICON_CARET_RIGHT, ICON_CHECK_CIRCLE};
        case AppPage::Battery:
            return {ICON_ARROW_LEFT, ICON_CARET_LEFT, ICON_MINUS, ICON_PLUS, ICON_CARET_RIGHT};
        case AppPage::Channels:
            return {ICON_ARROW_LEFT, ICON_CARET_LEFT, ICON_CHECK_CIRCLE, ICON_CARET_RIGHT, ICON_USERS};
        case AppPage::Props:
            return {ICON_ARROW_LEFT, ICON_CARET_LEFT, ICON_CARET_UP, ICON_CARET_DOWN, ICON_CARET_RIGHT};
        case AppPage::Log:
            return {ICON_ARROW_LEFT, ICON_PLUS_CIRCLE, ICON_PENCIL, ICON_PLUS, ICON_TRASH};
    }
    return {"", "", "", "", ""};
}

void BaseViewModel::nav_press(size_t index) {
    auto& m = model_;
    if (index == 0) {
        if (m.current_page() == AppPage::Menu) request_quit();
        else show_menu();
        return;
    }
    switch (m.current_page()) {
        case AppPage::Menu:
            if (index == 2) toggle_dark_mode();
            if (index == 3) toggle_language();
            return;
        case AppPage::Checklist:
            if (index == 1) m.switch_list(-1);
            if (index == 2) m.reset_active_list();
            if (index == 3) m.switch_list(1);
            if (index == 4) m.toggle_selected();
            break;
        case AppPage::Battery:
            if (index == 1) m.battery.switch_view(-1);
            if (index == 2) m.battery.adjust(-1);
            if (index == 3) m.battery.adjust(1);
            if (index == 4) m.battery.switch_view(1);
            break;
        case AppPage::Channels:
            if (index == 1) m.channels.move_channel(-1);
            if (index == 2) m.channels.toggle_mark();
            if (index == 3) m.channels.move_channel(1);
            if (index == 4) m.channels.next_preset();
            break;
        case AppPage::Props:
            if (index == 1) m.props.move_variant(-1);
            if (index == 2) m.props.move_size(-1);
            if (index == 3) m.props.move_size(1);
            if (index == 4) m.props.move_variant(1);
            break;
        case AppPage::Log:
            if (index == 1) m.log.add_pack();
            if (index == 2) {
                if (m.log.renaming) m.log.commit_rename();
                else m.log.start_rename();
            }
            if (index == 3) m.log.adjust_cycles(1);
            if (index == 4) m.log.delete_selected();
            break;
    }
    changed();
}

std::array<HelpRow, 5> BaseViewModel::help_rows() const {
    switch (model_.current_page()) {
        case AppPage::Menu:
            return {{{"F X Z C", tr("Move", "Рух")},
                     {"ENTER", tr("Open", "Відкрити")},
                     {"HOLD ESC / 4", tr("Exit", "Вихід")},
                     {"T  /  6", tr("Light / dark", "Світла / темна")},
                     {"L  /  7", tr("Language", "Мова")}}};
        case AppPage::Checklist:
            return {{{"F X", tr("Move", "Рух")},
                     {"ENTER / 8", tr("Check item", "Відмітити")},
                     {"Z C / 5 7", tr("Switch list", "Інший список")},
                     {"6", tr("Reset list", "Скинути список")},
                     {"ESC / 4", tr("Menu", "Меню")}}};
        case AppPage::Battery:
            return {{{"F X", tr("Select field", "Вибрати поле")},
                     {"Z C / 6 7", tr("Change value", "Змінити значення")},
                     {"5  /  8", tr("Charge/Check/Info", "Заряд/Перевірка/Інфо")},
                     {"ENTER", tr("Next field", "Наступне поле")},
                     {"ESC / 4", tr("Menu", "Меню")}}};
        case AppPage::Channels:
            return {{{"Z C / 5 7", tr("Channel", "Канал")},
                     {"F X", tr("System / band", "Система / бенд")},
                     {"ENTER / 6", tr("Mark (IMD check)", "Позначити (IMD)")},
                     {"8", tr("Group presets", "Групові набори")},
                     {"ESC / 4", tr("Menu", "Меню")}}};
        case AppPage::Props:
            return {{{"F X / 6 7", tr("Prop size", "Розмір пропа")},
                     {"Z C / 5 8", tr("Build variant", "Варіант збірки")},
                     {"", tr("Data: Oscar Liang", "Дані: Oscar Liang")},
                     {"", ""},
                     {"ESC / 4", tr("Menu", "Меню")}}};
        case AppPage::Log:
            return {{{"F X", tr("Select pack", "Вибрати пак")},
                     {"Z C / 7", tr("Cycles -/+", "Цикли -/+")},
                     {"ENTER / 6", tr("Edit / name (ID)", "Редагувати / назва (ID)")},
                     {"5  /  8 8", tr("Add / delete", "Додати / видалити")},
                     {"ESC / 4", tr("Menu", "Меню")}}};
    }
    return {};
}

std::string BaseViewModel::save_state() const {
    return model_.serialize();
}

void BaseViewModel::load_state(const std::string& text) {
    model_.deserialize(text);
    changed();
}

} // namespace viewmodel
