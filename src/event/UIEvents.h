#pragma once

enum class Theme { Light, Dark };

struct ThemeChangedEvent {
    Theme theme;
};