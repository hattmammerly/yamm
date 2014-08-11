i needed a new music manager so i decided to start writing one
here's my 'first commit' readme

plans
===
- migrate an itunes music library to postgresql, or export its playlists to arbitrary formats
- - already done in python; check out my pythonplaylistparser repository if that's all you want
- manage library and playlists on linooks in an ncurses interface, play music through mpd
- sync music to my android phone
- - probably will involve something android-side. maybe i'll write a player app there since the four my phone came with are all broken

what's done
===
- basically nothing
- i spent all my time tracking down a segfault in libplist and writing utility functions that weren't provided
- i wrote a lot of bulleted lists though

depends
===
- id3lib (LGPL)
- libplist (with edits described below) (GPL)
- postrgresql (and libpq) (postgresql license)
- c++11, probably


modifications to libplist
===
- added #include <cstddef> to plist/Node.h. library doesn't compile without it
- in src/Array.cpp i removed the calls to plist_array_remove_item() in the Remove
functions because it seems all it does is call plist_free which is done in the
node destructor anyway. the double free segfaulted, and its removal introduced no
memory concerns in valgrind, so let's call it a fix
