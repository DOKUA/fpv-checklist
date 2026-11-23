/*
 * SPDX-License-Identifier: MIT
 */

#include "i18n.h"

namespace model {
namespace {
Lang g_language = Lang::En;
} // namespace

Lang language() {
    return g_language;
}

void set_language(Lang lang) {
    g_language = lang;
}

} // namespace model
