#ifndef DB_H
#define DB_H

#include <postgresql/libpq-fe.h>
#include <string>

PGresult* addTrackToDatabase( PGconn*, std::string, std::string, int );

PGresult* addPlaylistToDatabase( PGconn*, std::string, std::string, int );

PGresult* appendTrackToPlaylist( PGconn*, std::string, std::string );

PGresult* insertTrackIntoPlaylist( PGconn*, std::string, std::string, double);

#endif
