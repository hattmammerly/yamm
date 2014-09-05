/**
 * \file db.h
 * 
 * \author Matt Hammerly
 *
 * \brief Defines functions for working with the music database
 *
 * Included in this file are functions for working with tracks
 * and playlists in the music database this application creates and uses.
 */

#ifndef DB_H
#define DB_H

#include <postgresql/libpq-fe.h>
#include <string>

PGconn* connectToDatabase();

PGresult* addTrackToDatabase( PGconn*, std::string, std::string, int );

PGresult* addPlaylistToDatabase( PGconn*, std::string, std::string, int );

PGresult* appendTrackToPlaylist( PGconn*, std::string, std::string );

PGresult* addTrackToPlaylist( PGconn*, std::string, std::string, double);

PGresult* removeTrackFromPlaylist( PGconn*, std::string );

PGresult* removePlaylistFromDatabase( PGconn*, std::string );

PGresult* removeTrackFromDatabase( PGconn*, std::string );

#endif
