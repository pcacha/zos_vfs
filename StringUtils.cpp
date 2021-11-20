#include "StringUtils.h"

#include <string>
#include <sstream>
#include <vector>

using namespace std;

template <typename Out>
void StringUtils::split(const string &s, char delim, Out result) {
    istringstream iss(s);
    string item;
    while (getline(iss, item, delim)) {
        *result++ = item;
    }
}

vector <string> StringUtils::split(const string &s, char delim) {
    // result vector
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}
