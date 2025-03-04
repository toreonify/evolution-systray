Name: evolution-systray
Version: 0.2
Release: alt1

Summary: Tray icon for Evolution

License: LGPLv2+
Group: Office
Url: https://github.com/toreonify/evolution-systray

Requires: evolution libstatusnotifier

BuildRequires: evolution-devel evolution-data-server-devel libstatusnotifier-devel 

Source: %name-%version.tar

%description
Minimize-to-tray extension for Gnome Evolution email client

%prep
%setup

%build
%make_build prefix=%buildroot%_prefix

%install
%makeinstall_std prefix=%buildroot%_prefix libdir=%buildroot%_libdir
%find_lang %name

%files -f %name.lang
%_libdir

%changelog
* Tue Jan 30 2024 Ivan Korytov <toreonify@altlinux.org> 0.2-alt1
- Updated to v0.2

* Thu Dec 28 2023 Ivan Korytov <toreonify@altlinux.org> 0.1-alt1
- Initial build for ALT Linux
