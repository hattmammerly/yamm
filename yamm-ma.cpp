/**
 * \file yamm-ma.cpp
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
#include "yamm-ma.h"
#include "db.h"

// this stuff will be in some config file in the future
char* library_file = "/home/matt/hdd/dev/yamm/test_data/iTunes Library.xml";
std::string new_library_path = "/home/matt/hdd/dev/yamm/test_data/";
char* database_name = "music";
char* database_host = "localhost";
char* database_user = "music";
char* database_pw = "x8sNVfWqVFuQIxzfCZxk";

/**
 * \brief Entry point into the program
 */
int main() {
    PGconn *conn;
    PGresult *res;
    int rec_count;
    int row;
    int col;

    // yeah yeah credentials on github these are going to be deleted later
    // i'm not even at the 'it works on my machine' phase yet
    char connection_string [512];
    snprintf( connection_string, 512, "dbname=%s host=%s user=%s password=%s", database_name, database_host, database_user, database_pw );
    conn = PQconnectdb( connection_string );
    if (PQstatus(conn) == CONNECTION_BAD) {
        puts("fail");
        exit(0);
    }

    dropTables(conn);

    createTables(conn);

    PList::Dictionary* library = openLibrary( library_file );
    migrateLibrary( conn, library );

    /*// test caseSensitiveFilePath()
    puts( "/home/matt/hdd/dev/yamm/test_data/Beirut/The Rip Tide/01 A candle's Fire.mp3" );
    puts( caseSensitiveFilePath( "/home/matt/hdd/dev/yamm/test_data/Beirut/The Rip Tide/01 A candle's Fire.mp3" ).c_str() );
    //*/

    /*// test fractional inserts into playlists
    // SELECT * FROM tracks_playlists Link LEFT JOIN playlists Playlist ON Link.playlist_id = Playlist.id WHERE Playlist.title = 'testing fractional inserts' ORDER BY position;
    PGresult* playlist_insert_test_res = addPlaylistToDatabase( conn, "", "testing fractional inserts", 0);
    char* playlist_id = PQgetvalue( playlist_insert_test_res, 0, 0 );
    PGresult* track_insert_res = appendTrackToPlaylist( conn, "1", playlist_id );
    track_insert_res = appendTrackToPlaylist( conn, "2", playlist_id );
    track_insert_res = appendTrackToPlaylist( conn, "4", playlist_id );
    track_insert_res = addTrackToPlaylist( conn, "3", playlist_id, 2.5 );
    PQclear( playlist_insert_test_res );
    PQclear( track_insert_res );
    //*/

    /*// test track removal from playlists
    // SELECT * FROM tracks_playlists Link LEFT JOIN playlists Playlist ON Link.playlist_id = Playlist.id WHERE Playlist.title = 'testing track removal' ORDER BY position;
    PGresult* playlist_insert_test_res = addPlaylistToDatabase( conn, "", "testing track removal", 0);
    char* playlist_id = PQgetvalue( playlist_insert_test_res, 0, 0 );
    PGresult* track_insert_res = appendTrackToPlaylist( conn, "1", playlist_id );
    track_insert_res = appendTrackToPlaylist( conn, "2", playlist_id );
    track_insert_res = appendTrackToPlaylist( conn, "4", playlist_id );
    char* track_playlist_id = PQgetvalue( track_insert_res, 0, 0 );
    track_insert_res = removeTrackFromPlaylist( conn, track_playlist_id );
    track_insert_res = appendTrackToPlaylist( conn, "3", playlist_id );
    PQclear( playlist_insert_test_res );
    PQclear( track_insert_res );
    //*/

    puts( PQerrorMessage(conn) );

    return 0;
}

/**
 * \brief Drops all tables this appliciation uses
 * \returns 0
 */
int dropTables(PGconn* conn) {
    PQexec(conn, "DROP TABLE IF EXISTS tracks");
    PQexec(conn, "DROP TABLE IF EXISTS playlists");
    PQexec(conn, "DROP TABLE IF EXISTS tracks_playlists");
    return 0;
}

/**
 * \brief Creates all tables this application uses
 * \returns 0
 */
int createTables(PGconn* conn) {
    PQexec(conn,
            "create table tracks (\
                id SERIAL NOT NULL PRIMARY KEY,\
                itunes_id TEXT NULL,\
                location TEXT NOT NULL,\
                flag INTEGER NOT NULL DEFAULT 0\
            )");

    PQexec(conn,
            "create table playlists (\
                id SERIAL NOT NULL PRIMARY KEY,\
                itunes_id TEXT NULL,\
                title TEXT NOT NULL,\
                length INTEGER NOT NULL DEFAULT 0,\
                flag INTEGER NOT NULL DEFAULT 0\
            )");

    PQexec(conn,
            "create table tracks_playlists (\
                id SERIAL NOT NULL PRIMARY KEY,\
                track_id INTEGER NOT NULL,\
                playlist_id INTEGER NOT NULL,\
                position FLOAT NOT NULL\
            )");
    return 0;
}

/**
 * \brief Opens an iTunes Library plist file
 *
 * \param filepath Path to a iTunes Library.xml file
 *
 * \return libplist PList::Dictionary* representing the library
 */
PList::Dictionary* openLibrary(char* filepath) {
    // read xml file into string to create plist object with FromXml
    std::ifstream file( filepath ); // gotta try-catch yo
    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string buffer (length, ' ');
    file.read(&buffer[0], length);

    // initially reading the library
    PList::Dictionary* library = (PList::Dictionary*)PList::Structure::FromXml(buffer);

    return library;
}

/**
 * \brief Replaces the first occurence of a certain substring in a string
 *
 * \param str The string of which a substring will be replaced
 * \param from The substring to replace
 * \param The string that gets substituted in
 *
 * \return The substituted string, even though it takes a reference so that's redundant.
 */
std::string replaceSubstring( std::string& str, std::string from, std::string to ) {
    int index = str.find(from, 0);
    if (index == std::string::npos) {
        return str;
    }
    str.replace(index, from.length(), to);
    return str;
}

/**
 * \brief Adds tracks, playlists to database
 *
 * \param conn Pointer to the database connection object
 * \param library Pointer to a libplist Dictionary object representing an iTunes library
 *
 * \return 0
 */
int migrateLibrary( PGconn* conn, PList::Dictionary* library ) {
    CURL* curl = curl_easy_init();

    int length = 0;
    //char* old_library_path = curl_easy_unescape( curl, ((PList::String*)(*library)["Music Folder"])->GetValue().c_str(), 0, &length );
    std::string old_library_path = ((PList::String*)(*library)["Music Folder"])->GetValue();

    PList::Dictionary* tracks = (PList::Dictionary*) ( library->Find("Tracks")->second );
    PList::Array* playlists = (PList::Array*) ( library->Find("Playlists")->second );

    // add tracks to database table
    for( auto itr = tracks->Begin(); itr != tracks->End(); ++itr ) {
        std::string itunes_id = itr->first;
        PList::Dictionary* track = (PList::Dictionary*) itr->second;

        std::string encoded_location = ((PList::String*)((*track)["Location"]))->GetValue();
        replaceSubstring( encoded_location, old_library_path, new_library_path );

        // decode the URI so it can actually be identified on the disk
        char* filepath = curl_easy_unescape( curl, encoded_location.c_str(), 0, &length );

        // find case sensitive path; coming from NTFS to a case-sensitive filesystem will break things
        std::string case_sensitive_path = caseSensitiveFilePath( filepath );

        // add the track
        PGresult* res = addTrackToDatabase( conn, itunes_id, case_sensitive_path, 0 );
        PQclear( res );

        // curl wants me to
        curl_free( filepath );
    }

    // add playlists to database table
    // some playlists in itunes library won't be music; figure out how to ignore them
    // my test data is fine though so i'll proceed for now
    for (int i = 0; i < ((PList::Structure*)playlists)->GetSize(); ++i) {
        PList::Dictionary* playlist = (PList::Dictionary*) (*playlists)[i];
        int playlist_itunes_id = ((PList::Integer*)((*playlist)["Playlist ID"]))->GetValue();
        std::string playlist_title = ((PList::String*)((*playlist)["Name"]))->GetValue();
        PGresult* res = addPlaylistToDatabase( conn, std::to_string(playlist_itunes_id), playlist_title, 0 );
        char* playlist_internal_id = PQgetvalue( res, 0, 0 ); // only one record with one column should be in here

        PList::Array* playlist_items = (PList::Array*) (*playlist)["Playlist Items"];
        // iterate through tracks in the playlist
        for (int i = 0; i < ((PList::Structure*)playlist_items)->GetSize(); ++i) {
            PList::Dictionary* track = (PList::Dictionary*) (*playlist_items)[i];
            int track_itunes_id = ((PList::Integer*)(*track)["Track ID"])->GetValue();
            std::string query = "SELECT DISTINCT id FROM tracks WHERE itunes_id = '";
            query.append( std::to_string( track_itunes_id ) );
            query.append( "'" );
            PGresult* track_res = PQexec( conn, query.c_str() );
            char* track_internal_id = PQgetvalue( track_res, 0, 0 );
            PGresult* playlist_track_res = appendTrackToPlaylist( conn, std::string(track_internal_id), std::string(playlist_internal_id) );

            PQclear(track_res);
            PQclear(playlist_track_res);
        }

        PQclear( res );
    }
    curl_easy_cleanup( curl );
    return 0;
}

std::string caseSensitiveFolderChild( std::string folder, std::string target) {
    DIR* dir = opendir( folder.c_str() );
    struct dirent* directory_on_disk = readdir( dir );

    std::string ret = "";

    for (; directory_on_disk != NULL; directory_on_disk = readdir( dir )) {
        char* lowercase_on_disk = new char[ 256 ]; // size limit of directory name in dirent struct
        char* lowercase_target = new char[ target.length() + 1]; // leave room for null termination

        for (int i = 0; i < 256; ++i) {
            lowercase_on_disk[i] = tolower( directory_on_disk->d_name[i] );
        }
        for (int i = 0; i < target.length(); ++i) {
            lowercase_target[i] = tolower( target[i] );
        }
        lowercase_target[ target.length() ] = '\0'; // make sure null terminated

        if (strcmp( lowercase_on_disk, lowercase_target ) == 0) {
            delete[] lowercase_on_disk;
            delete[] lowercase_target;
            ret = directory_on_disk->d_name;
            break;
        }
        delete[] lowercase_on_disk;
        delete[] lowercase_target;
    }
    closedir(dir);
    return ret;
}

/**
 * \brief Case-sensitizes a filepath
 *
 * \param path_to_test Case-insensitive filepath
 *
 * \return Case-sensitive filepath
 *
 * This function doesn't yet handle failure. iTunes often records
 * case-insensitive filepaths because it doesn't matter on NTFS
 * or default HFS+. To use the playlists on a case-sensitive filesystem,
 * accommodations must be made.
 */
std::string caseSensitiveFilePath( std::string path_to_test ) {
    std::string uncertain_path = path_to_test.substr( new_library_path.length() );
    std::string certain_path = new_library_path;

    // No such thing as a 0-character directory name; index 0 won't be / so we can start at index 1
    for (int i = 1; i < uncertain_path.length(); ++i) {
        if (uncertain_path[i] == '/' && uncertain_path[ i-1 ] != '\\') {
            certain_path.append( caseSensitiveFolderChild( certain_path, uncertain_path.substr(0, i) ) );
            certain_path.append("/");
            uncertain_path = uncertain_path.substr(i + 1);
        }
    }
    certain_path.append( caseSensitiveFolderChild( certain_path, uncertain_path ) );
    return certain_path;
}
