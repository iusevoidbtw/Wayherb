# mayherb
daemon-less notifications without dbus for wayland.

a project that intends to maintain the now seemingly unmaintained [wayherb](https://github.com/Vixeliz/Wayherb) project, with improvements and bugfixes.  

depends on:
- libwayland + development headers
- cairo + development headers

## usage

run `mayherb hello` to display a notification with the text `hello`.
arguments are automatically concatenated, so `mayherb hello world` will display "hello world".  

notifications can be dismissed with `DISMISS_BUTTON` (set in config.h, left-click by default). this causes mayherb to exit with a status of EXIT_DISMISS.

notifications can also be accepted with `ACTION_BUTTON` (right-click by default). this causes mayherb to exit with a status of 0 and can be used for actions, e.g:  
`mayherb notification && echo "notification accepted"`

you can pipe things to mayherb with `<cmd> | xargs mayherb`.

## configuration

mayherb can be configured by copying `config.def.h` to `config.h` (done automatically in the makefile) and editing `config.h`.  

the options should be fairly self-explanatory.
