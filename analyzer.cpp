#include "analyzer.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace {

struct Data {
    unordered_map<string, long long> zoneCnt;
    unordered_map<string, array<long long, 24>> hourCnt;
};

unordered_map<const TripAnalyzer*, Data> store;

Data& getData(const TripAnalyzer* t) {
    return store[t];
}

bool getHour(const string& s, int& h) {
    size_t p = s.find(' ');
    if (p == string::npos || p + 2 >= s.size()) return false;

    if (!isdigit(s[p + 1]) || !isdigit(s[p + 2])) return false;

    h = (s[p + 1] - '0') * 10 + (s[p + 2] - '0');
    return h >= 0 && h < 24;
}

void splitLine(const string& line, vector<string>& out) {
    out.clear();
    string part;
    stringstream ss(line);
    while (getline(ss, part, ',')) out.push_back(part);
}

}

void TripAnalyzer::ingestFile(const string& path) {
    Data& d = getData(this);
    d.zoneCnt.clear();
    d.hourCnt.clear();

    ifstream file(path);
    if (!file) return;

    string line;
    getline(file, line);

    vector<string> cols;

    while (getline(file, line)) {
        if (line.empty()) continue;

        splitLine(line, cols);
        if (cols.size() < 6) continue;

        string zone = cols[1];
        string time = cols[3];
        if (zone.empty() || time.empty()) continue;

        int hour;
        if (!getHour(time, hour)) continue;

        d.zoneCnt[zone]++;
        d.hourCnt[zone][hour]++;
    }
}

vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    if (k <= 0) return {};

    auto it = store.find(this);
    if (it == store.end()) return {};

    vector<ZoneCount> res;
    for (auto& e : it->second.zoneCnt)
        res.push_back({e.first, e.second});

    sort(res.begin(), res.end(), [](auto& a, auto& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
    });

    if ((size_t)k < res.size()) res.resize(k);
    return res;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    if (k <= 0) return {};

    auto it = store.find(this);
    if (it == store.end()) return {};

    vector<SlotCount> res;

    for (auto& z : it->second.hourCnt) {
        for (int h = 0; h < 24; h++) {
            long long c = z.second[h];
            if (c)
                res.push_back({z.first, h, c});
        }
    }

    sort(res.begin(), res.end(), [](auto& a, auto& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    });

    if ((size_t)k < res.size()) res.resize(k);
    return res;
}
