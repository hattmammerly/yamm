#include <postgresql/libpq-fe.h>
#include <string>
#include "db.h"

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

// add track to playlist; 'position' yet to be implemented
PGresult* appendTrackToPlaylist( PGconn* conn, std::string track_id, std::string playlist_id) {
    std::string query = "INSERT INTO tracks_playlists (track_id, playlist_id, position) SELECT ";
    char* escaped_track_id = PQescapeLiteral( conn, track_id.c_str(), track_id.length() );
    char* escaped_playlist_id = PQescapeLiteral( conn, playlist_id.c_str(), playlist_id.length() );
    query.append( escaped_track_id );
    query.append( "," );
    query.append( escaped_playlist_id );
    query.append( "," );
    query.append( "(COALESCE(MAX(position),0) + 1) FROM tracks_playlists WHERE playlist_id=" );
    query.append( escaped_playlist_id );
    query.append( " RETURNING id" );

    PGresult* res = PQexec( conn, query.c_str() );
    return res;
}
