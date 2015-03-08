Green Island
============

This is Green Island, a Qt-based Wayland compositor in library form.

The name comes from [Kure Atoll, Hawaii](http://en.wikipedia.org/wiki/Green_Island,_Hawaii).

The API is based on QtCompositor which is shipped by the qtwayland module.

Green Island offers multiple screen support and it also implements specific protocols
such as those for Plasma 5 that do not make much sense to be sent upstream.

Green Island can be used by any desktop environment that wish to implement its compositor
by using QML or for shells deeply integrated with the compositor in the same process.

It is primarily developed for the [Hawaii desktop environment](https://github.com/hawaii-desktop).

## Dependencies

* Wayland >= 1.6.0
* Qt >= 5.4.0 with at least the follow modules:
  * qtbase
  * qtdeclarative
  * qtwayland
* ECM >= 1.4.0
* libkscreen >= 5.2.0


A development version of qtwayland is required.

Please clone the output54 branch if you are using Qt 5.4.x:

```sh
git clone -b output54 https://github.com/maui-packages/qtwayland
```

or output55 if you are using Qt 5.5.x:

```sh
git clone -b output55 https://github.com/maui-packages/qtwayland
```

and build it yourself.

## Optional dependencies

* Weston >= 1.5.0 to nest the compositor into fullscreen shell

## Additional information on dependencies

Read Wayland and Weston build instructions from [here](http://wayland.freedesktop.org/building.html).

More information about building Qt from git can be found [here](http://qt-project.org/wiki/Building-Qt-5-from-Git).

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
