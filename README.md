# evolution-systray

This plugin enables Evolution to hide in system tray. Supports X11 and Wayland.

# Dependencies

- [statusnotifier](https://github.com/jjk-jacky/statusnotifier)
- [Evolution development libraries](https://gitlab.gnome.org/GNOME/evolution/)

Tested on: ALT Linux Workstation K 10.1 (x86_64), ALT Regular

# FAQ

**Q:** Evolution doesn't show up  
**A:** Check focus stealing settings in your desktop environment if you are using X11.

**Q:** Showing up Evolution from tray looks strange on Wayland  
**A:** Wayland doesn't support minimized/iconified state of the window and doesn't allow to unminimize it  
**Current workaround:**  Evolution is always "closed"/hiddden before showing because opening a new window will place it on top of other windows. This introduces not so pretty behavior of a flashing window and a jumping icon in taskbar.  
**Alternative workaround:** If you have XWayland installed, you can set `GDK_BACKEND=x11` environment variable to run Evolution as X11 application. This will allow it to show on top of other windows.

# About

Based on Serghei Amelian (@serghei) evolution-systray project. Previous version used XApp to create tray icon, but in KDE Plasma environment the icon displayed incorrectly (stretched, wrong size). Current version uses KDE StatusNotifier through wrapper library to resolve this issue.
