#ifndef ZOS_VFS_VFSDEFINITIONS_H
#define ZOS_VFS_VFSDEFINITIONS_H

// empty keyword
const char EMPTY = 0;
// full keyword
const char FULL = 1;

/*
 * Struct represents super block of VFS
 */
typedef struct theSuperBlock {
    // overall size of VFS [B]
    int diskSize;
    // size of one cluster [B]
    int clusterSize;
    // count of clusters
    int clusterCount;
    // count of inodes
    int inodesCount;
    // address of start of inode bitmap
    int inodesBitmapAddress;
    // address of start of data clusters bitmap
    int dataClustersBitmapAddress;
    // address of start of inodes
    int inodesAddress;
    // address of start of data clusters
    int dataClustersAddress;
} superBlock;

/*
 * Struct represents inode of VFS
 */
typedef struct theInode {
    // if inode represents a directory
    bool isDirectory;
    // number of references pointing to this inode (used with hardlinks)
    int references;
    // size of item [B]
    int size;
    // direct references to data clusters
    int directs[5];
    // reference to data cluster with references to data clusters with actual data - first level indirect reference
    int indirect1;
    // reference to data cluster with references to data clusters with references to data clusters with actual data - second level indirect reference
    int indirect2;
} inode;

/*
 * Struct represents one item of a dctory
 */
typedef struct theDirectoryItem {
    // id of inode
    int inode;
    // name of directory item - 8 chars of name + 3 chars of postfix + string terminating char /0
    char name[12];
} directoryItem;


#endif