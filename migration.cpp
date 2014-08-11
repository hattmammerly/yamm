#include <postgresql/libpq-fe.h>
#include <string>
#include <stdio.h>

int dropTables(PGconn* conn);
int createTables(PGconn* conn);

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
                location TEXT NOT NULL,\
                flag INTEGER NOT NULL DEFAULT 0\
            )");

    PQexec(conn,
            "create table playlists (\
                id SERIAL NOT NULL PRIMARY KEY,\
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
