#include "test-helpers.h"

bool set_int64_contains(const std::set<int64_t>& s, const int64_t value) {
    return s.find(value) != s.end();
}

bool set_pair_int64_contains(const std::set<std::pair<int64_t, int64_t>>& s, const std::pair<int64_t, int64_t> value) {
    return s.find(value) != s.end();
}
