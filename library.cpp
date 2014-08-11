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
#include "library.h"

Library::Library(std::string library_file) {
    // read xml file into string to create plist object with FromXml
    std::ifstream file(library_file); // gotta try-catch yo
    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string buffer (length, ' ');
    file.read(&buffer[0], length);

    // initially reading the library
    _library = (PList::Dictionary*)PList::Structure::FromXml(buffer);

    // naming the track/playlist collectinos
    _tracks = (PList::Dictionary*) (_library->Find("Tracks")->second );
    _playlists = (PList::Array*) ( _library->Find("Playlists")->second );
}

bool Library::NodesAreEqual(PList::Node* a, PList::Node* b) {
    plist_type type = a->GetType();
    if (type != b->GetType()) {
        return false;
    }
    switch (type) {
        case 0:
            return ((PList::Boolean*)a)->GetValue() == ((PList::Boolean*)b)->GetValue();
        case 1:
            return ((PList::Integer*)a)->GetValue() == ((PList::Integer*)b)->GetValue();
        case 2:
            return ((PList::Real*)a)->GetValue() == ((PList::Real*)b)->GetValue();
        case 3:
            return ((PList::String*)a)->GetValue() == ((PList::String*)b)->GetValue();
        case 4: {
            int size = ((PList::Structure*)a)->GetSize();
            if (size != ((PList::Structure*)b)->GetSize()) {
                return false;
            }
            for (int i = 0; i < size; i++) {
                if ( ! NodesAreEqual( (*((PList::Array*)a))[i], (*((PList::Array*)b))[i] ) ) {
                    return false;
                }
            }
            return true;
                }
        case 5:
            return std::equal(
                    ((PList::Dictionary*)a)->Begin(),
                    ((PList::Dictionary*)a)->End(),
                    ((PList::Dictionary*)b)->Begin(),
                    [] (std::pair<std::string, PList::Node*> p1, std::pair<std::string, PList::Node*> p2) { return p1.first == p1.first && NodesAreEqual( p1.second, p2.second); }
                    );
        case 6:
            return ((PList::Date*)a)->GetValue().tv_sec == ((PList::Date*)b)->GetValue().tv_sec;
        case 7:
            return ((PList::Data*)a)->GetValue() == ((PList::Data*)b)->GetValue();
        case 8:
            return ((PList::Key*)a)->GetValue() == ((PList::Key*)b)->GetValue();
        case 9:
            return ((PList::Uid*)a)->GetValue() == ((PList::Uid*)b)->GetValue();
    }
}

// Array::GetNodeIndex() matches index of an equal pointer and not a pointer to an equivalent node
int Library::FindNodeInArray(PList::Array* a, PList::Node* node) {
    int i;
    for (i = 0; i < ((PList::Structure*)a)->GetSize(); i++) {
        if (NodesAreEqual( (*a)[i], node )) {
            return i;
        }
    }
    return i;
}

// modifies array in place! make sure that's what you want before using!
void Library::ArrayRemoveDuplicates(PList::Array* array) {
    for (int i = 0; i < ((PList::Structure*)array)->GetSize(); i++) {
        for (int j = i+1; j < ((PList::Structure*)array)->GetSize(); j++) {
            if ( NodesAreEqual( (*array)[i], (*array)[j] ) ) {
                array->Remove(j);
                j++;
            }
        }
    }
}

// a naive approach to set union, since these are unlikely to be huge
// revisit if performance is ever an issue!
PList::Array* Library::PListArraySetUnion(PList::Array* a, PList::Array* b) {
    PList::Array* ret = new PList::Array (*a);
    for (int i = 0; i < ((PList::Structure*)b)->GetSize(); i++) {
        int index = FindNodeInArray( ret, (*b)[i] );
        if ( index == ((PList::Structure*)ret)->GetSize() ) {
            ret->Append( (*b)[i] );
        }
    }
    ArrayRemoveDuplicates( ret );
    return ret;
}

// a naive approach to set intersection
PList::Array* Library::PListArraySetIntersection(PList::Array* a, PList::Array* b) {
    PList::Array* ret = new PList::Array (*a);
    for (int i = 0; i < ((PList::Structure*)b)->GetSize(); i++) {
        int index = FindNodeInArray( ret, (*b)[i] );
        if ( index == ((PList::Structure*)ret)->GetSize() ) {
            ret->Remove( (*b)[i] );
        }
    }
    ArrayRemoveDuplicates( ret );
    return ret;
}

// a naive approach to set difference
PList::Array* Library::PListArraySetDifference(PList::Array* a, PList::Array* b) {
    PList::Array* ret = new PList::Array (*a);
    for (int i = 0; i < ((PList::Structure*)b)->GetSize(); i++) {
        int index = FindNodeInArray( ret, (*b)[i] );
        if ( index != ((PList::Structure*)ret)->GetSize() ) {
            ret->Remove( index );
        }
    }
    ArrayRemoveDuplicates( ret );
    return ret;
}

std::string Library::TypeIntToName(int type) {
    switch (type) {
        case 0:
            return "PLIST_BOOLEAN";
        case 1:
            return "PLIST_UINT";
        case 2:
            return "PLIST_REAL";
        case 3:
            return "PLIST_STRING";
        case 4:
            return "PLIST_ARRAY";
        case 5:
            return "PLIST_DICT";
        case 6:
            return "PLIST_DATE";
        case 7:
            return "PLIST_DATA";
        case 8:
            return "PLIST_KEY";
        case 9:
            return "PLIST_UID";
        default:
            return "PLIST_NONE";
    }
}

std::string Library::indent(int indent_level) {
    std::string str = "";
    for (int i = 0; i < indent_level; i++) {
        str += "    ";
    }
    return str;
}

std::string Library::NodeToString(PList::Node* node, int depth, int indent_level) {
    switch (node->GetType()) {
        case 0: // PList::Boolean
            if (((PList::Boolean*)node)->GetValue())
                return indent(indent_level) + "true";
            return indent(indent_level) + "false";
        case 1: // PList::Integer
            return indent(indent_level) + std::to_string( ((PList::Integer*)node)->GetValue() );
        case 2: // PLIST::Real
            return indent(indent_level) + std::to_string( ((PList::Real*)node)->GetValue() );
        case 3: // PLIST::String
            return indent(indent_level) + ((PList::String*)node)->GetValue();
        case 4: { // PList::Array
            if (depth == 0) {
                return indent(indent_level) + "<recursive depth hit>";
            }
            int size = ((PList::Structure*)node)->GetSize();
            std::string str = "==START ARRAY==\n";
            // currently this ignores indentation level
            for (int i = 0; i < size; i++) {
                str += NodeToString((*(PList::Array*)node)[i], depth - 1, indent_level + 1);
                str += "\n";
            }
            str += indent(indent_level) + "==END ARRAY==";
            return indent(indent_level) + str;
                }
        case 5: { // PList::Dictionary
            if (depth == 0) {
                return indent(indent_level) + "<recursive depth hit>";
            }
            std::string str = "==START DICTIONARY==\n";
            for (auto itr = ((PList::Dictionary*)node)->Begin(); itr != ((PList::Dictionary*)node)->End(); ++itr) {
                str += indent(indent_level) + itr->first;
                str += " => ";
                if (itr->second->GetType() == 4) {
                    str += "array:\n";
                    str += NodeToString(itr->second, depth - 1, indent_level + 1);
                    str += "\n";
                } else if (itr->second->GetType() == 5) {
                    str += "dictionary:\n";
                    str += NodeToString(itr->second, depth - 1, indent_level + 1);
                    str += "\n";
                } else {
                    str += NodeToString(itr->second, depth - 1, 0);
                    str += "\n";
                }
            }
            str += indent(indent_level) + "==END DICTIONARY==";
            return indent(indent_level) + str;
                }
        case 6: // PList::Date
            return indent(indent_level) + std::to_string((((PList::Date*)node)->GetValue()).tv_sec);
        case 7: { // PList::Data
            std::vector<char> value = ((PList::Data*)node)->GetValue();
            std::string str(value.begin(), value.end());
            return indent(indent_level) + str;
                }
        case 8: // PList::Key
            return indent(indent_level) + ((PList::String*)node)->GetValue();
        case 9: // PList::Uid
            return indent(indent_level) + std::to_string( ((PList::Uid*)node)->GetValue() );
        default:
            return indent(indent_level) + "<not a node>";
    }
}
