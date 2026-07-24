// ember_theme.h — EmberDragon's Binary-Ninja-style dark theme: one place for the
// palette, syntax colors, and layout metrics so every panel stays consistent.
#ifndef EMBER_THEME_H
#define EMBER_THEME_H

namespace th {
struct Col { float r, g, b, a = 1.0f; };

// ── surfaces (BN-ish dark) ───────────────────────────────────────────────────
constexpr Col WINDOW   {0.086f, 0.090f, 0.102f};   // app background
constexpr Col PANEL    {0.110f, 0.114f, 0.129f};   // panel body
constexpr Col PANEL_HI {0.137f, 0.141f, 0.161f};   // raised / header
constexpr Col TOOLBAR  {0.149f, 0.153f, 0.176f};   // toolbar / tab strip
constexpr Col SUNKEN   {0.071f, 0.075f, 0.086f};   // gutter / inputs
constexpr Col BORDER   {0.043f, 0.045f, 0.055f};   // hairline dividers
constexpr Col HOVER    {0.196f, 0.204f, 0.235f};   // hover wash
constexpr Col SEL      {0.157f, 0.247f, 0.357f};   // selection (BN blue)
constexpr Col SEL_DIM  {0.122f, 0.165f, 0.227f};   // inactive selection
constexpr Col SCROLL   {0.286f, 0.298f, 0.345f};   // scrollbar thumb

// ── text ─────────────────────────────────────────────────────────────────────
constexpr Col TEXT     {0.851f, 0.859f, 0.878f};
constexpr Col TEXT_DIM {0.435f, 0.451f, 0.498f};
constexpr Col TEXT_MUT {0.310f, 0.322f, 0.365f};

// ── accents ──────────────────────────────────────────────────────────────────
constexpr Col ACCENT   {1.000f, 0.553f, 0.176f};   // ember orange
constexpr Col ACCENT2  {0.847f, 0.337f, 0.616f};   // pink
constexpr Col GREEN    {0.486f, 0.749f, 0.420f};
constexpr Col RED      {0.878f, 0.392f, 0.396f};

// ── syntax (shared by pseudocode + disasm) ───────────────────────────────────
constexpr Col KW   {0.510f, 0.631f, 0.851f};   // keywords
constexpr Col TYPE {0.337f, 0.745f, 0.706f};   // types
constexpr Col CMT  {0.420f, 0.553f, 0.404f};   // comments
constexpr Col STR  {0.831f, 0.604f, 0.486f};   // strings
constexpr Col FNC  {0.871f, 0.812f, 0.471f};   // function names
constexpr Col NUM  {0.706f, 0.808f, 0.659f};   // numbers / addresses
constexpr Col REG  {0.741f, 0.576f, 0.976f};   // registers (disasm)
constexpr Col PUNC {0.553f, 0.573f, 0.620f};   // punctuation

// type-chip colors by symbol kind (function/class/struct/data)
constexpr Col CHIP_FN{1.000f, 0.553f, 0.176f}, CHIP_CLASS{0.337f, 0.745f, 0.706f},
              CHIP_STRUCT{0.847f, 0.337f, 0.616f}, CHIP_DATA{0.435f, 0.451f, 0.498f};

// ── metrics (logical px; multiply by backing for device px) ──────────────────
constexpr float TOOLBAR_H = 34;   // toolbar height
constexpr float TAB_H     = 26;   // tab strip height
constexpr float STATUS_H  = 22;   // status bar
constexpr float ROW_PAD   = 6;    // list row vertical padding
constexpr float PAD       = 8;    // generic padding
} // namespace th
#endif
