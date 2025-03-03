#pragma once
#include <vector>
#include <string>
#include <map>
#include <set>

namespace utils {

// Convert a set to a vector
template<typename T>
std::vector<T> SetToVector(const std::set<T>& set) {
    return std::vector<T>(set.begin(), set.end());
}

// Extract keys from a map into a vector
template<typename K, typename V>
std::vector<K> MapKeysToVector(const std::map<K, V>& map) {
    std::vector<K> keys;
    keys.reserve(map.size());
    for (const auto& [key, _] : map) {
        keys.push_back(key);
    }
    return keys;
}

// Extract values from a map into a vector
template<typename K, typename V>
std::vector<V> MapValuesToVector(const std::map<K, V>& map) {
    std::vector<V> values;
    values.reserve(map.size());
    for (const auto& [_, value] : map) {
        values.push_back(value);
    }
    return values;
}

} // namespace utils 