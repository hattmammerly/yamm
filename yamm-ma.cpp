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
#include "ma.h"
#include "db.h"

/**
 * \brief Entry point into the program
 */
int main() {
    PGconn *conn = connectToDatabase();

    dropTables(conn);

    createTables(conn);

    PList::Dictionary* library = openLibrary();
    migrateLibrary( conn, library );

    puts( PQerrorMessage(conn) );

    return 0;
}

