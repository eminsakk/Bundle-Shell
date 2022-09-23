#ifndef PROCESSBUNDLE
#define PROCESSBUNDLE
#include <unistd.h>
#include <vector>
#include <string.h>
#include <iostream>

using namespace std;

class processBundle
{
private:
    //Bundle name
    string bundleName;

    // Process' info
    vector<string> commandsToExecute;
    vector<vector<char *>> commandsArgs;

    //Predecessor and successor of a processBundle.
    processBundle * pred;
    processBundle * succ;

    // It is for bundle that there exists a inp file to bundle or out file. 
    // If not, strings are empty.
    string inp_txt;
    string out_txt;

    //Number of process in the bundle.
    unsigned short int bundleSize;

public:
    processBundle(string);
    ~processBundle();
    string getName();
    void insertCommand(string command,vector<string> args);
    void setInputPath(string in);
    void setOutputPath(string out);
    bool isInPiped();
    bool isOutPiped();
    int numberOfProcess();
    string getInPath();
    string getOutPath();
    vector<string> getCommands();
    vector<vector<char *>> getArgs();
    void setPredSucc(processBundle * pred, processBundle * succ);
    void clearPredSucc();
    processBundle * getSucc();
    processBundle * getPred();
};

#endif