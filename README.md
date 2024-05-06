# mayherb
daemon-less notifications without dbus for wayland.

a project that intends to maintain the now seemingly unmaintained [wayherb](https://github.com/Vixeliz/Wayherb) project, with improvements and bugfixes.  

depends on:
- libwayland + development headers
- cairo + development headers

## building

a simple `make` should be enough. if your system doesn't have `cc` as an alias to a C compiler, specify it manually with, for example, `make CC=gcc`.

you might want to edit `config.h` before building; see the configuration section.

mayherb has been tested to compile and run successfully with gcc, clang and tcc.

## usage

run `mayherb hello` to display a notification with the text `hello`.  
arguments are automatically concatenated, so `mayherb hello world` will display "hello world".  

notifications can be dismissed with `DISMISS_BUTTON` (set in config.h, left-click by default). this causes mayherb to exit with a status of EXIT_DISMISS. notifications are also automatically dismissed after `duration` (by default 5) seconds.

notifications can also be accepted with `ACTION_BUTTON` (right-click by default). this causes mayherb to exit with a status of 0 and can be used for actions, e.g:  
`mayherb notification && echo "notification accepted"`

notifications can be dismissed/accepted with signals:
```
pkill -SIGUSR1 mayherb # dismiss notification
pkill -SIGUSR2 mayherb # accept notification
```

you can pipe things to mayherb with `<cmd> | xargs mayherb`.

## configuration

mayherb can be configured by copying `config.def.h` to `config.h` (done automatically in the makefile) and editing `config.h`.  

the options should be fairly self-explanatory.

## why is named "mayherb"?

because:

- it rhymes with "way"
- my cat is named May
- as of writing this the month is currently May
