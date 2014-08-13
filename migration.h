#ifndef MIGRATION_H
#define MIGRATION_H

#include <postgresql/libpq-fe.h>
#include <string>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <curl/curl.h>
#include <sys/stat.h>
#include <plist/plist++.h>

int dropTables(PGconn* conn);

int createTables(PGconn* conn);

PList::Dictionary* openLibrary(char* filepath);

int migrateLibrary( PGconn* conn, PList::Dictionary* library );

std::string replaceSubstring( std::string& str, std::string from, std::string to );

std::string caseSensitivePath( std::string old_path );

#endif
