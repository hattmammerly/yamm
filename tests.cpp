/**
 * \file tests.cpp
 *
 * \author Matt Hammerly
 */

#include <postgresql/libpq-fe.h>
#include <string>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <curl/curl.h>
#include <plist/plist++.h>
#include <dirent.h>
#include <sys/types.h>
#include <cctype>
#include <cstring>
#include <cassert>
#include "ma.h"
#include "db.h"
#include "tests.h"

/**
 * \brief Entry point into test program
 */
int main() {

    PGconn* conn = connectToDatabase();

    dropTables(conn);

    createTables(conn);

    PList::Dictionary* library = openLibrary();

    // test caseSensitiveFilePath()
    test_caseSensitiveFilePath();

    // test adding track to database
    test_addTrackToDatabase(conn);

    // test adding playlist to database
    test_addPlaylistToDatabase(conn);

    // test fractional inserts into playlists
    test_track_inserts(conn);

    // test track removal from playlists
    test_removal(conn);

    // test playlist normalization
    test_playlist_normalization(conn);

    puts( PQerrorMessage(conn) );

    return 0;
}

/**
 * \brief Test caseSensitiveFilePath() function
 */
void test_caseSensitiveFilePath() {
    std::string new_library_path = "/home/matt/hdd/dev/yamm/test_data/";
    std::string wrong = new_library_path + "beirut/THe Rip tiDe/01 A candle's Fire.mp3";
    std::string right = new_library_path + "Beirut/The Rip Tide/01 A Candle's Fire.mp3";

    assert( caseSensitiveFilePath(wrong) == right );
    puts( "caseSensitiveFilePath() works correctly!" );
}

/**
 * \brief Test addTrackToDatabase(...)
 *
 * \param conn Pointer to database connection struct
 */
void test_addTrackToDatabase(PGconn* conn) {
    std::string new_library_path = "/home/matt/hdd/dev/yamm/test_data/";

    PGresult* res = addTrackToDatabase( conn, "", new_library_path + "Beirut/The Rip Tide/01 A Candle's Fire.mp3", 0 );
    char* track_id = PQgetvalue( res, 0, 0 );
    char const *expected_id = "1";

    assert ( strcmp(track_id, expected_id) == 0 );
    puts ("addTrackToDatabase(...) works correctly!" );

    PQclear(res);
}

/**
 * \brief Test addPlaylistToDatabase(...)
 *
 * \param conn Pointer to database connection struct
 */
void test_addPlaylistToDatabase(PGconn* conn) {
    PGresult* res = addPlaylistToDatabase( conn, "", "Test Playlist", 0 );
    char *playlist_id = PQgetvalue( res, 0, 0 );
    char const *expected_id = "1";

    assert( strcmp( playlist_id, expected_id) == 0 );
    puts ("addPlaylistToDatabase(...) works correctly!" );

    PQclear(res);
}

/**
 * \brief Test appendTrackToPlaylist(...) and addTrackToPlaylist(...) functions
 *
 * \param conn Pointer to database connection struct
 *
 * Todo: check to make sure the fractional insert is ordered correctly
 */
void test_track_inserts(PGconn* conn) {
    // SELECT * FROM tracks_playlists Link LEFT JOIN playlists Playlist ON Link.playlist_id = Playlist.id WHERE Playlist.title = 'testing fractional inserts' ORDER BY position;
    PGresult* playlist_insert_test_res = addPlaylistToDatabase( conn, "", "testing fractional inserts", 0);
    char* playlist_id = PQgetvalue( playlist_insert_test_res, 0, 0 );
    PGresult* track_insert_res = appendTrackToPlaylist( conn, "1", playlist_id );
    char* association_id = PQgetvalue( track_insert_res, 0, 0 );
    assert( strcmp( association_id, "1") == 0 );
    track_insert_res = appendTrackToPlaylist( conn, "2", playlist_id );
    track_insert_res = appendTrackToPlaylist( conn, "4", playlist_id );
    association_id = PQgetvalue( track_insert_res, 0, 0 );
    assert( strcmp(association_id, "3") == 0 );
    puts ("appendTrackToPlaylist(...) works correctly!");
    track_insert_res = addTrackToPlaylist( conn, "3", playlist_id, 2.5 );
    association_id = PQgetvalue( track_insert_res, 0, 0 );
    assert( strcmp(association_id, "4") == 0 );
    puts ("addTrackToPlaylist(...) works correctly!");
    PQclear( playlist_insert_test_res );
    PQclear( track_insert_res );
}

/**
 * \brief Test normalizePlaylist(...) or whatever I called it
 *
 * \param conn Pointer to database connection struct
 */
void test_playlist_normalization(PGconn* conn) {
    PGresult* playlist_insert_res = addPlaylistToDatabase( conn, "", "testing normalizing playlist", 0);
    char* playlist_id = PQgetvalue( playlist_insert_res, 0, 0 );
    PGresult* track_insert_res = addTrackToPlaylist( conn, "1", playlist_id, 0.5 );
    char* association_id = PQgetvalue( track_insert_res, 0, 0 );
    PGresult* normalize_playlist_res = normalizePlaylist( conn, std::string(playlist_id) );

    char* association_id_2 = PQgetvalue( normalize_playlist_res, 0, 0 );
    char* track_id = PQgetvalue( normalize_playlist_res, 0, 1 );
    char* playlist_id_2 = PQgetvalue( normalize_playlist_res, 0, 2 );
    char* position = PQgetvalue( normalize_playlist_res, 0, 3 );

    assert( strcmp(association_id, association_id_2) == 0 );
    assert( strcmp(track_id, "1") == 0 );
    assert( strcmp(playlist_id, playlist_id_2) == 0 );
    assert( strcmp(position, "1") == 0 );

    puts ("normalizePlaylist(...) works correctly!");
    PQclear( playlist_insert_res );
    PQclear( track_insert_res );
    PQclear( normalize_playlist_res );
}

/**
 * \brief Test removeTrackFromPlaylist(...) assuming appendTrackToPlaylist works fine
 *
 * \param conn Pointer to database connection struct
 */
void test_removal(PGconn* conn) {
    // SELECT * FROM tracks_playlists Link LEFT JOIN playlists Playlist ON Link.playlist_id = Playlist.id WHERE Playlist.title = 'testing track removal' ORDER BY position;
    PGresult* playlist_insert_test_res = addPlaylistToDatabase( conn, "", "testing track removal", 0);
    char* playlist_id = PQgetvalue( playlist_insert_test_res, 0, 0 );
    PGresult* track_insert_res = appendTrackToPlaylist( conn, "1", playlist_id );
    track_insert_res = appendTrackToPlaylist( conn, "2", playlist_id );
    track_insert_res = appendTrackToPlaylist( conn, "4", playlist_id );
    char* association_id = PQgetvalue( track_insert_res, 0, 0 );
    track_insert_res = removeTrackFromPlaylist( conn, association_id );
    association_id = PQgetvalue( track_insert_res, 0, 0 );
    assert( strcmp(association_id, "7") == 0 );
    puts("removeTrackFromPlaylist(...) works correctly!");
    track_insert_res = appendTrackToPlaylist( conn, "3", playlist_id );
    PQclear( playlist_insert_test_res );
    PQclear( track_insert_res );
}
