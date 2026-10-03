find_package(Wayland REQUIRED COMPONENTS Client SessionLock)
find_package(XKB REQUIRED)
find_package(PAM REQUIRED)

spm_require_package(NAME stb_image VERSION 2026.08.02)

set(LINK_LIBS
        Wayland::Client
        Wayland::SessionLock
        PAM::pam
        XKB::XKB
        stb_image::stb_image
)
