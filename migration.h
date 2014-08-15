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
#include <dirent.h>
#include <sys/types.h>
#include <cctype>
#include <cstring>

int dropTables(PGconn*);

int createTables(PGconn*);

PList::Dictionary* openLibrary(char*);

int migrateLibrary( PGconn* conn, PList::Dictionary*);

int addTrackToDatabase( PGconn*, std::string, std::string, int);

std::string replaceSubstring( std::string&, std::string, std::string);

std::string caseSensitiveFolderChild( std::string, std::string );

std::string caseSensitiveFilePath( std::string );

#endif
