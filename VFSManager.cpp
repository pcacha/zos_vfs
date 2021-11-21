#include "VFSManager.h"
#include "Constants.h"
#include "StringUtils.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <fstream>
#include <math.h>

using namespace std;

VFSManager::VFSManager(char *vfsName): sb() {
    this->vfsName = vfsName;
    path[0] = Constants::PATH_DELIM;
    path[1] = '\0';
    inodesBitmap = nullptr;
    dataBitmap = nullptr;
    inodes = nullptr;
    currentInode = 0;
    formatted = false;
    fp = NULL;

    // load vfs if exists
    fp = fopen(vfsName,"rb+");
    if(fp != NULL) {
        // read super block
        fread(&sb, sizeof(sb), 1, fp);

        // read inodes bitmap
        inodesBitmap = (char *) malloc(sb.inodesCount * sizeof(char));
        fread(inodesBitmap, sizeof(char), sb.inodesCount, fp);

        // read data clusters bitmap
        dataBitmap = (char *) malloc(sb.clusterCount * sizeof(char));
        fread(dataBitmap, sizeof(char), sb.clusterCount, fp);

        // read inodes
        inodes = (inode *) malloc(sb.inodesCount * sizeof(inode));
        fread(inodes, sizeof(inode), sb.inodesCount, fp);

        formatted = true;
    }
}

VFSManager::~VFSManager() {
    if(fp != NULL) {
        fclose(fp);
    }
}

void VFSManager::handleCommand(string commandLine) {
    // get the parts of command
    vector<string> parts = StringUtils::split(commandLine, Constants::COMMAND_DELIM);
    string command = parts[0];

    // execute command
    if(command != Constants::FORMAT && !formatted) {
        cout << Constants::NOT_FORMATTED_MSG << endl;
    }
    else if(command == Constants::CP) {
        cp(parts[1], parts[2]);
    }
    else if(command == Constants::MV) {
        mv(parts[1], parts[2]);
    }
    else if(command == Constants::RM) {
        rm(parts[1]);
    }
    else if(command == Constants::MKDIR) {
        mkdir(parts[1]);
    }
    else if(command == Constants::RMDIR) {
        rmdir(parts[1]);
    }
    else if(command == Constants::LS) {
        if(parts.size() == 2) {
            ls(parts[1]);
        }
        else {
            ls("");
        }
    }
    else if(command == Constants::CAT) {
        cat(parts[1]);
    }
    else if(command == Constants::CD) {
        cd(parts[1]);
    }
    else if(command == Constants::PWD) {
        pwd();
        cout << endl;
    }
    else if(command == Constants::INFO) {
        info(parts[1]);
    }
    else if(command == Constants::INCP) {
        incp(parts[1], parts[2]);
    }
    else if(command == Constants::OUTCP) {
        outcp(parts[1], parts[2]);
    }
    else if(command == Constants::LOAD) {
        load(parts[1]);
    }
    else if(command == Constants::FORMAT) {
        format(parts[1]);
    }
    else if(command == Constants::LN) {
        ln(parts[1], parts[2]);
    }
    else {
        cout << Constants::UNKNOWN_COMMAND_MSG << endl;
    }
}

void VFSManager::pwd() {
    cout << path;
}

void VFSManager::format(string size) {
    // set new state
    path[0] = Constants::PATH_DELIM;
    path[1] = '\0';
    currentInode = 0;
    formatted = false;

    // get size in bytes
    int bytesSize = getBytesSize(size);

    // set super block
    sb.diskSize = bytesSize;
    sb.clusterSize = Constants::CLUSTER_SIZE;
    sb.inodesCount = (bytesSize * Constants::INODES_BITMAP_SIZE_RATIO) / 1;
    sb.clusterCount = (bytesSize - sizeof(superBlock) - sizeof(char) * sb.inodesCount - sizeof(inode) * sb.inodesCount) / (Constants::CLUSTER_SIZE + 1);

    // set addresses
    sb.inodesBitmapAddress = sizeof(superBlock);
    sb.dataClustersBitmapAddress = sb.inodesBitmapAddress + sizeof(char) * sb.inodesCount;
    sb.inodesAddress = sb.dataClustersBitmapAddress + sizeof(char) * sb.clusterCount;
    sb.dataClustersAddress = sb.inodesAddress + sizeof(inode) * sb.inodesCount;

    // free vfs in memory
    if(inodesBitmap != NULL) {
        free(inodesBitmap);
    }
    if(dataBitmap != NULL) {
        free(dataBitmap);
    }
    if(inodes != NULL) {
        free(inodes);
    }

    // allocate space and init
    inodesBitmap = (char *) malloc(sb.inodesCount * sizeof(char));
    memset(inodesBitmap, EMPTY, sb.inodesCount);
    dataBitmap = (char *) malloc(sb.clusterCount * sizeof(char));
    memset(dataBitmap, EMPTY, sb.clusterCount);
    inodes = (inode *) malloc(sb.inodesCount * sizeof(inode));

    // save vfs on hard drive
    if(fp != NULL) {
        fclose(fp);
    }
    fp = fopen(vfsName, "wb+");
    if (fp == NULL)
    {
        // cannot create file
        cout << Constants::CANNOT_CREATE_FILE << endl;
        return;
    }
    fwrite(&sb, sizeof(sb), 1, fp);
    fwrite(inodesBitmap, sizeof(char), sb.inodesCount, fp);
    fwrite(dataBitmap, sizeof(char), sb.clusterCount, fp);
    fwrite(inodes, sizeof(inode), sb.inodesCount, fp);
    int bytesLeft = bytesSize - sizeof(superBlock) - sizeof(char) * sb.inodesCount - sizeof(char) * sb.clusterCount - sizeof(inode) * sb.inodesCount;
    char *dataPlaceholder = (char *) malloc(bytesLeft * sizeof(char));
    memset(dataPlaceholder, 0, bytesLeft);
    fwrite(dataPlaceholder, sizeof(char), bytesLeft, fp);
    free(dataPlaceholder);
    fflush(fp);

    // set initial state - root dir
    inodesBitmap[0] = FULL;
    inodes[0].isDirectory = true;
    inodes[0].references = 1;
    inodes[0].size = 0;
    addTraversalReference(0, 0);
    saveMetadata();

    // now vfs is formatted
    formatted = true;
    cout << Constants::COMMAND_SUCCESS << endl;
}

void VFSManager::cp(string source, string target) {

}

void VFSManager::mv(string source, string target) {

}

void VFSManager::rm(string target) {

}

void VFSManager::mkdir(string target) {
    int parentInodeIdx;
    char *targetName;
    // parse path
    parseParentPath(target, &parentInodeIdx, &targetName);

    // inode not exist - path not found
    if(parentInodeIdx == Constants::INODE_NOT_EXISTS_CODE) {
        cout << Constants::PATH_NOT_FOUND << endl;
        return;
    }

    // check if name is unique
    if(!itemNameUnique(parentInodeIdx, targetName)) {
        cout << Constants::EXIST << endl;
        return;
    }

    // create inode, mark it in inode map and init it
    int newInodeIdx = getFreeInodeIdx();
    inodesBitmap[newInodeIdx] = FULL;
    inodes[newInodeIdx].isDirectory = true;
    inodes[newInodeIdx].references = 0;
    inodes[newInodeIdx].size = 0;

    // add it to parent
    addDirectoryItem(parentInodeIdx, newInodeIdx, targetName);
    free(targetName);

    // add parent .. and current dir .
    addTraversalReference(newInodeIdx, parentInodeIdx);
    saveMetadata();
    cout << Constants::COMMAND_SUCCESS << endl;
}

void VFSManager::rmdir(string target) {
    int parentInodeIdx;
    char *targetName;
    // parse path
    parseParentPath(target, &parentInodeIdx, &targetName);

    // inode not exist or user wants to delete hidden dirs - path not found
    if(parentInodeIdx == Constants::INODE_NOT_EXISTS_CODE || strcmp(targetName, ".") == 0 || strcmp(targetName, "..") == 0) {
        cout << Constants::PATH_NOT_FOUND << endl;
        return;
    }

    // get inode idx of target dir
    int deleteDirInodeIdx = getItemInodeIdxByName(parentInodeIdx, targetName);
    // check if it was found and if it is dir
    if(deleteDirInodeIdx == Constants::INODE_NOT_EXISTS_CODE || !inodes[deleteDirInodeIdx].isDirectory) {
        cout << Constants::PATH_NOT_FOUND << endl;
        return;
    }
    // check if dir is empty
    if((inodes[deleteDirInodeIdx].size / sizeof(directoryItem)) > 2) {
        cout << Constants::NOT_EMPTY << endl;
        return;
    }

    // delete folder from parent folder
    deleteItemFromParentCluster(parentInodeIdx, targetName);

    // remove from bitmaps
    dataBitmap[inodes[deleteDirInodeIdx].directs[0]] = EMPTY;
    inodesBitmap[deleteDirInodeIdx] = EMPTY;

    saveMetadata();
    cout << Constants::COMMAND_SUCCESS << endl;
}

void VFSManager::ls(string target) {
    int lsDirInodeIdx;
    if(target.empty()) {
        // no path defined
        lsDirInodeIdx = currentInode;
    }
    else {
        // parse path
        parsePath(target, &lsDirInodeIdx);

        // inode not exist - path not found
        if(lsDirInodeIdx == Constants::INODE_NOT_EXISTS_CODE || !inodes[lsDirInodeIdx].isDirectory) {
            cout << Constants::PATH_NOT_FOUND << endl;
            return;
        }
    }

    // get items of wanted dir
    int itemsCount = inodes[lsDirInodeIdx].size / sizeof(directoryItem);
    directoryItem *items = (directoryItem *) malloc(itemsCount * sizeof(directoryItem));
    getAllDirectoryItems(items, lsDirInodeIdx, itemsCount);

    // iterate items and print them
    for(int i = 0; i < itemsCount; i++) {
        if(inodes[items[i].inode].isDirectory) {
            // for directories
            cout << "+" << items[i].name << endl;
        }
        else {
            // for files
            cout << "-" << items[i].name << endl;
        }
    }
    free(items);
}

void VFSManager::cat(string target) {

}

void VFSManager::cd(string target) {
    // update current inode
    int cdDirInodeIdx;
    if(target.empty()) {
        // no path defined - jump to root
        currentInode = Constants::ROOT_INODE_IDX;
    }
    else {
        // parse path
        parsePath(target, &cdDirInodeIdx);

        // inode not exist of it is not dir- path not found
        if(cdDirInodeIdx == Constants::INODE_NOT_EXISTS_CODE || !inodes[cdDirInodeIdx].isDirectory) {
            cout << Constants::PATH_NOT_FOUND << endl;
            return;
        }
        currentInode = cdDirInodeIdx;
    }

    // update current working directory
    if(target[0] == '/') {
        // path is absolute
        memset(path, 0, 1000);
        strcpy(path, target.c_str());
    }
    else {
        // path is relative
        string stringPath(path);
        vector<string> pathParts;
        if(stringPath != "/") {
            stringPath = stringPath.substr(1, stringPath.size() - 1);
            pathParts = StringUtils::split(stringPath, Constants::PATH_DELIM);
        }

        // iterate through user written relative path
        vector<string> userPath =  StringUtils::split(target, Constants::PATH_DELIM);
        for(int i = 0; i < userPath.size(); i++) {
            string current = userPath[i];
            if(current == ".") {
                // current dir
                continue;
            }
            if(current == "..") {
                // parent dir
                if(!pathParts.empty()) {
                    pathParts.erase(pathParts.begin() + pathParts.size() - 1);
                }
                continue;
            }
            pathParts.push_back(current);
        }

        memset(path, 0, 1000);
        if(pathParts.empty()) {
            // if parts are empty - path is root
            strcpy(path, "/");
        }
        else {
            // join parts to full path
            stringPath = "/" + pathParts[0];
            for(int i = 1; i < pathParts.size(); i++) {
                stringPath += "/" + pathParts[i];
            }
            strcpy(path, stringPath.c_str());
        }
    }
    cout << Constants::COMMAND_SUCCESS << endl;
}

void VFSManager::info(string target) {

}

void VFSManager::incp(string source, string target) {
    int parentInodeIdx;
    char *targetName;
    // parse path
    parseParentPath(target, &parentInodeIdx, &targetName);

    // inode not exist - path not found
    if(parentInodeIdx == Constants::INODE_NOT_EXISTS_CODE) {
        cout << Constants::PATH_NOT_FOUND << endl;
        return;
    }

    // check if name is unique
    if(!itemNameUnique(parentInodeIdx, targetName)) {
        cout << Constants::EXIST << endl;
        return;
    }

    // transform name to char pointer
    char *sourcePath = (char *) malloc((source.length() + 1) * sizeof(char));
    strcpy(sourcePath, source.c_str());
    // open target file
    FILE *sourceFile = fopen(sourcePath, "rb");
    // check if file exists
    if(sourceFile == NULL) {
        cout << Constants::FILE_NOT_FOUND << endl;
        return;
    }

    // create inode, mark it in inode map and init it
    int newInodeIdx = getFreeInodeIdx();
    inodesBitmap[newInodeIdx] = FULL;
    inodes[newInodeIdx].isDirectory = false;
    inodes[newInodeIdx].references = 1;
    inodes[newInodeIdx].size = 0;

    // load file data and write it to wfs
    char *buffer = (char *) malloc(sb.clusterSize * sizeof(char));
    memset(buffer, 0, sb.clusterSize);
    int bytesRead;
    while((bytesRead = fread(buffer, sizeof(char), sb.clusterSize, sourceFile)) > 0) {
        addDataChunk(newInodeIdx, buffer, bytesRead);
        memset(buffer, 0, sb.clusterSize);
    }

    // free sources
    free(buffer);
    fclose(sourceFile);

    // add it to parent
    addDirectoryItem(parentInodeIdx, newInodeIdx, targetName);
    free(targetName);

    saveMetadata();
    cout << Constants::COMMAND_SUCCESS << endl;
}

void VFSManager::outcp(string source, string target) {

}

void VFSManager::load(string target) {
    string command;
    ifstream commandFile(target.c_str(), ios::in);

    // check if it was successfully opened
    if(!commandFile) {
        cout << Constants::FILE_NOT_FOUND << endl;
        return;
    }

    // process all lines
    while (getline(commandFile, command))
    {
        pwd();
        cout << Constants::PATH_END << Constants::COMMAND_DELIM << command << endl;
        handleCommand(command);
    }

    cout << Constants::COMMAND_SUCCESS << endl;
}

void VFSManager::ln(string source, string target) {
    int sourceInodeIdx;

    // parse source path
    parsePath(source, &sourceInodeIdx);

    // inode not exists or it is a directory - source path not found
    if(sourceInodeIdx == Constants::INODE_NOT_EXISTS_CODE || inodes[sourceInodeIdx].isDirectory) {
        cout << Constants::PATH_NOT_FOUND << endl;
        return;
    }

    int parentInodeIdx;
    char *targetName;
    // parse target path
    parseParentPath(target, &parentInodeIdx, &targetName);

    // target parent inode not exist - path not found
    if(parentInodeIdx == Constants::INODE_NOT_EXISTS_CODE) {
        cout << Constants::PATH_NOT_FOUND << endl;
        return;
    }

    // check if name is unique
    if(!itemNameUnique(parentInodeIdx, targetName)) {
        cout << Constants::EXIST << endl;
        return;
    }

    // add hardlink to parent dir
    addDirectoryItem(parentInodeIdx, sourceInodeIdx, targetName);
    // increment hardlink references
    inodes[sourceInodeIdx].references += 1;

    saveMetadata();
    cout << Constants::COMMAND_SUCCESS << endl;
}

void VFSManager::saveMetadata() {
    // save metadata
    fseek(fp, sb.inodesBitmapAddress, SEEK_SET);
    fwrite(inodesBitmap, sizeof(char), sb.inodesCount, fp);
    fwrite(dataBitmap, sizeof(char), sb.clusterCount, fp);
    fwrite(inodes, sizeof(inode), sb.inodesCount, fp);
    fflush(fp);
}

int VFSManager::getBytesSize(string sizeString) {
    if(sizeString[sizeString.length() - 2]  == 'K' || sizeString[sizeString.length() - 2]  == 'k' ||
            sizeString[sizeString.length() - 2]  == 'M' || sizeString[sizeString.length() - 2]  == 'G') {
        // unit is KB/MB/GB
        // get unit and value passed
        string unit = sizeString.substr(sizeString.length() - 2, 2);
        int value = stoi(sizeString.substr(0, sizeString.length() - 2));

        if(unit == "kB" || unit == "KB") {
            return value * 1000;
        }
        else if(unit == "GB") {
            return value * 1000000000;
        }
        else {
            return value * 1000000;
        }
    }
    else {
        // unit is Byte
        return stoi(sizeString.substr(0, sizeString.length() - 1));
    }
}

void VFSManager::addTraversalReference(int inodeIdx, int parentIdx) {
    // self reference
    addDirectoryItem(inodeIdx, inodeIdx, Constants::SELF_REF);
    // parent reference
    addDirectoryItem(inodeIdx, parentIdx, Constants::PARENT_REF);
}

void VFSManager::addDirectoryItem(int dirInodeIdx, int targetInodeIdx, char *itemName) {
    // init item
    directoryItem item;
    item.inode = targetInodeIdx;
    memset(&item.name, 0, Constants::ITEM_MAX_NAME_LEN);
    strcpy(item.name, itemName);

    // item index in cluster
    int itemClusterIdx = inodes[dirInodeIdx].size / sizeof(directoryItem);

    if(itemClusterIdx == 0) {
        // first item in cluster - need to allocate cluster, insert item, mark it in bitmap and set direct address
        int freeClusterIdx = getFreeClusterIdx();
        saveDirItem(freeClusterIdx * sb.clusterSize, &item);
        dataBitmap[freeClusterIdx] = FULL;
        inodes[dirInodeIdx].directs[0] = freeClusterIdx;
    }
    else {
        // it is possible to insert item in already existing cluster
        saveDirItem(inodes[dirInodeIdx].directs[0] * sb.clusterSize + itemClusterIdx * sizeof(directoryItem), &item);
    }

    // increment size
    inodes[dirInodeIdx].size += sizeof(directoryItem);
}

void VFSManager::saveDirItem(int addressInClusters, directoryItem *item) {
    // set right address and save
    int address = sb.dataClustersAddress + addressInClusters;
    fseek(fp, address, SEEK_SET);
    fwrite(item, sizeof(directoryItem), 1, fp);
    fflush(fp);
}

int VFSManager::getFreeClusterIdx() {
    // iterate all clusters and try to find empty
    for(int i = 0; i < sb.clusterCount; i++) {
        if(dataBitmap[i] == EMPTY) {
            return i;
        }
    }

    // if no free cluster found exit app
    cout << Constants::FULL_CLUSTERS_MSG << endl;
    exit(EXIT_FAILURE);
}

int VFSManager::checkPathExists(vector<string> path, int startInodeIdx) {
    int currentInodeIdx = startInodeIdx;
    // iterate through path
    for(int i = 0; i < path.size(); i++) {
        // convert current dir name to char pointer
        char *itemName = (char *) malloc((path[i].length() + 1) * sizeof(char));
        strcpy(itemName, path[i].c_str());

        // get all items of dir
        int itemsCount = inodes[currentInodeIdx].size / sizeof(directoryItem);
        directoryItem *items = (directoryItem *) malloc(itemsCount * sizeof(directoryItem));
        getAllDirectoryItems(items, currentInodeIdx, itemsCount);

        // iterate items and check for the same names
        bool found = false;
        for(int j = 0; j < itemsCount; j++) {
            // check if directory name is the same
            if(strcmp(items[j].name, itemName) == 0) {
                found = true;
                currentInodeIdx = items[j].inode;
                break;
            }
        }
        free(items);
        free(itemName);

        if(!found) {
            return Constants::INODE_NOT_EXISTS_CODE;
        }
    }

    return currentInodeIdx;
}

int VFSManager::getFreeInodeIdx() {
    // iterate all inodes and try to find empty
    for(int i = 0; i < sb.inodesCount; i++) {
        if(inodesBitmap[i] == EMPTY) {
            return i;
        }
    }

    // if no free inodes found exit app
    cout << Constants::FULL_INODES_MSG << endl;
    exit(EXIT_FAILURE);
}

bool VFSManager::itemNameUnique(int dirInodeIdx, char *itemName) {
    // get items count and init them
    int itemsCount = inodes[dirInodeIdx].size / sizeof(directoryItem);
    directoryItem *items = (directoryItem *) malloc(itemsCount * sizeof(directoryItem));
    // load items
    getAllDirectoryItems(items, dirInodeIdx, itemsCount);

    // iterate items and check for the same names
    for(int i = 0; i < itemsCount; i++) {
        if(strcmp(items[i].name, itemName) == 0) {
            free(items);
            return false;
        }
    }
    free(items);

    return true;
}

void VFSManager::getAllDirectoryItems(directoryItem *items, int dirInodeIdx, int itemsCount) {
    // seek to the address and load items
    int itemsClusterAddress = sb.dataClustersAddress + inodes[dirInodeIdx].directs[0] * sb.clusterSize;
    fseek(fp, itemsClusterAddress, SEEK_SET);
    fread(items, sizeof(directoryItem), itemsCount, fp);
}

void VFSManager::parseParentPath(string path, int * parentInodeIdx, char ** itemName) {
    // check validity
    if(path.empty() || path == "/") {
        *parentInodeIdx = Constants::INODE_NOT_EXISTS_CODE;
        return;
    }
    // whether path is absolut
    bool absolutPath = false;

    if(path[0] == Constants::PATH_DELIM) {
        // if path is absolut, remove root dir and mark flag
        path = path.substr(1, path.length() - 1);
        absolutPath = true;
    }

    vector<string> fullPathParts = StringUtils::split(path, Constants::PATH_DELIM);
    if(fullPathParts.empty()) {
        *parentInodeIdx = Constants::INODE_NOT_EXISTS_CODE;
        return;
    }

    // set itemName to the last path part
    path = fullPathParts[fullPathParts.size() - 1];
    *itemName = (char *) malloc((path.length() + 1) * sizeof(char));
    strcpy(*itemName, path.c_str());

    if(fullPathParts.size() == 1) {
        if(absolutPath) {
            // creating dir in root
            *parentInodeIdx = Constants::ROOT_INODE_IDX;
        }
        else {
            // creating dir in current dir
            *parentInodeIdx = currentInode;
        }
    }
    else {
        // remove item name from the path
        fullPathParts.erase(fullPathParts.begin() + fullPathParts.size() - 1);

        if(absolutPath) {
            *parentInodeIdx = checkPathExists(fullPathParts, Constants::ROOT_INODE_IDX);
        }
        else {
            *parentInodeIdx = checkPathExists(fullPathParts, currentInode);
        }

        if(*parentInodeIdx != Constants::INODE_NOT_EXISTS_CODE && !inodes[*parentInodeIdx].isDirectory) {
            *parentInodeIdx = Constants::INODE_NOT_EXISTS_CODE;
        }
    }
}

void VFSManager::parsePath(string path, int * targetInodeIdx) {
    // check if it is current dir
    if(path.empty()) {
        *targetInodeIdx = currentInode;
        return;
    }

    // check if it is root dir
    if(path == "/") {
        *targetInodeIdx = Constants::ROOT_INODE_IDX;
        return;
    }

    // whether path is absolut
    bool absolutPath = false;

    if(path[0] == Constants::PATH_DELIM) {
        // if path is absolut, remove root dir and mark flag
        path = path.substr(1, path.length() - 1);
        absolutPath = true;
    }

    // split path to parts
    vector<string> fullPathParts = StringUtils::split(path, Constants::PATH_DELIM);
    if(fullPathParts.empty()) {
        *targetInodeIdx = Constants::INODE_NOT_EXISTS_CODE;
        return;
    }

    // check if path exist
    if(absolutPath) {
        *targetInodeIdx = checkPathExists(fullPathParts, Constants::ROOT_INODE_IDX);
    }
    else {
        *targetInodeIdx = checkPathExists(fullPathParts, currentInode);
    }
}

int VFSManager::getItemInodeIdxByName(int parentInodeIdx, char *itemName) {
    // get items count and init them
    int itemsCount = inodes[parentInodeIdx].size / sizeof(directoryItem);
    directoryItem *items = (directoryItem *) malloc(itemsCount * sizeof(directoryItem));
    // load items
    getAllDirectoryItems(items, parentInodeIdx, itemsCount);

    // iterate items and check for the same names, if names are equal, return inode idx
    for(int i = 0; i < itemsCount; i++) {
        if(strcmp(items[i].name, itemName) == 0) {
            int resultIdx = items[i].inode;
            free(items);
            return resultIdx;
        }
    }
    free(items);

    return Constants::INODE_NOT_EXISTS_CODE;
}

void VFSManager::deleteItemFromParentCluster(int parentInodeIdx, char *itemName) {
    // get all dir items
    int itemsCount = inodes[parentInodeIdx].size / sizeof(directoryItem);
    directoryItem *items = (directoryItem *) malloc(itemsCount * sizeof(directoryItem));
    // load items
    getAllDirectoryItems(items, parentInodeIdx, itemsCount);

    // init array for items without deleted
    directoryItem *itemsWithoutDeleted = (directoryItem *) malloc((itemsCount - 1) * sizeof(directoryItem));
    // fill array with items without deleted
    int j = 0;
    for(int i = 0; i < (itemsCount - 1); i++) {
        if(strcmp(items[j].name, itemName) == 0) {
            j++;
        }
        itemsWithoutDeleted[i] = items[j];
        j++;
    }
    // save directory items without deleted
    int address = sb.dataClustersAddress + inodes[parentInodeIdx].directs[0] * sb.clusterSize;
    fseek(fp, address, SEEK_SET);
    fwrite(itemsWithoutDeleted, sizeof(directoryItem), (itemsCount - 1) , fp);
    fflush(fp);

    inodes[parentInodeIdx].size -= sizeof(directoryItem);
    free(items);
    free(itemsWithoutDeleted);
}

void VFSManager::addDataChunk(int inodeIdx, char *buffer, int bytesRead) {
    // helpers
    int inodeChunksCount = ceil(inodes[inodeIdx].size / (double) sb.clusterSize);
    int intsPerCluster = sb.clusterSize / sizeof(int);

    if(inodeChunksCount < Constants::DIRECTS_COUNT) {
        // it is possible to store data chunk in directs
        // need to allocate cluster, insert data chunk, mark it in bitmap and set direct address
        int newClusterIdx = getFreeClusterIdx();
        saveDataChunk(newClusterIdx * sb.clusterSize, buffer, bytesRead);
        dataBitmap[newClusterIdx] = FULL;
        inodes[inodeIdx].directs[inodeChunksCount - 1] = newClusterIdx;
    }
    else if (inodeChunksCount < Constants::DIRECTS_COUNT + intsPerCluster)  {
        // first level indirect

        if(inodeChunksCount == Constants::DIRECTS_COUNT) {
            // setup first level indirect cluster
            int newClusterIdx = getFreeClusterIdx();
            dataBitmap[newClusterIdx] = FULL;
            inodes[inodeIdx].indirect1 = newClusterIdx;
        }

        // need to allocate cluster, insert data chunk, mark it in bitmap and set cluster id to indirect
        int newClusterIdx = getFreeClusterIdx();
        saveDataChunk(newClusterIdx * sb.clusterSize, buffer, bytesRead);
        dataBitmap[newClusterIdx] = FULL;
        saveReferenceToCluster(inodes[inodeIdx].indirect1 * sb.clusterSize + sizeof(int) * (inodeChunksCount - 1 - Constants::DIRECTS_COUNT), &newClusterIdx);
    }
    else if(inodeChunksCount < Constants::DIRECTS_COUNT + intsPerCluster + intsPerCluster * intsPerCluster) {

        if(inodeChunksCount == Constants::DIRECTS_COUNT + intsPerCluster) {
            // setup first level indirect cluster
            int newClusterIdx = getFreeClusterIdx();
            dataBitmap[newClusterIdx] = FULL;
            inodes[inodeIdx].indirect2 = newClusterIdx;
        }

        int idxInSecondLevel = inodeChunksCount - 1 - Constants::DIRECTS_COUNT - intsPerCluster;
        if(idxInSecondLevel % intsPerCluster == 0) {
            // setup second level indirect cluster
            int newClusterIdx = getFreeClusterIdx();
            dataBitmap[newClusterIdx] = FULL;
            saveReferenceToCluster(inodes[inodeIdx].indirect2 * sb.clusterSize + sizeof(int) * (idxInSecondLevel / intsPerCluster), &newClusterIdx);
        }

        // need to allocate cluster, insert data chunk, mark it in bitmap and set cluster id to indirect direct
        int newClusterIdx = getFreeClusterIdx();
        saveDataChunk(newClusterIdx * sb.clusterSize, buffer, bytesRead);
        dataBitmap[newClusterIdx] = FULL;
        saveReferenceToCluster( getReferenceFromCluster(inodes[inodeIdx].indirect2 * sb.clusterSize + sizeof(int) * (idxInSecondLevel / intsPerCluster))
            * sb.clusterSize + sizeof(int) * (idxInSecondLevel % intsPerCluster), &newClusterIdx);
    }
    else {
        cout << Constants::FULL_REFERENCES_MSG << endl;
        exit(EXIT_FAILURE);
    }

    // increment size
    inodes[inodeIdx].size += bytesRead;
}

void VFSManager::saveDataChunk(int address, char *buffer, int bytes) {
    // set right address and save
    address += sb.dataClustersAddress;
    fseek(fp, address, SEEK_SET);
    fwrite(buffer, sizeof(char), bytes, fp);
    fflush(fp);
}

void VFSManager::saveReferenceToCluster(int address, int *clusterIdx) {
    // set right address and save
    address += sb.dataClustersAddress;
    fseek(fp, address, SEEK_SET);
    fwrite(clusterIdx, sizeof(int), 1, fp);
    fflush(fp);
}

int VFSManager::getReferenceFromCluster(int address) {
    // set right address and save
    address += sb.dataClustersAddress;
    fseek(fp, address, SEEK_SET);
    int result;
    fread(&result, sizeof(int), 1, fp);
    fflush(fp);
    return result;
}