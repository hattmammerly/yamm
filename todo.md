what do i want out of a music manager
===
- playlist union, intersection, difference
- not dependent on itunes for adding music to library
- ability to sync to my phone

applications to meet this end
===
- music manager
  - read/edit playlists in a pg db, display to user
    - including set intersect, union, difference, append (nodelete)
  - can interact with MPD's active playlist (read/update)
  - can read/edit mpd state; play/pause, seek, next, prev, restart
  - track metadata editor? id3v2 tags
  - organize library on filesystem like itunes does; Artist > Album > Track, case-insensitive
  - add music to library from watch folder, or user specifies directory

- migration assistant
  - reads itunes's xml plist
    - case-sensitize filepaths and import to postgres
  - port the playlist template engine from python

- syncing application
  - paired with an android phone over wifi
  - copies tracks to specified folder, organized the same as on computer
  - exports to playlist format that android understands (m3u)

- phone music manager
  - should this be able to modify the library and send changes to the PC to be applied?
    - how to track and merge library changes?
      - maybe keep basically a log w timestamps
    - watchfolder for adding songs purchased on the device
  - maybe copy the PC database to an sqlite database on the android device
    - reuse a lot of code; db model exposes info, ncurses on PC or java on android does stuff

common code
===
- playlist template engine ported from python (used by at least migration assistant, probably sync, maybe music manager)
- stuff to talk to database (used by music manager, at least read by sync, possibly by migration)

tentative database layout
===
tracks
  id integer
  location varchar
  flag integer -- i'll use this as a 'checked' flag, but leave it open for bitmaskability in other clients or we

playlists
  id integer
  title varchar
  length integer -- maybe set triggers, before insert/delete on tracks_playlists where id = new.playlist_id, increment/decrement id
  flag integer -- unused for now

tracks_playlists
  id integer -- is this even needed
  track_id integer
  playlist_id integer
  position integer -- i guess duplicates will have to be addressed in the app


stretch goals
===
maybe some day the music manager could support movie/tv cataloging or we

notes
===
- remember to call setlocale (LC_ALL, ""); first thing in ncurses when working
with unicode. #include <locale.h>
- so it turns out my library isn't entirely .mp3s and i'm not aware of any
  common libs for working with m4a and such metadata. must decide if metadata
  is to be stuffed in the database to be format-agnostic
    - maybe http://taglib.github.io/ will do what i want
