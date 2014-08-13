#include <postgresql/libpq-fe.h>
#include <string>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <curl/curl.h>
#include <sys/stat.h>
#include <plist/plist++.h>
#include "migration.h"

char* library_file = "/home/matt/hdd/dev/yamm/test_data/iTunes Library.xml";
std::string new_library_path = "/home/matt/hdd/dev/yamm/test_data/";

char* unicode = "大好きな東方の曲をメドレーにして弾いてみた【ピアノ】";
std::string uni = "大好きな東方の曲をメドレーにして弾いてみた【ピアノ】";

int main() {
    PGconn *conn;
    PGresult *res;
    int rec_count;
    int row;
    int col;

    conn = PQconnectdb("dbname=music host=localhost user=music password=x8sNVfWqVFuQIxzfCZxk");
    if (PQstatus(conn) == CONNECTION_BAD) {
        puts("fail");
        exit(0);
    }

    dropTables(conn);

    createTables(conn);

    PList::Dictionary* library = openLibrary( library_file );
    migrateLibrary( conn, library );

    puts( PQerrorMessage(conn) );
    puts( unicode );
    std::cout << uni << std::endl;

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

        // decode the URI so it can be run through stat and mean something
        char* filepath = curl_easy_unescape( curl, encoded_location.c_str(), 0, &length );

        // find case sensitive path; coming from NTFS to a case-sensitive filesystem will break things
        std::string case_sensitive_path = caseSensitivePath( filepath );

        // build the query
        std::string query = "INSERT INTO tracks (itunes_id, location) VALUES (";
        char* escaped_itunes_id = PQescapeLiteral( conn, itunes_id.c_str(), itunes_id.length() );
        char* escaped_case_sensitive_path = PQescapeLiteral( conn, case_sensitive_path.c_str(), case_sensitive_path.length() );
        query.append( escaped_itunes_id );
        query.append( ", " );
        query.append( escaped_case_sensitive_path );
        query.append( ")" );

        // add the track to the database
        PQexec( conn, query.c_str() );

        // print to make sure everything looks okay
        /*
        puts( "===========" );
        puts( query.c_str() );
        puts( PQerrorMessage( conn ) );
        puts( case_sensitive_path.c_str() );
        puts( "" );//*/
        // curl wants me to
        curl_free( filepath );
    }

    //curl_free( old_library_path );
    curl_easy_cleanup( curl );
    return 0;
}

std::string caseSensitivePath( std::string old_path ) {
    return old_path; // this is temporary, i just wanna see how far i get
}
