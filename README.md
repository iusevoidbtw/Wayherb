# mayflower
daemon-less notifications without dbus for wayland.

a project that intends to maintain the now seemingly unmaintained [wayherb](https://github.com/Vixeliz/Wayherb) project, with improvements and bugfixes.  

depends on:
- libwayland + development headers
- cairo + development headers

## building

a simple `make` should be enough. if your system doesn't have `cc` as an alias to a C compiler, specify it manually with, for example, `make CC=gcc`.

you might want to edit `config.h` before building; see the configuration section.

mayflower has been tested to compile and run successfully with gcc, clang and tcc.

## usage

run:  
`mayflower hello`  
to display a notification with the text `hello`.  
arguments are automatically concatenated, so `mayflower hello world` will display "hello world".  

notifications can be dismissed with `DISMISS_BUTTON` (set in config.h, left-click by default). this causes mayflower to exit with a status of EXIT_DISMISS. notifications are also automatically dismissed after `duration` (by default 5) seconds.

notifications can also be accepted with `ACTION_BUTTON` (right-click by default). this causes mayflower to exit with a status of 0 and can be used for actions, e.g:  
`mayflower notification && echo "notification accepted"`

notifications can be dismissed/accepted with signals:
```
pkill -SIGUSR1 mayflower # dismiss notification
pkill -SIGUSR2 mayflower # accept notification
```

to change the `duration` time after which the notification is auto-dismissed, use the `-d`/`--duration` option:
```
mayflower -d 25 very important notification # will stay on-screen for 25 seconds!
mayflower -d 0 another important notification # setting the duration to 0 disables auto-dismiss
```

## configuration

mayflower can be configured by copying `config.def.h` to `config.h` (done automatically in the makefile) and editing `config.h`.  

the options should be fairly self-explanatory.

## why "may"?

because:

- it rhymes with "way"
- my cat is named May
- as of writing this the month is currently May
