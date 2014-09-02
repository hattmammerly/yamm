/**
 * \file yamm-ma.h
 *
 * \author Matt Hammerly
 *
 * \brief Contains functions for the migration assistant application
 *
 * Functions in this file are specific to the library migration assistant
 * component of the application
 */

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

void replaceSubstring( std::string&, std::string, std::string );

std::string caseSensitiveFolderChild( std::string, std::string );

std::string caseSensitiveFilePath( std::string );

#endif
