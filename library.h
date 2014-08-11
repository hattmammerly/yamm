#ifndef LIBRARY_H
#define LIBRARY_H

#include "/usr/include/plist/plist++.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>
#include <map>
#include <time.h>

class Library {
    public:
        Library(std::string library_file);
        PList::Dictionary* GetLibrary() { return _library; }
        PList::Array* GetPlaylists() { return _playlists; }
        PList::Dictionary* GetTracks() { return _tracks; }
        static bool NodesAreEqual(PList::Node*, PList::Node*);
        static std::string NodeToString(PList::Node*, int, int);
        static PList::Array* PListArraySetUnion(PList::Array*, PList::Array*);
        static PList::Array* PListArraySetIntersection(PList::Array*, PList::Array*);
        static PList::Array* PListArraySetDifference(PList::Array*, PList::Array*);
        static int FindNodeInArray(PList::Array*, PList::Node*);

    private:
        PList::Dictionary* _tracks;
        PList::Array* _playlists;
        PList::Dictionary* _library;
        std::string TypeIntToName(int type);

        Library();
        static std::string indent(int);
        static void ArrayRemoveDuplicates(PList::Array* array);
};

#endif
