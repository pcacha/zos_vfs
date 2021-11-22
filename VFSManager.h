#ifndef ZOS_VFS_VFSMANAGER_H
#define ZOS_VFS_VFSMANAGER_H

#include <string>
#include "Constants.h"
#include "VFSDefinitions.h"
#include <vector>

using namespace std;

/*
 * Class containing the logic of virtual file system
 */
class VFSManager {
private:
    // vfs name
    char *vfsName;
    // super block
    superBlock sb;
    // inodes bitmap
    char *inodesBitmap;
    // data cluster bitmap
    char *dataBitmap;
    // inodes
    inode *inodes;
    // current path
    char path[1000];
    // current inode idx
    int currentInode;
    // if vfs is already formatted
    bool formatted;
    // file
    FILE *fp;

    // format vfs
    void format(string size);
    // copy
    void cp(string source, string target);
    // move
    void mv(string source, string target);
    // remove
    void rm(string target);
    // make directory
    void mkdir(string target);
    // remove directory
    void rmdir(string target);
    // list items
    void ls(string target);
    // print text
    void cat(string target);
    // change directory
    void cd(string target);
    // print info about item
    void info(string target);
    // copy file to vfs
    void incp(string source, string target);
    // copy file from vfs
    void outcp(string source, string target);
    // execute file with commands
    void load(string target);
    // hard link
    void ln(string source, string target);
    // save bitmaps and array of inodes
    void saveMetadata();
    // get the size of bytes from user input
    int getBytesSize(string sizeString);
    // add reference to self and parent
    void addTraversalReference(int inodeIdx, int parentIdx);
    // add item to directory
    void addDirectoryItem(int dirInodeIdx, int targetInodeIdx, char *itemName);
    // save dir item to vfs
    void saveDirItem(int addressInClusters, directoryItem *item);
    // get index of first free data cluster
    int getFreeClusterIdx();
    // check if given path exists (starting at dir with passed index), if yes it returns dir inode index, if no it returns -1
    int checkPathExists(vector<string> path, int startInodeIdx);
    // get next free inode
    int getFreeInodeIdx();
    // checks if name of new item is unique in dir
    bool itemNameUnique(int dirInodeIdx, char *itemName);
    // get all directory items by reference
    void getAllDirectoryItems(directoryItem *items, int dirInodeIdx, int itemsCount);
    // parse parent path - returns value by parentInodeIdx -> -1 if path not exits or the index of inode of parent of target item
    void parseParentPath(string path, int * parentInodeIdx, char ** itemName);
    // parse path - returns value by targetInodeIdx -> -1 if path not exists or the index of inode of target item
    void parsePath(string path, int * targetInodeIdx);
    // get the inode idx of item with given name, if not exists, -1 is returned
    int getItemInodeIdxByName(int parentInodeIdx, char *itemName);
    // delete item by its inode idx from parent
    void deleteItemFromParentCluster(int parentInodeIdx, char *itemName);
    // add next data chunk of file to vfs
    void addDataChunk(int inodeIdx, char *buffer, int bytesRead);
    // save data chunk to vfs
    void saveDataChunk(int address, char *buffer, int bytes);
    // save reference to cluster to cluster
    void saveReferenceToCluster(int address, int *clusterIdx);
    // get reference to cluster from cluster
    int getReferenceFromCluster(int address);
    // get the data cluster indexes of given item
    vector<int> getDataClustersIdxs(int sourceInodeIdx, int clusterCount);
    // read chunk of data from vfs
    void readDataChunk(int dataClusterIdx, char *buffer, int bytesCount);
    // get index of data cluster based on index of data chunk
    int getDataClusterIdxByChunkIdx(int sourceInodeIdx, int chunkIdx);

public:
    // constructor
    explicit VFSManager(char *vfsName);
    // destructor
    ~VFSManager();
    // handles user command
    void handleCommand(string commandLine);
    // prints current directory
    void pwd();
};


#endif
