#include "FastBlockBreak/Config.h"

#include <cstdint>
#include <fstream>
#include <iterator>

namespace fastblockbreak {
namespace {

class JsonReader {
public:
    explicit JsonReader(std::string const& text) : text_(text) {}

    bool Parse(bool& enabled) {
        Skip();
        if (!Take('{')) return false;
        Skip();
        bool found = false;
        if (Take('}')) return End();
        do {
            std::string key;
            if (!String(key)) return false;
            Skip();
            if (!Take(':')) return false;
            Skip();
            if (key == "enabled") {
                if (found) return false;
                if (At("true")) { Literal("true"); enabled = true; }
                else if (At("false")) { Literal("false"); enabled = false; }
                else return false;
                found = true;
            } else if (!Value()) return false;
            Skip();
            if (Take('}')) break;
            if (!Take(',')) return false;
            Skip();
        } while (true);
        return End();
    }

private:
    void Skip() { while (pos_ < text_.size() && (text_[pos_] == ' ' || text_[pos_] == '\t' || text_[pos_] == '\r' || text_[pos_] == '\n')) ++pos_; }
    bool Take(char c) { if (pos_ < text_.size() && text_[pos_] == c) { ++pos_; return true; } return false; }
    bool At(char const* s) const { return text_.compare(pos_, std::char_traits<char>::length(s), s) == 0; }
    bool Literal(char const* s) { if (!At(s)) return false; pos_ += std::char_traits<char>::length(s); return true; }
    bool End() { Skip(); return pos_ == text_.size(); }
    bool String(std::string& out) {
        if (!Take('"')) return false;
        out.clear();
        while (pos_ < text_.size()) {
            char c = text_[pos_++];
            if (c == '"') return true;
            if (static_cast<unsigned char>(c) < 0x20) return false;
            if (c == '\\') {
                if (pos_ >= text_.size()) return false;
                char escaped = text_[pos_++];
                switch (escaped) {
                case '"': case '\\': case '/': out.push_back(escaped); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case 'u': {
                    std::uint32_t codepoint{};
                    if (!CodeUnit(codepoint)) return false;
                    if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
                        if (pos_ + 2 > text_.size() || text_[pos_] != '\\' || text_[pos_ + 1] != 'u') return false;
                        pos_ += 2;
                        std::uint32_t low{};
                        if (!CodeUnit(low) || low < 0xDC00 || low > 0xDFFF) return false;
                        codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (low - 0xDC00);
                    } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF) return false;
                    AppendUtf8(out, codepoint);
                    break;
                }
                default: return false;
                }
            } else out.push_back(c);
        }
        return false;
    }
    static bool Hex(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }
    bool CodeUnit(std::uint32_t& value) {
        value = 0;
        for (int i = 0; i < 4; ++i) {
            if (pos_ >= text_.size() || !Hex(text_[pos_])) return false;
            char c = text_[pos_++];
            value = (value << 4) | (c <= '9' ? c - '0' : (c | 0x20) - 'a' + 10);
        }
        return true;
    }
    static void AppendUtf8(std::string& out, std::uint32_t c) {
        if (c <= 0x7F) out.push_back(static_cast<char>(c));
        else if (c <= 0x7FF) { out.push_back(static_cast<char>(0xC0 | (c >> 6))); out.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
        else if (c <= 0xFFFF) { out.push_back(static_cast<char>(0xE0 | (c >> 12))); out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F))); out.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
        else { out.push_back(static_cast<char>(0xF0 | (c >> 18))); out.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F))); out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F))); out.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
    }
    bool Value() {
        Skip();
        if (pos_ >= text_.size()) return false;
        if (text_[pos_] == '"') { std::string ignored; return String(ignored); }
        if (Take('{')) {
            Skip(); if (Take('}')) return true;
            do { std::string key; if (!String(key)) return false; Skip(); if (!Take(':') || !Value()) return false; Skip(); if (Take('}')) return true; if (!Take(',')) return false; Skip(); } while (true);
        }
        if (Take('[')) {
            Skip(); if (Take(']')) return true;
            do { if (!Value()) return false; Skip(); if (Take(']')) return true; if (!Take(',')) return false; Skip(); } while (true);
        }
        if (Literal("true") || Literal("false") || Literal("null")) return true;
        auto start = pos_;
        if (Take('-')) {}
        if (!Take('0')) {
            if (pos_ >= text_.size() || text_[pos_] < '1' || text_[pos_] > '9') return false;
            while (pos_ < text_.size() && text_[pos_] >= '0' && text_[pos_] <= '9') ++pos_;
        }
        if (Take('.')) { auto before = pos_; while (pos_ < text_.size() && text_[pos_] >= '0' && text_[pos_] <= '9') ++pos_; if (before == pos_) return false; }
        if (Take('e') || Take('E')) { if (!Take('+')) Take('-'); auto before = pos_; while (pos_ < text_.size() && text_[pos_] >= '0' && text_[pos_] <= '9') ++pos_; if (before == pos_) return false; }
        return pos_ > start;
    }
    std::string const& text_;
    std::size_t pos_{};
};

} // namespace

bool ParseEnabledConfig(std::string const& json, bool& enabled) noexcept {
    try { bool value = true; if (!JsonReader(json).Parse(value)) return false; enabled = value; return true; }
    catch (...) { return false; }
}

bool LoadEnabledConfig(std::filesystem::path const& path, bool& enabled) noexcept {
    try {
        std::ifstream file(path, std::ios::binary);
        if (!file) return false;
        file.seekg(0, std::ios::end);
        auto const size = file.tellg();
        if (size < 0 || size > 64 * 1024) return false;
        file.seekg(0, std::ios::beg);
        std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return ParseEnabledConfig(text, enabled);
    } catch (...) { return false; }
}

} // namespace fastblockbreak
