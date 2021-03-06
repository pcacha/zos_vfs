#include "Constants.h"
#include "string"

using namespace std;

const string Constants::EXIT = "exit";
const string Constants::CP = "cp";
const string Constants::MV = "mv";
const string Constants::RM = "rm";
const string Constants::MKDIR = "mkdir";
const string Constants::RMDIR = "rmdir";
const string Constants::LS = "ls";
const string Constants::CAT = "cat";
const string Constants::CD = "cd";
const string Constants::PWD = "pwd";
const string Constants::INFO = "info";
const string Constants::INCP = "incp";
const string Constants::OUTCP = "outcp";
const string Constants::LOAD = "load";
const string Constants::FORMAT = "format";
const string Constants::LN = "ln";
const string Constants::UNKNOWN_COMMAND_MSG = "Unknown command detected";
const string Constants::NOT_FORMATTED_MSG = "The file system is not formatted";
const string Constants::COMMAND_SUCCESS = "OK";
const string Constants::CANNOT_CREATE_FILE = "CANNOT CREATE FILE";
const string Constants::FULL_CLUSTERS_MSG = "No more free data clusters!";
const string Constants::FULL_INODES_MSG = "No more free inodes!";
const string Constants::PATH_NOT_FOUND = "PATH NOT FOUND";
const string Constants::EXIST = "EXIST";
const string Constants::NOT_EMPTY = "NOT EMPTY";
const string Constants::FILE_NOT_FOUND = "FILE NOT FOUND";
const string Constants::FULL_REFERENCES_MSG = "No more references to data clusters - file is too big!";
char * Constants::SELF_REF = ".";
char * Constants::PARENT_REF = "..";