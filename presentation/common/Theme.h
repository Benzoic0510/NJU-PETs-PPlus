//
// Created by BenzoicAcid on 2026/5/12.
//

#ifndef THEME_H
#define THEME_H

namespace Theme {

// ── Brand ────────────────────────────────────────────────
inline constexpr auto Primary     = "#534AB7";
inline constexpr auto PrimaryMid  = "#7F77DD";
inline constexpr auto PrimaryBg   = "#EEEDFE";
inline constexpr auto PrimaryDark = "#3C3489";

// ── Event colors ─────────────────────────────────────────
inline constexpr auto EventPurpleFg  = "#3C3489";
inline constexpr auto EventPurpleBg  = "#EEEDFE";
inline constexpr auto EventPurpleBar = "#534AB7";

inline constexpr auto EventTealFg    = "#085041";
inline constexpr auto EventTealBg    = "#E1F5EE";
inline constexpr auto EventTealBar   = "#1D9E75";

inline constexpr auto EventCoralFg   = "#712B13";
inline constexpr auto EventCoralBg   = "#FAECE7";
inline constexpr auto EventCoralBar  = "#D85A30";

inline constexpr auto EventAmberFg   = "#633806";
inline constexpr auto EventAmberBg   = "#FAEEDA";
inline constexpr auto EventAmberBar  = "#BA7517";

// ── Text ─────────────────────────────────────────────────
inline constexpr auto TextPrimary   = "#1A1A2E";
inline constexpr auto TextSecondary = "#4A4A6A";
inline constexpr auto TextTertiary  = "#8888AA";

// ── Background ───────────────────────────────────────────
inline constexpr auto White       = "#FDFDFF";
inline constexpr auto BgPrimary   = White;
inline constexpr auto BgSecondary = "#F5F5FB";
inline constexpr auto BgSidebar   = "#F8F8FF";

// ── Border ───────────────────────────────────────────────
inline constexpr auto Border          = "#E0E0EE";
inline constexpr auto BorderSecondary = "#EEEEF8";

// ── Sizing ───────────────────────────────────────────────
inline constexpr int SidebarWidth = 200;
inline constexpr int WindowW      = 960;
inline constexpr int WindowH      = 620;
inline constexpr int Radius       = 8;

// ── Typography ───────────────────────────────────────────
inline constexpr int AppFontPointSize = 10;

} // namespace Theme

#endif // THEME_H
