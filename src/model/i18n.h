/*
 * FPV Checklist — tiny two-language (EN / UA) helper.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

namespace model {

enum class Lang {
    En = 0,
    Ua = 1,
};

Lang language();
void set_language(Lang lang);

// Pick the string for the current UI language.
inline const char* tr(const char* en, const char* ua) {
    return language() == Lang::Ua ? ua : en;
}

struct Text {
    const char* en;
    const char* ua;

    const char* get() const {
        return tr(en, ua);
    }
};

} // namespace model
