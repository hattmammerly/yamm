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
#include "migration.h"

// this stuff will be in some config file in the future along with db credentials
char* library_file = "/home/matt/hdd/dev/yamm/test_data/iTunes Library.xml";
std::string new_library_path = "/home/matt/hdd/dev/yamm/test_data/";

int main() {
    PGconn *conn;
    PGresult *res;
    int rec_count;
    int row;
    int col;

    // yeah yeah credentials on github these are going to be deleted later
    // i'm not even at the 'it works on my machine' phase yet
    conn = PQconnectdb("dbname=music host=localhost user=music password=x8sNVfWqVFuQIxzfCZxk");
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

    puts( PQerrorMessage(conn) );

    return 0;
}

int dropTables(PGconn* conn) {
    PQexec(conn, "DROP TABLE IF EXISTS tracks");
    PQexec(conn, "DROP TABLE IF EXISTS playlists");
    PQexec(conn, "DROP TABLE IF EXISTS tracks_playlists");
    return 0;
}

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
                position INTEGER NOT NULL\
            )");
    return 0;
}

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

std::string replaceSubstring( std::string& str, std::string from, std::string to ) {
    int index = str.find(from, 0);
    if (index == std::string::npos) {
        return str;
    }
    str.replace(index, from.length(), to);
    return str;
}

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
        addTrackToDatabase( conn, itunes_id, case_sensitive_path, 0 );

        // curl wants me to
        curl_free( filepath );
    }

    curl_easy_cleanup( curl );
    return 0;
}

// pulled into own function because it's probably going to be a function in common with other programs
int addTrackToDatabase( PGconn* conn, std::string itunes_id, std::string filepath, int flag) {
    // if i do anything with `flag` it will actually probably be calculated in here and not passed as a parameter
    std::string query = "INSERT INTO tracks (itunes_id, location, flag) VALUES (";
    char* escaped_itunes_id = PQescapeLiteral( conn, itunes_id.c_str(), itunes_id.length() );
    char* escaped_filepath = PQescapeLiteral( conn, filepath.c_str(), filepath.length() );
    query.append( escaped_itunes_id );
    query.append( "," );
    query.append( escaped_filepath );
    query.append( "," );
    query.append( std::to_string(flag) );
    query.append( ")" );

    PQexec( conn, query.c_str() );
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
