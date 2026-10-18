#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

enum da_mode_t {
    DA_MODE_GLOBAL = 0,
    DA_MODE_FOCUS,
    DA_MODE_LOCAL,
};

struct da_tag_t {
    int type = -1;
    std::vector<int32_t> keep_nums;
    size_t start = 0;
    size_t end = 0;
};

bool da_tag_start_allowed(const std::string & text, size_t lt);
da_tag_t scan_da_tag(const std::string & text, size_t from);
bool da_tag_prefix(const std::string & text);
size_t da_tag_hold_len(const std::string & text);