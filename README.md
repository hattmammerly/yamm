i needed a new music manager so i decided to start writing one  
here's my 'first commit' readme

plans
===
- migrate an itunes music library to postgresql, or export its playlists to arbitrary formats
    - already done in python; check out my pythonplaylistparser repository if that's all you want
- manage library and playlists on linooks in an ncurses interface, play music through mpd
- sync music to my android phone
    - go-mtpfs allows mounting of android 4.x devices p nicely so i will likely use this to rsync a library or something

depends
===
- libplist (with edits described below) (GPL)
- postrgresql (and libpq) (postgresql license)
- c++11, probably
- probably taglib (LGPL)
- probably ncurses

what's done
===
- basically nothing
- i spent all my time tracking down a segfault in libplist and writing utility functions that weren't provided
  - i have since discarded all of that code and don't use the segfaulting part anymore haha
- i wrote a lot of bulleted lists though
- can read iTunes library, can put things in database

immediate plans
===
- write some Real (Enough) Tests for the code i have now
- work with taglib to put metadata from the iTunes library in the track if not there

modifications to libplist
===
- added #include <cstddef> to plist/Node.h. library doesn't compile without it
- in src/Array.cpp i removed the calls to plist_array_remove_item() in the Remove
functions because it seems all it does is call plist_free which is done in the
node destructor anyway. the double free segfaulted, and its removal introduced no /new/
memory concerns in valgrind, so let's call it a fix
- i sent a pull request that might get merged someday
