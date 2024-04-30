# SimpleQmlWithLibMPV

This is an example of using libmpv with Qt6 QML

## Always show new window when use libmpv?
If you are facing the problem that use the example code of libmpv with qml but found it always shows a new player window instead of draw on qml, congraduations! you get the right place.

The problme is that you need to set an option to use libmpv not mpv as the video output
```cpp
mpv::qt::set_option_variant(mpvHandle, "vo", "libmpv");
```
