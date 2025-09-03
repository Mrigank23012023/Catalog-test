#include <bits/stdc++.h>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/rational.hpp>
#include "json.hpp"  // nlohmann/json single-header

using namespace std;
using json = nlohmann::json;
using boost::multiprecision::cpp_int;
using Rat = boost::rational<cpp_int>;

// --- helpers ---

int charToDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'z') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
    return -1;
}

cpp_int convertBase(const string& value, int base) {
    cpp_int result = 0;
    for (char c : value) {
        int d = charToDigit(c);
        if (d < 0 || d >= base) {
            cerr << "Invalid digit '" << c << "' for base " << base << "\n";
            exit(1);
        }
        result = result * base + d;
    }
    return result;
}

// parse JSON key (e.g., "10") to cpp_int safely (no stoi truncation)
cpp_int parseKeyToCppInt(const string& s) {
    cpp_int x = 0;
    for (char c : s) {
        if (c < '0' || c > '9') {
            cerr << "Non-decimal key encountered: " << s << "\n";
            exit(1);
        }
        x = x * 10 + (c - '0');
    }
    return x;
}

// Lagrange interpolation at x=0 using exact rationals
Rat lagrangeAtZero_rational(const vector<pair<cpp_int, cpp_int>>& pts, int k, bool verbose) {
    Rat sum = 0;
    for (int j = 0; j < k; ++j) {
        const cpp_int& xj = pts[j].first;
        const cpp_int& yj = pts[j].second;

        cpp_int num = 1; // product of x_m
        cpp_int den = 1; // product of (x_m - x_j)
        for (int m = 0; m < k; ++m) {
            if (m == j) continue;
            num *= pts[m].first;
            den *= (pts[m].first - xj);
        }

        Rat term(num, den);   // product_m x_m / (x_m - x_j)
        term *= yj;           // multiply by y_j

        if (verbose) {
            cout << "Term j=" << j << ":\n";
            cout << "  xj=" << xj << ", yj=" << yj << "\n";
            cout << "  product numerator   = " << num << "\n";
            cout << "  product denominator = " << den << "\n";
            cout << "  reduced term        = " << term.numerator()
                 << "/" << term.denominator() << "\n";
        }

        sum += term;

        if (verbose) {
            cout << "  running sum         = " << sum.numerator()
                 << "/" << sum.denominator() << "\n\n";
        }
    }
    return sum; // should be integral (denominator == 1)
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Read entire JSON from stdin
    string input, line;
    while (getline(cin, line)) input += line;
    if (input.empty()) {
        cerr << "No input provided.\n";
        return 1;
    }

    json j = json::parse(input);

    int n = j["keys"]["n"];
    int k = j["keys"]["k"];

    // Decode all points
    vector<pair<cpp_int, cpp_int>> points;
    struct Decoded {
        cpp_int x;
        int base;
        string valStr;
        cpp_int y;
    };
    vector<Decoded> decoded;

    for (auto& el : j.items()) {
        if (el.key() == "keys") continue;
        cpp_int x = parseKeyToCppInt(el.key()); // x is the key (decimal)
        int base = stoi(el.value()["base"].get<string>());
        string valStr = el.value()["value"];
        cpp_int y = convertBase(valStr, base);
        points.push_back({x, y});
        decoded.push_back({x, base, valStr, y});
    }

    if ((int)points.size() < k) {
        cerr << "Not enough points: have " << points.size() << ", need " << k << "\n";
        return 1;
    }

    // Sort by x
    sort(points.begin(), points.end(),
         [](const auto& a, const auto& b){ return a.first < b.first; });
    sort(decoded.begin(), decoded.end(),
         [](const Decoded& a, const Decoded& b){ return a.x < b.x; });

    // --- Step-by-step prints ---

    cout << "=== Decoded points (x, base, value -> y_decimal) ===\n";
    for (auto &d : decoded) {
        cout << "x=" << d.x << ", base=" << d.base
             << ", value=\"" << d.valStr << "\""
             << "  -> y=" << d.y << "\n";
    }
    cout << "\n";

    // Select first k points
    vector<pair<cpp_int, cpp_int>> selected(points.begin(), points.begin() + k);

    cout << "=== Selected first k=" << k << " points (sorted by x) ===\n";
    for (auto &p : selected) {
        cout << "(" << p.first << ", " << p.second) << " ";
    }
    cout << "\n\n";

    // Lagrange interpolation at 0 with verbose per-term output
    cout << "=== Lagrange interpolation @ x=0 (per-term) ===\n";
    Rat sum = lagrangeAtZero_rational(selected, k, /*verbose=*/true);

    cout << "=== Final sum (reduced) ===\n";
    cout << "sum = " << sum.numerator() << "/" << sum.denominator() << "\n";

    if (sum.denominator() != 1) {
        cerr << "WARNING: result is not an integer! (unexpected)\n";
    }

    cpp_int c = sum.numerator() / sum.denominator();
    cout << "\n=== Constant term c (= f(0)) ===\n" << c << "\n";

    return 0;
}
