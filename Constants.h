#ifndef ZOS_VFS_CONSTANTS_H
#define ZOS_VFS_CONSTANTS_H

#include "string"

using namespace std;

/*
 * Class containing all constants
 */
class Constants {
public:
    // path end
    static const char PATH_END = '$';
    // command delimiter
    static const char COMMAND_DELIM = ' ';
    // path delimiter
    static const char PATH_DELIM = '/';
    // exit command
    static const string EXIT;
    // cp command
    static const string CP;
    // mv command
    static const string MV;
    // rm command
    static const string RM;
    // mkdir command
    static const string MKDIR;
    // rmdir command
    static const string RMDIR;
    // ls command
    static const string LS;
    // cat command
    static const string CAT;
    // cd command
    static const string CD;
    // pwd command
    static const string PWD;
    // info command
    static const string INFO;
    // incp command
    static const string INCP;
    // outcp command
    static const string OUTCP;
    // load command
    static const string LOAD;
    // format command
    static const string FORMAT;
    // ln command
    static const string LN;
    // unknown command msg
    static const string UNKNOWN_COMMAND_MSG;
    // vfs not formatted msg
    static const string NOT_FORMATTED_MSG;
    // command successful result
    static const string COMMAND_SUCCESS;
    // cannot create file message
    static const string CANNOT_CREATE_FILE;
    // path not found message
    static const string PATH_NOT_FOUND;
    // traversal reference to self
    static char *SELF_REF;
    // traversal reference to parent
    static char *PARENT_REF;
    // ratio of inodes bitmap
    constexpr static const double INODES_BITMAP_SIZE_RATIO = 0.001;
    // count of direct references
    static const int DIRECTS_COUNT = 5;
    // max name of item
    static const int ITEM_MAX_NAME_LEN = 12;
    // size of cluster [B]
    static const int CLUSTER_SIZE = 8192;
    // inode idx of root dir
    static const int ROOT_INODE_IDX = 0;
    // code for not existing inode
    static const int INODE_NOT_EXISTS_CODE = -1;
    // no more free data clusters message
    static const string FULL_CLUSTERS_MSG;
    // no more free inodes message
    static const string FULL_INODES_MSG;
    // dir is not empty msg
    static const string NOT_EMPTY;
    // exist msg
    static const string EXIST;
};


#endif
