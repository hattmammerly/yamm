/**
 * \file db.cpp
 *
 * \author Matt Hammerly
 */

#include <postgresql/libpq-fe.h>
#include <string>
#include "db.h"

// yeah, yeah, database credentials hardcoded. this will be fixed
const char *database_name = "music";
const char *database_host = "localhost";
const char *database_user = "music";
const char *database_pw = "x8sNVfWqVFuQIxzfCZxk";

/**
 * \brief Connect to the database
 *
 * \return Pointer to connection struct
 */
PGconn* connectToDatabase() {
    char connection_string [512];
    snprintf( connection_string, 512, "dbname=%s host=%s user=%s password=%s", database_name, database_host, database_user, database_pw );
    
    PGconn *conn = PQconnectdb( connection_string );
    
    if (PQstatus(conn) == CONNECTION_BAD) {
        puts("failed to connect to database");
        exit(0);
    }
    
    return conn;
}

/**
 * \brief Add a track to the 'tracks' table
 *
 * \param conn Pointer to the database connection object
 * \param itunes_id The iTunes id of the track, if one exists
 * \param filepath The location of the track on the disk
 * \param flag Integer value in case a bitmask flag becomes necessary
 *
 * \return Results set containing the ID of the added track
 */
PGresult* addTrackToDatabase( PGconn* conn, std::string itunes_id, std::string filepath, int flag) {
    // if i do anything with `flag` it will actually probably be calculated in here and not passed as a parameter
    std::string query = "INSERT INTO tracks (itunes_id, location, flag) VALUES (";
    char* escaped_itunes_id = PQescapeLiteral( conn, itunes_id.c_str(), itunes_id.length() );
    char* escaped_filepath = PQescapeLiteral( conn, filepath.c_str(), filepath.length() );
    query.append( escaped_itunes_id );
    query.append( "," );
    query.append( escaped_filepath );
    query.append( "," );
    query.append( std::to_string(flag) );
    query.append( ") RETURNING id" );

    PGresult* res = PQexec( conn, query.c_str() );
    return res;
}

/**
 * \brief Create a playlist in the 'playlists' table
 *
 * \param conn Pointer to the database connection object
 * \param itunes_id The iTunes id of the playlist, if one exists
 * \param title The title of the playlist
 * \param flag Integer value in case a bitmask flag becomes necessary
 *
 * \return Results set containing the ID of the added playlist
 */
PGresult* addPlaylistToDatabase( PGconn* conn, std::string itunes_id, std::string title, int flag) {
    std::string query = "INSERT INTO playlists (itunes_id, title, flag) VALUES (";
    char* escaped_itunes_id = PQescapeLiteral( conn, itunes_id.c_str(), itunes_id.length() );
    char* escaped_title = PQescapeLiteral( conn, title.c_str(), title.length() );
    query.append( escaped_itunes_id );
    query.append( "," );
    query.append( escaped_title );
    query.append( "," );
    query.append( std::to_string(flag) );
    query.append( ") RETURNING id" );

    PGresult* res = PQexec( conn, query.c_str() );
    return res;
}

/**
 * \brief Append a track to the end of a playlist
 * 
 * \param conn Pointer to the database connection object
 * \param track_id Internal (not iTunes) id of the track to be added
 * \param playlist)id Internal (not iTunes) id of the playlist to add to
 *
 * \return Results set containing the ID of the created association
 *
 * The position of the added track in the playlist will be one higher than the
 * previous highest value; if the previous last track was in position 3.5,
 * this track is in position 4.5. If no tracks are in the playlist, position 1 will be filled.
 */
PGresult* appendTrackToPlaylist( PGconn* conn, std::string track_id, std::string playlist_id) {
    std::string query = "INSERT INTO tracks_playlists (track_id, playlist_id, position) SELECT ";
    char* escaped_track_id = PQescapeLiteral( conn, track_id.c_str(), track_id.length() );
    char* escaped_playlist_id = PQescapeLiteral( conn, playlist_id.c_str(), playlist_id.length() );
    query.append( escaped_track_id );
    query.append( "," );
    query.append( escaped_playlist_id );
    query.append( "," );
    query.append( "FLOOR(COALESCE(MAX(position),0) + 1) FROM tracks_playlists WHERE playlist_id=" );
    query.append( escaped_playlist_id );
    query.append( " RETURNING id" );

    PGresult* res = PQexec( conn, query.c_str() );
    return res;
}

/**
 * \brief Add a track to a playlist with a specified position
 *
 * \param conn Pointer to the database connection object
 * \param track_id Internal (not iTunes) id of the track to be added
 * \param playlist_id Internal (not iTunes) id of the playlist to add to
 * \param position The value for the 'position' field of the track/playlist association
 *
 * \return Results set containing the ID of the created association
 */
PGresult* addTrackToPlaylist( PGconn* conn, std::string track_id, std::string playlist_id, double position) {
    std::string query = "INSERT INTO tracks_playlists (track_id, playlist_id, position) VALUES (";
    char* escaped_track_id = PQescapeLiteral( conn, track_id.c_str(), track_id.length() );
    char* escaped_playlist_id = PQescapeLiteral( conn, playlist_id.c_str(), playlist_id.length() );
    query.append( escaped_track_id );
    query.append( "," );
    query.append( escaped_playlist_id );
    query.append( "," );
    query.append( std::to_string(position) );
    query.append( ") RETURNING id" );

    PGresult* res = PQexec( conn, query.c_str() );
    return res;
}

/**
 * \brief Remove a specified track/playlist association
 *
 * \param conn Pointer to the database connection object
 * \param track_playlist_id ID of the track/playlist association record to be deleted
 *
 * \return I honestly don't know. The ID of the deleted track/playlist association?
 */
PGresult* removeTrackFromPlaylist( PGconn* conn, std::string track_playlist_id ) {
    std::string query = "DELETE FROM tracks_playlists WHERE id=";
    char* escaped_track_playlist_id = PQescapeLiteral( conn, track_playlist_id.c_str(), track_playlist_id.length() );
    query.append( escaped_track_playlist_id );
    query.append( " RETURNING id" ); // why not

    PGresult* res = PQexec( conn, query.c_str() );
    return res;
}

/**
 * \brief Remove a specified playlist from the database
 *
 * \param conn Pointer to the database connection object
 * \param playlist_id Internal ID (not iTunes) of the playlist to be deleted
 *
 * This function removes the playlist record from the playlists table,
 * and also all records of playlist membership from tracks_playlists
 */
PGresult* removePlaylistFromDatabase( PGconn* conn, std::string playlist_id ) {
    std::string query = "DELETE FROM playlists WHERE id=";
    char* escaped_playlist_id = PQescapeLiteral( conn, playlist_id.c_str(), playlist_id.length() );
    query.append( escaped_playlist_id );
    query.append( "; DELETE FROM tracks_playlists WHERE playlist_id=" );
    query.append( escaped_playlist_id );

    PGresult* res = PQexec( conn, query.c_str() );
    return res;
}

/**
 * \brief Remove a specified track from the database
 *
 * \param conn Pointer to the database connection object
 * \param track_id Internal ID (not iTunes) of the track to be deleted
 *
 * This function removes the track record from the tracks table,
 * and also all records of playlist membership from tracks_playlists
 */
PGresult* removeTrackFromDatabase( PGconn* conn, std::string track_id ) {
    std::string query = "DELETE FROM tracks WHERE id=";
    char* escaped_track_id = PQescapeLiteral( conn, track_id.c_str(), track_id.length() );
    query.append( escaped_track_id );
    query.append( "; DELETE FROM tracks_playlists WHERE track_id=" );
    query.append( escaped_track_id );

    PGresult* res = PQexec( conn, query.c_str() );
    return res;
}
