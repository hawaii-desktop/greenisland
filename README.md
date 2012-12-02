Green Island
============

This is Green Island, the Wayland compositor for the Hawaii desktop.

The name comes from Kure Atoll, Hawaii, see here:
  http://en.wikipedia.org/wiki/Green_Island,_Hawaii

It's a simple Qt-based Wayland compositor that can be extended by
specific shells for different kind of workflows.  At the moment
the focus is on the desktop shell, but in the future a tablet
version is expected to see the light of the day.

## Dependencies

In order to build and install Green Island, you will need a complete
and up to date Wayland, Qt 5 and Vibe development environment.

The Wayland site has some information to bring it up:

  http://wayland.freedesktop.org/building.html

More information about building Qt 5 can be found here:

  http://qt-project.org/wiki/Building-Qt-5-from-Git

Vibe and other Hawaii components can be easily built with our
Continuous Integration tool, read the instructions here:

  https://github.com/hawaii-desktop/hawaii

The Continuous Integration tool builds the whole desktop.

## Build

Building Green Island is a piece of cake.

Assuming you are in the source directory, just create a build directory
and run cmake:

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/opt/hawaii ..

To do a debug build the last command can be:

    cmake -DCMAKE_INSTALL_PREFIX=/opt/hawaii -DCMAKE_BUILD_TYPE=Debug ..

To do a release build instead it can be:

    cmake -DCMAKE_INSTALL_PREFIX=/opt/hawaii -DCMAKE_BUILD_TYPE=Release ..

If not passed, the CMAKE_INSTALL_PREFIX parameter defaults to /usr/local.
You have to specify a path that fits your needs, /opt/hawaii is just an example.

Package maintainers would pass *-DCMAKE_INSTALL_PREFIX=/usr*.

The CMAKE_BUILD_TYPE parameter allows the following values:

    Debug: debug build
    Release: release build
    RelWithDebInfo: release build with debugging information

## Installation

It's really easy, it's just a matter of typing:

    make install

from the build directory.

Running Green Island on X11 and KMS
===================================

You can run Green Island in a X11 window or on KMS, in both cases the right
QPA platform plugin should be detected automatically.

If the DISPLAY environment variable is set, the xcb QPA plugin is used otherwise
kms is assumed.

All relevant environment variables are automatically set except XDG_RUNTIME_DIR.
Almost all GNU/Linux distributions has XDG_RUNTIME_DIR correctly set, if it's not
the case read the [XDG Base Directory Specification](http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html).

**Remember:** at the moment Green Island under KMS must be run by the root user.

Running Green Island on Raspberry Pi
====================================

For starters take a look at the [Qt Project wiki page](http://qt-project.org/wiki/RaspberryPi).

The RaspberryPi platform is **not** automatically detected, you must run
Green Island with the -platform argument and set the environment:

```sh
greenisland -platform eglfs
```

You might want to use the Raspberry Pi QPA platform plugin from the Mer project instead of Qt's eglfs.
The source code is [here](https://build.pub.meego.com/package/files?package=qtplatformplugin-rpi&project=CE%3AAdaptation%3ARaspberryPi).
After you built and installed it you can run Green Island like this:

```sh
greenisland -platform eglfsrpi
```

The XDG_RUNTIME_DIR environment variable is required, if your GNU/Linux distribution
doesn't take care of it consult the [XDG Base Directory Specification](http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html).

What should I do if something is wrong?
=======================================

If something is wrong like *greenisland* doesn't run or you only see the desktop
background without any panel, we need to see a more verbose output to figure out
what's happening.

Set the following environment variables before running the *greenisland* command:

```sh
export MESA_DEBUG=1
export EGL_LOG_LEVEL=debug
export LIBGL_DEBUG=verbose
export WAYLAND_DEBUG=1
```

Run the *greenisland* with the parameters that suit your needs.
Send the output to the developers along with a detailed explaination of what's
happening and how did you run *greenisland*.
