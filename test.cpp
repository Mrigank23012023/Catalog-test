#include <bits/stdc++.h>
#include <boost/multiprecision/cpp_int.hpp>
#include "json.hpp"  

using namespace std;
using json = nlohmann::json;
using boost::multiprecision::cpp_int;

// Convert digit character to integer
int charToDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
    return -1;
}

// Convert string from base -> cpp_int
cpp_int convertBase(const string& value, int base) {
    cpp_int result = 0;
    for (char c : value) {
        int digit = charToDigit(c);
        if (digit < 0 || digit >= base) {
            cerr << "Invalid digit for base " << base << ": " << c << "\n";
            exit(1);
        }
        result = result * base + digit;
    }
    return result;
}

// Lagrange interpolation to compute f(0)
cpp_int lagrangeInterpolationAtZero(const vector<pair<cpp_int, cpp_int>>& points, int k) {
    cpp_int ans = 0;
    for (int j = 0; j < k; j++) {
        cpp_int xj = points[j].first;
        cpp_int yj = points[j].second;

        // Compute product over m != j
        cpp_int num = 1, den = 1;
        for (int m = 0; m < k; m++) {
            if (m == j) continue;
            num *= points[m].first;           // numerator: x_m
            den *= (points[m].first - xj);    // denominator: (x_m - x_j)
        }

        // Add term: yj * num/den
        ans += yj * num / den;
    }
    return ans;
}

int main() {
    // Read JSON from stdin
    string input, line;
    while (getline(cin, line)) input += line;
    json j = json::parse(input);

    int n = j["keys"]["n"];
    int k = j["keys"]["k"];

    vector<pair<cpp_int, cpp_int>> points;
    for (auto& el : j.items()) {
        if (el.key() == "keys") continue;
        cpp_int x = stoi(el.key()); // x is the key
        int base = stoi(el.value()["base"].get<string>());
        string valStr = el.value()["value"];
        cpp_int y = convertBase(valStr, base);
        points.push_back({x, y});
    }

    // Take first k points
    sort(points.begin(), points.end(), [](auto& a, auto& b){ return a.first < b.first; });
    vector<pair<cpp_int, cpp_int>> selected(points.begin(), points.begin() + k);

    cpp_int c = lagrangeInterpolationAtZero(selected, k);
    cout << c << "\n";

    return 0;
}

