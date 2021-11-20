#include <iostream>
#include "string"
#include "Constants.h"
#include "VFSManager.h"

using namespace std;

// entry point of program
int main(int argc, char *argv[]) {
    // check program arguments
    if(argc != 2) {
        cout << "Exit - bad arguments count" << endl;
        return EXIT_FAILURE;
    }

    VFSManager manager(argv[1]);

    // print root path
    cout << Constants::PATH_DELIM << Constants::PATH_END << " ";
    string command;
    getline(cin, command);

    // enable user to write command until exit is written
    while(command != Constants::EXIT) {
        // handle command
        manager.handleCommand(command);
        // enable another command
        manager.pwd();
        cout << Constants::PATH_END << " " << flush;
        getline(cin, command);
    }

    return EXIT_SUCCESS;
}
