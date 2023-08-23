#pragma once

#include <unordered_map>

class FontDatabase {
public:
    enum class Style {
        Regular,
        Large,
        Monospace,
    };

    void add_font(Style style, ImFont* font) {
        font_map[style] = font;
    }

    void push_style(Style style) {
        ImGui::PushFont(font_map[style]);
    }

    void pop_style() {
        ImGui::PopFont();
    }

private:
    std::unordered_map<Style, ImFont*> font_map;
};