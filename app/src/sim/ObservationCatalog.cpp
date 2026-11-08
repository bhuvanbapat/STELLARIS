#include "sim/ObservationCatalog.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace stellaris {

namespace {

void appendUtf8(std::string& s, unsigned cp) {
    if (cp < 0x80) s += static_cast<char>(cp);
    else if (cp < 0x800) { s += static_cast<char>(0xC0 | (cp >> 6)); s += static_cast<char>(0x80 | (cp & 0x3F)); }
    else { s += static_cast<char>(0xE0 | (cp >> 12)); s += static_cast<char>(0x80 | ((cp >> 6) & 0x3F)); s += static_cast<char>(0x80 | (cp & 0x3F)); }
}

struct Parser {
    const std::string& t;
    size_t i = 0;
    void ws() { while (i < t.size() && std::isspace(static_cast<unsigned char>(t[i]))) ++i; }
    bool eat(char c) { ws(); if (i < t.size() && t[i] == c) { ++i; return true; } return false; }
    bool string(std::string& out) {
        ws();
        if (i >= t.size() || t[i] != '"') return false;
        ++i;
        out.clear();
        while (i < t.size() && t[i] != '"') {
            char c = t[i++];
            if (c != '\\') { out += c; continue; }
            if (i >= t.size()) return false;
            const char e = t[i++];
            switch (e) {
            case '"': out += '"'; break;
            case '\\': out += '\\'; break;
            case '/': out += '/'; break;
            case 'n': out += '\n'; break;
            case 't': out += '\t'; break;
            case 'u': {
                if (i + 4 > t.size()) return false;
                appendUtf8(out, static_cast<unsigned>(std::strtoul(t.substr(i, 4).c_str(), nullptr, 16)));
                i += 4;
                break;
            }
            default: return false;
            }
        }
        if (i >= t.size()) return false;
        ++i;  // closing quote
        return true;
    }
};

} // namespace

bool parseFlatJson(const std::string& text, std::map<std::string, std::string>& out, std::string* error) {
    Parser p{text};
    auto fail = [&](const char* what) { if (error) *error = what; return false; };
    if (!p.eat('{')) return fail("expected '{'");
    if (p.eat('}')) return true;
    do {
        std::string key, value;
        if (!p.string(key)) return fail("expected a string key");
        if (!p.eat(':')) return fail("expected ':'");
        if (!p.string(value)) return fail("expected a string value");
        out[key] = value;
    } while (p.eat(','));
    if (!p.eat('}')) return fail("expected '}'");
    return true;
}

const std::string& Observation::get(const std::string& key) const {
    static const std::string empty;
    const auto it = fields.find(key);
    return it == fields.end() ? empty : it->second;
}

bool Observation::matches(Stage stage, Pathway pathway) const {
    // "stage" is "N" or "N-M" (1-based); "pathway" is "all" or a pathway name.
    const std::string& s = get("stage");
    const int lo = std::atoi(s.c_str());
    const size_t dash = s.find('-');
    const int hi = dash == std::string::npos ? lo : std::atoi(s.c_str() + dash + 1);
    const int n = static_cast<int>(stage) + 1;
    if (n < lo || n > hi) return false;
    const std::string& p = get("pathway");
    if (p == "all") return true;
    switch (pathway) {
    case Pathway::WhiteDwarf:  return p == "white-dwarf";
    case Pathway::NeutronStar: return p == "neutron-star";
    case Pathway::BlackHole:   return p == "black-hole";
    }
    return false;
}

std::vector<Observation> loadObservationCatalog(const std::filesystem::path& dir, std::vector<std::string>* errors) {
    std::vector<Observation> catalog;
    std::error_code ec;
    if (!std::filesystem::is_directory(dir, ec)) {
        if (errors) errors->push_back("observation folder not found: " + dir.string());
        return catalog;
    }
    std::vector<std::filesystem::path> files;
    for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
        if (entry.path().extension() == ".json") files.push_back(entry.path());
    std::sort(files.begin(), files.end());
    for (const auto& file : files) {
        std::ifstream in(file, std::ios::binary);
        std::stringstream ss;
        ss << in.rdbuf();
        Observation o;
        std::string err;
        if (!parseFlatJson(ss.str(), o.fields, &err)) {
            if (errors) errors->push_back(file.filename().string() + ": " + err);
            continue;
        }
        o.imagePath = dir / o.get("local_file");
        if (o.get("credit").empty() || o.get("license").empty() || !std::filesystem::exists(o.imagePath, ec)) {
            if (errors) errors->push_back(file.filename().string() + ": missing credit, licence or image");
            continue;  // never show an asset without provenance
        }
        catalog.push_back(std::move(o));
    }
    return catalog;
}

} // namespace stellaris
