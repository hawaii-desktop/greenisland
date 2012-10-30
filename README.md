Green Island
============

This is Green Island, the Wayland compositor for the Hawaii desktop.

The name comes from Kure Atoll, Hawaii, see here:
  http://en.wikipedia.org/wiki/Green_Island,_Hawaii

It's a simple Qt-based Wayland compositor that can be extended by
specific shells for different kind of workflows.  At the moment
the focus is on the desktop shell, but in the future a tablet
version is expected to see the light of the day.

Dependencies
============

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

Build
=====

Building Green Island is a piece of cake.

Assuming you are in the source directory, just create a build directory
and run cmake:

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/system ..

To do a debug build the last command can be:

    cmake -DCMAKE_INSTALL_PREFIX=/system -DCMAKE_BUILD_TYPE=Debug ..

To do a release build instead it can be:

    cmake -DCMAKE_INSTALL_PREFIX=/system -DCMAKE_BUILD_TYPE=Release ..

The CMAKE_BUILD_TYPE parameter allows the following values:

    Debug: debug build
    Release: release build
    RelWithDebInfo: release build with debugging information

Installation
============

The installation is easy-peasy, it's just a matter of typing:

    sudo make install

from the build directory.

Run as a X window
=================

You can run Green Island in a X11 window for development and debugging purpose.
Remember to set the environment variable before running it:

```sh
export XDG_RUNTIME_DIR=/tmp
greenisland -platform xcb
```

Run from KMS
============

On production systems you can run Green Island under KMS as root:

```sh
export XDG_RUNTIME_DIR=/tmp
export QT_KMS_TTYKBD=1
greenisland -platform kms -plugin EvdevMouse -plugin EvdevKeyboard -plugin EvdevTouch
```

**Remember:** at the moment Green Island under KMS must be run as the root user.

Run on RaspberryPi
==================

```sh
export XDG_RUNTIME_DIR=/tmp
greenisland -platform eglfsrpi
```
