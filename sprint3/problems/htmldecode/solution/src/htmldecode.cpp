#include "htmldecode.h"

#include <algorithm>
#include <unordered_map>
#include <optional>
#include <iostream>

namespace consts {

const std::unordered_map<std::string_view, char> mnemonics = {
    {"lt", '<'},
    {"gt", '>'},
    {"amp", '&'},
    {"apos", '\''},
    {"quot", '"'},
};

}  // namespace consts

bool IsSameCase(std::string_view str) {
    for (size_t i = 1; i < str.size(); ++i) {
        if (std::islower(str[i - 1]) != std::islower(str[i])) {
            return false;
        }
    }

    return true;
}

bool IEquals(std::string_view lhs, std::string_view rhs) {
    return std::equal(
        lhs.begin(),
        lhs.end(),
        rhs.begin(),
        rhs.end(),
        [](char lhs, char rhs) {
            return std::tolower(lhs) == std::tolower(rhs);
        }
    );
}

std::optional<std::string_view> FindMnemonic(std::string_view str) {
    for (const auto& [mnemonic, _] : consts::mnemonics) {
        if (str.size() < mnemonic.size()) {
            continue;
        }

        std::string_view substr = str.substr(0, mnemonic.size());
        if (IsSameCase(substr) && IEquals(substr, mnemonic)) {
            return mnemonic;
        }
    }

    return std::nullopt;
}

std::string HtmlDecode(std::string_view str) {
    std::string result;

    for (size_t pos = str.find('&'); pos != str.npos; pos = str.find('&')) {
        result += str.substr(0, pos);
        str.remove_prefix(pos + 1);

        if (auto mnemonic = FindMnemonic(str); mnemonic) {
            str.remove_prefix(mnemonic->size());
            result += consts::mnemonics.at(*mnemonic);

            if (str.front() == ';') {
                str.remove_prefix(1);
            }
        } else {
            result += '&';
        }
    }

    if (!str.empty()) {
        result += str;
    }

    return result;
}
