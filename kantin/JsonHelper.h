#pragma once
#include <string>
#include <map>
#include <vector>
#include <sstream>
using namespace std;

// Simple JSON parser & builder
class JsonHelper {
public:
    // Extract string value dari JSON by key
    static string getString(const string& json, const string& key) {
        string search = "\"" + key + "\":\"";
        size_t pos = json.find(search);
        if (pos == string::npos) return "";
        pos += search.length();
        size_t end = json.find("\"", pos);
        if (end == string::npos) return "";
        return json.substr(pos, end - pos);
    }

    // Extract integer value dari JSON by key
    static int getInt(const string& json, const string& key) {
        string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == string::npos) return -1;
        pos += search.length();
        // skip quote jika ada
        if (json[pos] == '"') return -1;
        size_t end = pos;
        while (end < json.size() && (isdigit(json[end]) || json[end] == '-')) end++;
        return stoi(json.substr(pos, end - pos));
    }

    // Extract double value dari JSON by key
    static double getDouble(const string& json, const string& key) {
        string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == string::npos) return 0.0;
        pos += search.length();
        size_t end = pos;
        while (end < json.size() && (isdigit(json[end]) || json[end] == '.' || json[end] == '-')) end++;
        return stod(json.substr(pos, end - pos));
    }

    // Parse array of JSON objects (simple, satu level)
    static vector<string> getArray(const string& json, const string& key) {
        vector<string> result;
        string search = "\"" + key + "\":[";
        size_t pos = json.find(search);
        if (pos == string::npos) return result;
        pos += search.length();

        int depth = 1;
        size_t start = pos;
        while (pos < json.size() && depth > 0) {
            if (json[pos] == '{') depth++;
            else if (json[pos] == '}') {
                depth--;
                if (depth == 0) {
                    result.push_back(json.substr(start, pos - start + 1));
                    start = pos + 2; // skip "},"
                    if (pos + 1 < json.size() && json[pos+1] == ',') start++;
                    depth = 1;
                }
            }
            pos++;
        }
        return result;
    }

    // Build order JSON dari input manual
    static string buildOrderJson(const string& customerName,
                                  const vector<pair<int,int>>& items) {
        // items = vector of {menuId, quantity}
        ostringstream oss;
        oss << "{\"type\":\"ORDER\","
            << "\"customer\":\"" << customerName << "\","
            << "\"items\":[";
        for (int i = 0; i < (int)items.size(); i++) {
            if (i > 0) oss << ",";
            oss << "{\"menuId\":" << items[i].first
                << ",\"quantity\":" << items[i].second << "}";
        }
        oss << "]}";
        return oss.str();
    }

    static string buildRequestJson(const string& type) {
        return "{\"type\":\"" + type + "\"}";
    }

    static string getType(const string& json) {
        return getString(json, "type");
    }
};
