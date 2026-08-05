#include "mse/stemmer.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <utility>

namespace mse {
namespace {

bool ends_with(const std::string& w, std::string_view suf) {
    return w.size() >= suf.size() &&
           w.compare(w.size() - suf.size(), suf.size(), suf) == 0;
}

bool has_vowel(const std::string& w) {
    for (char c : w) {
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y')
            return true;
    }
    return false;
}

// Measure: number of VC sequences (Porter).
int measure(const std::string& w) {
    int m = 0;
    bool in_v = false;
    for (char c : w) {
        const bool v = (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
                        (c == 'y' && !in_v));
        if (v) {
            in_v = true;
        } else if (in_v) {
            ++m;
            in_v = false;
        }
    }
    return m;
}

bool stem_cvc(const std::string& w) {
    if (w.size() < 3)
        return false;
    const char a = w[w.size() - 3];
    const char b = w[w.size() - 2];
    const char c = w[w.size() - 1];
    const auto is_v = [](char ch) {
        return ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u';
    };
    if (is_v(a) || !is_v(b) || is_v(c) || c == 'w' || c == 'x' || c == 'y')
        return false;
    return !is_v(a);
}

void step1a(std::string& w) {
    if (ends_with(w, "sses"))
        w.resize(w.size() - 2);
    else if (ends_with(w, "ies"))
        w.resize(w.size() - 2);
    else if (ends_with(w, "ss"))
        ;
    else if (ends_with(w, "s"))
        w.pop_back();
}

void step1b(std::string& w) {
    bool flag = false;
    if (ends_with(w, "eed")) {
        std::string stem = w.substr(0, w.size() - 3);
        if (measure(stem) > 0)
            w.pop_back();
    } else if (ends_with(w, "ed")) {
        std::string stem = w.substr(0, w.size() - 2);
        if (has_vowel(stem)) {
            w = stem;
            flag = true;
        }
    } else if (ends_with(w, "ing")) {
        std::string stem = w.substr(0, w.size() - 3);
        if (has_vowel(stem)) {
            w = stem;
            flag = true;
        }
    }
    if (flag) {
        if (ends_with(w, "at") || ends_with(w, "bl") || ends_with(w, "iz")) {
            w.push_back('e');
        } else if (w.size() >= 2 && w[w.size() - 1] == w[w.size() - 2] &&
                   w.back() != 'l' && w.back() != 's' && w.back() != 'z') {
            w.pop_back();
        } else if (measure(w) == 1 && stem_cvc(w)) {
            w.push_back('e');
        }
    }
}

void step1c(std::string& w) {
    if (ends_with(w, "y") && has_vowel(w.substr(0, w.size() - 1)))
        w.back() = 'i';
}

void replace_if_m(std::string& w, std::string_view suf, std::string_view rep, int min_m) {
    if (!ends_with(w, suf))
        return;
    std::string stem = w.substr(0, w.size() - suf.size());
    if (measure(stem) > min_m)
        w = stem + std::string(rep);
}

void step2(std::string& w) {
    static const std::pair<const char*, const char*> rules[] = {
        {"ational", "ate"}, {"tional", "tion"}, {"enci", "ence"}, {"anci", "ance"},
        {"izer", "ize"},    {"abli", "able"},   {"alli", "al"},   {"entli", "ent"},
        {"eli", "e"},       {"ousli", "ous"},   {"ization", "ize"}, {"ation", "ate"},
        {"ator", "ate"},    {"alism", "al"},    {"iveness", "ive"}, {"fulness", "ful"},
        {"ousness", "ous"}, {"aliti", "al"},    {"iviti", "ive"}, {"biliti", "ble"},
    };
    for (auto [suf, rep] : rules) {
        if (ends_with(w, suf)) {
            replace_if_m(w, suf, rep, 0);
            return;
        }
    }
}

void step3(std::string& w) {
    static const std::pair<const char*, const char*> rules[] = {
        {"icate", "ic"}, {"ative", ""}, {"alize", "al"}, {"iciti", "ic"},
        {"ical", "ic"},  {"ful", ""},   {"ness", ""},
    };
    for (auto [suf, rep] : rules) {
        if (ends_with(w, suf)) {
            replace_if_m(w, suf, rep, 0);
            return;
        }
    }
}

void step4(std::string& w) {
    static const char* sufs[] = {"al",   "ance", "ence", "er",   "ic",   "able", "ible",
                                 "ant",  "ement","ment", "ent",  "ou",   "ism",  "ate",
                                 "iti",  "ous",  "ive",  "ize"};
    for (const char* suf : sufs) {
        if (ends_with(w, suf)) {
            std::string stem = w.substr(0, w.size() - std::char_traits<char>::length(suf));
            if (measure(stem) > 1) {
                w = stem;
                return;
            }
        }
    }
    if (ends_with(w, "ion")) {
        std::string stem = w.substr(0, w.size() - 3);
        if (measure(stem) > 1 && !stem.empty() && (stem.back() == 's' || stem.back() == 't'))
            w = stem;
    }
}

void step5(std::string& w) {
    if (ends_with(w, "e")) {
        std::string stem = w.substr(0, w.size() - 1);
        int m = measure(stem);
        if (m > 1 || (m == 1 && !stem_cvc(stem)))
            w = stem;
    }
    if (ends_with(w, "ll") && measure(w) > 1)
        w.pop_back();
}

} // namespace

std::string porter_stem(std::string_view word) {
    std::string w;
    w.reserve(word.size());
    for (char c : word) {
        if (std::isalpha(static_cast<unsigned char>(c)))
            w.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    if (w.size() <= 2)
        return w;
    step1a(w);
    step1b(w);
    step1c(w);
    step2(w);
    step3(w);
    step4(w);
    step5(w);
    return w;
}

} // namespace mse
