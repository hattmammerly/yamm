#ifndef YAMM_MA_H
#define YAMM_MA_H

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
#include "db.h"

int dropTables(PGconn*);

int createTables(PGconn*);

PList::Dictionary* openLibrary(char*);

int migrateLibrary( PGconn* conn, PList::Dictionary* );

std::string replaceSubstring( std::string&, std::string, std::string );

std::string caseSensitiveFolderChild( std::string, std::string );

std::string caseSensitiveFilePath( std::string );

#endif
