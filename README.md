# pengsh

A puush.me client for Linux, using Qt 5.

## Known bugs
  * Unfortunately, there is a Qt bug that prevents Qt Multimedia from supporting both ALSA and PulseAudio, so your distribution's Qt Multimedia packages will likely refuse to function without PulseAudio. That means the notification sound will fail to play under systems without PulseAudio. The least intrusive solution I have found in those cases is to setup PulseAudio to use ALSA's dmix device, and disable the udev auto-detect module. In this case, you do not want to install PulseAudio's ALSA compatibility layer, as programs running under PulseAudio will experience both the latency of ALSA dmix along with the latency of PulseAudio.
  
  * All of the functions work properly if invoked from the global keyboard shortcut. However, invoking "Capture Area" from the menu somehow causes a permanent keyboard + mouse grab. For some reason, the XF86Ungrab key doesn't seem to fix this. Save yourself some trouble and set up OpenSSH before you run the program, just in case you need to SSH in to kill it and release the grabs (Sorry.) The "Capture Active Window" functionality also does not work properly when invoked from the menu, as it is unable to detect the active window. A solution that wouldn't be a race condition is not apparent.

  * The program was only tested under the i3 window manager. It probably isn't going to work well across the many different window managers.
  

## Installation

### Prerequisites
pengsh requires the following Qt submodules: core, gui, gui-private, widgets, network, x11extras, multimedia. Under Arch Linux, these are provided by the qt5-base, qt5-x11extras and qt5-multimedia packages. Under Debian Linux, these are provided by the qtbase5-dev, qtbase5-private-dev, and libqt5x11extras-dev packages.

### Building
pengsh currently uses the qmake build system. It may be updated to use the Qt Build System in the future. To build, open a terminal in the directory, and:

```
mkdir build
cd build
qmake ..
make -j4
```

### Installing
A pengsh build consists of a single binary. You can install it with `sudo make install`.

## Usage
When you first load pengsh, it will prompt you for your puush.me login credentials. Your password will not be stored, but your API key (which is effectively as powerful) will be. After this, should see an icon in your tray for pengsh. Right click it to access the pengsh menu - you can access your settings, your account and your 5 most recent puushes from here, as well as trigger a screenshot or file upload (but see known bugs before doing so, please.)

The default keyboard shortcut scheme is similar to the official puush.me client:

  * __Ctrl+Shift+2__: Capture the entire desktop. On TwinView or Xinerama multihead setups, this should capture the entire virtual desktop.
  * __Ctrl+Shift+3__: Capture the current window. pengsh attempts to include the window borders, but this is difficult on some window managers.
  * __Ctrl+Shift+4__: Capture an area. You will be presented with a rectangle select. Any key breaks out of it.
  * __Ctrl+Shift+5__: Upload the clipboard. This will take whatever is in the clipboard and upload it. Please note: the clipboard, not the primary selection buffer.
  * __Ctrl+Shift+U__: Upload a file. Currently pops up a file dialog. There is no file manager integration a la puush.me yet.

You can switch accounts by logging out in settings. If some settings causes pengsh to be unusable, the settings should be stored in `~/.config/jchadwick/pengsh.conf` and removing this file will reset everything.
