#include "da-tag.h"

#include <cctype>
#include <cstring>
#include <limits>
#include <utility>

bool da_tag_start_allowed(const std::string & text, size_t lt) {
    return lt == 0 || text[lt - 1] == '\n';
}

da_tag_t scan_da_tag(const std::string & text, size_t from) {
    da_tag_t best;
    for (size_t lt = text.find('<', from); lt != std::string::npos; lt = text.find('<', lt + 1)) {
        if (!da_tag_start_allowed(text, lt)) {
            continue;
        }

        da_tag_t candidate;
        candidate.start = lt;
        if (text.compare(lt, 8, "</focus>") == 0) {
            candidate.type = 2;
            candidate.end = lt + 8;
        } else if (text.compare(lt, 8, "</local>") == 0) {
            candidate.type = 3;
            candidate.end = lt + 8;
        } else if (text.compare(lt, 9, "</global>") == 0) {
            candidate.type = 5;
            candidate.end = lt + 9;
        } else if (text.compare(lt, 8, "<global>") == 0) {
            candidate.type = 4;
            candidate.end = lt + 8;
        } else if (text.compare(lt, 7, "<local>") == 0) {
            candidate.type = 1;
            candidate.end = lt + 7;
        } else if (text.compare(lt, 6, "<focus") == 0) {
            const size_t close = text.find('>', lt + 6);
            if (close == std::string::npos) {
                continue;
            }
            const std::string tag = text.substr(lt, close - lt + 1);
            const size_t attr = tag.find("magic_chunks");
            if (attr == std::string::npos) {
                continue;
            }
            const size_t quote = tag.find('"', attr + 12);
            const size_t end = quote == std::string::npos ? quote : tag.find('"', quote + 1);
            if (quote == std::string::npos || end == std::string::npos) {
                continue;
            }
            bool valid = true;
            for (size_t i = quote + 1; i < end;) {
                if (!std::isdigit((unsigned char) tag[i])) {
                    valid = false;
                    break;
                }
                int32_t value = 0;
                do {
                    const int digit = tag[i] - '0';
                    if (value > (std::numeric_limits<int32_t>::max() - digit) / 10) {
                        valid = false;
                        break;
                    }
                    value = value * 10 + digit;
                    ++i;
                } while (i < end && std::isdigit((unsigned char) tag[i]));
                if (!valid) {
                    break;
                }
                candidate.keep_nums.push_back(value);
                if (i == end) {
                    break;
                }
                if (tag[i] != ',') {
                    valid = false;
                    break;
                }
                ++i;
            }
            if (!valid) {
                candidate.keep_nums.clear();
            }
            if (!candidate.keep_nums.empty()) {
                candidate.type = 0;
                candidate.end = close + 1;
            }
        }

        if (candidate.type >= 0 && (best.type < 0 || candidate.end < best.end)) {
            best = std::move(candidate);
        }
    }
    return best;
}

bool da_tag_prefix(const std::string & text) {
    static const char * const fixed[] = { "<local>", "</focus>", "</local>", "<global>", "</global>" };
    for (const char * tag : fixed) {
        const size_t size = std::strlen(tag);
        if (text.size() < size && text.compare(0, text.size(), tag, text.size()) == 0) {
            return true;
        }
    }

    static const char head[] = "<focus magic_chunks=";
    const size_t head_size = sizeof(head) - 1;
    if (text.size() < head_size) {
        return text.compare(0, text.size(), head, text.size()) == 0;
    }
    if (text.compare(0, head_size, head) != 0) {
        return false;
    }

    size_t i = head_size;
    if (i < text.size() && text[i] == '"') {
        ++i;
    }
    if (i == text.size()) {
        return true;
    }

    bool has_number = false;
    for (; i < text.size(); ++i) {
        const unsigned char c = (unsigned char) text[i];
        if (std::isdigit(c)) {
            has_number = true;
        } else if (c == ',' && has_number) {
            has_number = false;
        } else if (c == '"' && has_number) {
            return i + 1 == text.size();
        } else {
            return false;
        }
    }
    return true;
}

size_t da_tag_hold_len(const std::string & text) {
    size_t last_lt = std::string::npos;
    for (size_t i = text.find('<'); i != std::string::npos; i = text.find('<', i + 1)) {
        if (da_tag_start_allowed(text, i)) {
            last_lt = i;
        }
    }
    return last_lt != std::string::npos && da_tag_prefix(text.substr(last_lt))
        ? text.size() - last_lt
        : 0;
}