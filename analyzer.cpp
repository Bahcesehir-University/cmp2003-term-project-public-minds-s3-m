#include "analyzer.h"
using namespace std;

void TripAnalyzer::ingestFile(const string& csvPath) {
    ifstream file(csvPath);
    if (!file.is_open()) {
        return;
    }
    
    string line;
    bool isFirstLine = true;
    
    while (getline(file, line)) {
        if (isFirstLine) {
            isFirstLine = false;
            continue;
        }
        
        if (line.empty()) {
            continue;
        }
        
        vector<string> fields;
        stringstream ss(line);
        string field;
        
        while (getline(ss, field, ',')) {
            fields.push_back(field);
        }
        
       if (fields.size()  !=6 ){
           continue;
       }
        
        string zone = fields[1];
        if (zone.empty()) {
            continue;
        }
        
        string datetime = fields[3];
        if (datetime.empty()) {
            continue;
        }
        
        if (datetime.length() < 16) {
            continue;
        }
        
        size_t spacePos = datetime.find(' ');
        if (spacePos == string::npos) {
            continue;
        }
        
        if (spacePos + 3 >= datetime.length()) {
            continue;
        }
        
        string hourStr = datetime.substr(spacePos + 1, 2);
        if (hourStr.length() != 2) {
            continue;
        }
        
        int hour = -1;
        try {
            hour = stoi(hourStr);
        } catch (...) {
            continue;
        }
        
        if (hour < 0 || hour > 23) {
            continue;
        }
        
        zoneCounts[zone]++;
        slotCounts[{zone, hour}]++;
    }
    
    file.close();
}

vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    vector<ZoneCount> result;
    result.reserve(zoneCounts.size());
    
    for (const auto& pair : zoneCounts) {
        result.push_back({pair.first, pair.second});
    }
    
    sort(result.begin(), result.end(), 
        [](const ZoneCount& a, const ZoneCount& b) {
            if (a.count != b.count) {
                return a.count > b.count;
            }
            return a.zone < b.zone;
        });
    
    if (k <= 0) {
        return {};
    }
    if (static_cast<size_t>(k) >= result.size()) {
        return result;
    }
    return vector<ZoneCount>(result.begin(), result.begin() + k);
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    vector<SlotCount> result;
    result.reserve(slotCounts.size());
    
    for (const auto& pair : slotCounts) {
        result.push_back({pair.first.first, pair.first.second, pair.second});
    }
    
    sort(result.begin(), result.end(),
        [](const SlotCount& a, const SlotCount& b) {
            if (a.count != b.count) {
                return a.count > b.count;
            }
            if (a.zone != b.zone) {
                return a.zone < b.zone;
            }
            return a.hour < b.hour;
        });
    
    if (k <= 0) {
        return {};
    }
    if (static_cast<size_t>(k) >= result.size()) {
        return result;
    }
    return vector<SlotCount>(result.begin(), result.begin() + k);
}


