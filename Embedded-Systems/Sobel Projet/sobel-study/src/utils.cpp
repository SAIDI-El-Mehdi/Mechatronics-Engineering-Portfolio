#include "utils.hpp"
#include <algorithm>
#include <cmath>

double mean_of(const std::vector<double>& v) {
    if (v.empty()) return 0.0;
    double s = 0.0;
    for (double x : v) s += x;
    return s / static_cast<double>(v.size());
}

double median_of(std::vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    const size_t n = v.size();
    if (n % 2 == 0) return (v[n / 2 - 1] + v[n / 2]) / 2.0;
    return v[n / 2];
}

double stddev_of(const std::vector<double>& v, double mean) {
    if (v.size() < 2) return 0.0;
    double acc = 0.0;
    for (double x : v) {
        double d = x - mean;
        acc += d * d;
    }
    return std::sqrt(acc / static_cast<double>(v.size()));
}
