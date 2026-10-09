# About WSecureLock

WSecureLock is an wayland screen lock utility designed to be simple, inspired by
[XSecureLock](https://github.com/google/xsecurelock).

In XSecureLock, security is achieved using a modular design, for extensible utilities.

# Requirements

The minimum dependencies:

* cmake (>= 3.27)
* pkg-config
* ninja
* git
* clang or gcc

* cacert
* wayland
* wayland-scanner
* wayland-protocols
* libxkbcommon
* pam

# Installation

```
git clone https://codeberg.org/JassJam/wsecurelock.git
cd wsecurelock
# configure with flake
# NO_AUTOCONFIGURE=0 nix run .#pick
cmake -S . -B build --preset unixlike-x86_64-clang-debug
cmake --build build
```

# License

The code is released under the MIT. See the LICENSE file for more details.
