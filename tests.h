/**
 * \file tests.h
 *
 * \author Matt Hammerly
 *
 * \brief This file includes declarations of functions written to test yamm components
 */

#ifndef TESTS_H
#define TESTS_H

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

void test_caseSensitiveFilePath();

void test_inserts();

void test_removal();

#endif