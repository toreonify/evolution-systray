# evolution-systray

This plugin enables Evolution to hide in system tray.

# Dependencies

- [statusnotifier](https://github.com/jjk-jacky/statusnotifier)
- [Evolution development libraries](https://gitlab.gnome.org/GNOME/evolution/)

Tested on: ALT Linux Workstation K 10.1 (x86_64)

# About

Based on Serghei Amelian (@serghei) evolution-systray project. Previous version used XApp to create tray icon, but in KDE Plasma environment the icon displayed incorrectly (stretched, wrong size). Current version uses KDE StatusNotifier through wrapper library to resolve this issue.
