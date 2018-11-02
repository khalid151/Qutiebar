# Qutiebar
A customizable panel made with Qt inspired by [polybar](https://github.com/jaagr/polybar).

*still under construction*

## Dependencies
- Compiler with C++14 support
- `cmake` (and `extra-cmake-modules`)
- `pkgconf`
- `libxcb`
- `xcb-util-wm`
- `qt5-base`
- `qt5-x11extras`

### Optional dependencies
- `alsa-lib` (for volume module)
- `libmpdclient` (for mpd music control\display module)

### Installing dependencies on Arch
```
$ sudo pacman -S cmake extra-cmake-modules pkgconf \
    libxcb xcb-util-wm qt5-base qt5-x11extras \
    alsa-lib libmpdclient
```

### Installing dependencies on Debian\Ubuntu
```
$ sudo apt install cmake extra-cmake-modules pkgconf \
    libxcb1-dev libxcb-ewmh-dev \
    libxcb-icccm4-dev qtbase5-dev libqt5x11extras5-dev \
    libasound2-dev libmpdclient-dev
```

## Build and Install
```
$ git clone https://github.com/khalid151/Qutiebar
$ mkdir Qutiebar/build
$ cd Qutiebar/build
$ cmake ..
$ make
$ sudo make install
```

## TODO
- [ ] Menu module
- [ ] Systray
- [ ] Task switcher
