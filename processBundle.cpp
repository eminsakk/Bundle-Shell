#include "processBundle.h"

processBundle::processBundle(string name){ 
    bundleName = name; 
    inp_txt = ""; 
    out_txt = ""; 
    pred = NULL;
    succ = NULL;
}
processBundle::~processBundle(){
    for(int i = 0;i < commandsToExecute.size();i++)
        commandsToExecute[i].~basic_string();
    commandsToExecute.~vector();
    for(int i = 0; i < commandsArgs.size();i++){
        for(int j = 0; j < commandsArgs[i].size();j++)
            delete commandsArgs[i][j];
        commandsArgs[i].~vector();
    }
    commandsArgs.~vector();
}
string processBundle::getName() {return bundleName; }
void processBundle::insertCommand(string command, vector<string> args){
    vector<char * > toBePushed;
    char * cmd = new char[command.size()];
    commandsToExecute.push_back(command);
    strcpy(cmd,command.c_str());
    toBePushed.push_back(cmd);
    
    for(int i = 0;i < args.size();i++){
        char * tmp = new char[args[i].size()];
        strcpy(tmp,args[i].c_str());
        toBePushed.push_back(tmp);
    }
    commandsArgs.push_back(toBePushed);
    bundleSize++;
}
void processBundle::setInputPath(string in) {inp_txt = in; }
void processBundle::setOutputPath(string out) {out_txt = out; }
int processBundle::numberOfProcess() { return this->bundleSize;}
string processBundle::getInPath() {return inp_txt; }
string processBundle::getOutPath() {return out_txt; }
vector<string> processBundle::getCommands() {return commandsToExecute; }
vector<vector<char *>> processBundle::getArgs() {return commandsArgs; }
void processBundle::setPredSucc(processBundle * pred, processBundle * succ) { this->pred = pred; this->succ = succ;}
void processBundle::clearPredSucc(){ pred = NULL; succ = NULL; }
processBundle * processBundle::getPred() {return pred; }
processBundle * processBundle::getSucc() {return succ; }
