Green Island
============

[![GitHub release](https://img.shields.io/github/release/greenisland/greenisland.svg)](https://github.com/greenisland/greenisland)
[![GitHub issues](https://img.shields.io/github/issues/greenisland/greenisland.svg)](https://github.com/greenisland/greenisland/issues)
[![IRC Network](https://img.shields.io/badge/irc-freenode-blue.svg "IRC Freenode")](https://webchat.freenode.net/?channels=hawaii-desktop)

QtQuick-based Wayland compositor in library form.

The name comes from [Kure Atoll, Hawaii](http://en.wikipedia.org/wiki/Green_Island,_Hawaii).

The API extends QtCompositor with additional features needed by any real world
Wayland compositor.

Green Island offers multiple screen support and it also implements specific
protocols such as xdg-shell, gtk-shell and those for Plasma 5.

Additional Wayland protocols for shells built with Green Island are provided,
such as greenisland_applications and greenisland_windows for task managers.

Also include a launcher that executes the compositor on different hardware,
a screencaster protocol and command line application, plus a
minimal Wayland compositor and shell written with QML.

It is primarily developed for the [Hawaii desktop environment](https://github.com/hawaii-desktop),
however it can be used by any desktop environment that wish to implement
its compositor by using QML or for shells deeply integrated with the compositor
in the same process.

## Dependencies

Compiler requirements:

* [gcc >= 4.8](https://gcc.gnu.org/gcc-4.8/) or
* [Clang](http://clang.llvm.org/)

Qt >= 5.5 with at least the following modules is required:

* [qtbase](http://code.qt.io/cgit/qt/qtbase.git)
* [qtdeclarative](http://code.qt.io/cgit/qt/qtdeclarative.git)
* [qtwayland](http://code.qt.io/cgit/qt/qtwayland.git)

Qt 5.4 is supported but you need a qtwayland branch with backports,
see below.

You may also need [Weston >= 1.5.0](http://wayland.freedesktop.org) for
fullscreen shell support.  However if you have Qt >= 5.5 you won't likely need it.

The following modules and their dependencies are required:

* [ECM >= 1.4.0](http://quickgit.kde.org/?p=extra-cmake-modules.git)

### qtwayland

If you are still on Qt 5.4 then you need a special qtwayland branch
with backports from qtwayland 5.5:

```sh
git clone -b output54 https://github.com/maui-packages/qtwayland
```

and build it yourself.

## Build

Building Green Island is a piece of cake.

Assuming you are in the source directory, just create a build directory
and run cmake:

```sh
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/opt/hawaii ..
```

To do a debug build the last command can be:

```sh
cmake -DCMAKE_INSTALL_PREFIX=/opt/hawaii -DCMAKE_BUILD_TYPE=Debug ..
```

To do a release build instead it can be:

```sh
cmake -DCMAKE_INSTALL_PREFIX=/opt/hawaii -DCMAKE_BUILD_TYPE=Release ..
```

If not passed, the `CMAKE_INSTALL_PREFIX` parameter defaults to /usr/local.
You have to specify a path that fits your needs, /opt/hawaii is just an example.

Package maintainers would pass `-DCMAKE_INSTALL_PREFIX=/usr`.

The `CMAKE_BUILD_TYPE` parameter allows the following values:

* **Debug:** debug build
* **Release:** release build
* **RelWithDebInfo:** release build with debugging information

## Installation

It's really easy, it's just a matter of typing:

```sh
make install
```

from the build directory.

# Notes

## Plugins

Green Island can be extend with plugins.
Specify a colon separated list of plugins with the ``GREENISLAND_PLUGINS``
environment variable.

List of plugins:

* **plasma:** Wayland protocols for Plasma

## Logging categories

Qt 5.2 introduced logging categories and Hawaii takes advantage of
them to make debugging easier.

Please refer to the [Qt](http://doc.qt.io/qt-5/qloggingcategory.html) documentation
to learn how to enable them.

### Available categories

* Compositor:
  * **greenisland.compositor:** Compositor
  * **greenisland.screenbackend.native:** Native screen backend
  * **greenisland.screenbackend.fake:** Fake screen backend
  * **greenisland.screenconfiguration:** Screen configuration loader

* Client-side:
  * **greenisland.wlregistry:** Registry
  * **greenisland.wlshmpool:** Shared memory pool

* Protocols:
  * **greenisland.protocols.xdgshell:** xdg_shell
  * **greenisland.protocols.xdgshell.trace:** xdg_shell trace
  * **greenisland.protocols.wlshell:** wl_shell
  * **greenisland.protocols.wlshell.trace:** wl_shell trace
  * **greenisland.protocols.wlsubcompositor.trace:** wl_subcompositor and wl_subsurface trace
  * **greenisland.protocols.gtkshell:** gtk_shell
  * **greenisland.protocols.gtkshell.trace:** gtk_shell trace
  * **greenisland.protocols.greenisland.recorder:** greenisland_recorder
  * **greenisland.protocols.greenisland.screenshooter:** greenisland_screenshooter
  * **greenisland.protocols.fullscreenshell.client:** wl_fullscreen_shell client side

* Plugins:
  * **greenisland.plugins.plasma.shell:** org_kde_plasma_shell protocol
  * **greenisland.plugins.plasma.effects:** org_kde_plasma_effects protocol
