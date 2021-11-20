#ifndef ZOS_VFS_STRINGUTILS_H
#define ZOS_VFS_STRINGUTILS_H


#include <string>
#include <vector>

using namespace std;

/*
 * Class contains utility methods for strings
 */
class StringUtils {
public:
    // splits given string based on delimiter into list of strings
    static vector<string> split(const string &s, char delim);
private:
    template <typename Out>
    // help method for split
    static void split(const string &s, char delim, Out result);
};


#endif
