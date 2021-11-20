#include "VFSManager.h"
#include "Constants.h"
#include "StringUtils.h"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <iostream>

using namespace std;

VFSManager::VFSManager(char *vfsName): sb() {
    strcpy(this->vfsName, vfsName);
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
        ls(parts[1]);
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

}

void VFSManager::rmdir(string target) {

}

void VFSManager::ls(string target) {

}

void VFSManager::cat(string target) {

}

void VFSManager::cd(string path) {

}

void VFSManager::info(string target) {

}

void VFSManager::incp(string source, string target) {

}

void VFSManager::outcp(string source, string target) {

}

void VFSManager::load(string target) {

}

void VFSManager::ln(string source, string target) {

}

void VFSManager::saveMetadata() {
    // save metadata and close file
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
    fseek(fp, sb.dataClustersAddress + addressInClusters, SEEK_SET);
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