#include "parser.h"
#include "processBundle.h"
#include <iostream>
#include "string.h"
#define MAXLEN 255
#include <unordered_map>
#include <fcntl.h>
#include <sys/wait.h>

void call(processBundle *bundle){
    int child_status;
    
    for(int i = 0;i < bundle->numberOfProcess();i++){
        
        const char *cmd = bundle->getCommands()[i].c_str();

        int size = bundle->getArgs()[i].size();
        char *argsArr[size + 1];

        for(int j = 0; j < size;j++)
            argsArr[j] = bundle->getArgs()[i][j];

        argsArr[size] = NULL;
        if(fork() == 0){ // Childs.

            int fdOut,fdIn;
            if(bundle->getInPath() != ""){
                fdIn = open(bundle->getInPath().c_str(),O_RDONLY,0);
                int in = fileno(stdin);
                dup2(fdIn,in);
            }
            if(bundle->getOutPath() != ""){
                mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IRWXU;
                fdOut = open(bundle->getOutPath().c_str(),O_APPEND | O_WRONLY);
                if(fdOut == -1)
                    fdOut = open(bundle->getOutPath().c_str(), O_APPEND | O_WRONLY | O_CREAT,mode);
                
                int out = fileno(stdout);
                dup2(fdOut,out);
            }

            int x = execvp(cmd,argsArr);
            exit(0);

        }
    }
    while(wait(&child_status) > 0);
}


void setPredAndSucc(bundle_execution * bundle,unordered_map<string,processBundle *>& map,parsed_input inp){
    bundle_execution firstExecution = inp.command.bundles[0];
    string firstExecName = firstExecution.name;
    processBundle * firstExec = map[firstExecName];
    firstExec->setPredSucc(NULL,map[inp.command.bundles[1].name]);
    firstExecution.input;
    if(firstExecution.input != NULL)
        firstExec->setInputPath(firstExecution.input);

    int i = 1;
    int size = inp.command.bundle_count;

    for(;i < size - 1;i++){
        string prevExecName = inp.command.bundles[i - 1].name;
        processBundle * prevPb = map[prevExecName];
                    
        string currExecName = inp.command.bundles[i].name;
        processBundle * currPb = map[currExecName];

        string nextExecName = inp.command.bundles[i].name;
        processBundle * nextPb = map[nextExecName];

        currPb->setPredSucc(prevPb,nextPb);
    }

    bundle_execution lastExecution = inp.command.bundles[i];
    string lastExecName = lastExecution.name;
    processBundle * lastExec = map[lastExecName];
    lastExec->setPredSucc(map[inp.command.bundles[i - 1].name],NULL);

    if(lastExecution.output != NULL)
        lastExec->setOutputPath(lastExecution.output);

    return;
}


void createPipeAndExec(processBundle* bundle, int ** old_fds, int idx,int bSize){

    int size = bundle->numberOfProcess();

    for(int i = 0; i < size;i++){

        const char *cmd = bundle->getCommands()[i].c_str();
        int size = bundle->getArgs()[i].size();
        char *argsArr[size + 1];

        for(int j = 0; j < size;j++)
            argsArr[j] = bundle->getArgs()[i][j];

        argsArr[size] = NULL;
        
        int *cmd_fds = new int[2];
        pipe(cmd_fds);
        int fuckedStat = fork();
        //close(old_fds[idx - 1][0]);
        if(fuckedStat == 0){ //child

            if(bundle->getPred() != NULL){
                int teeControl = tee(old_fds[idx - 1][0],cmd_fds[1],65536,0);
                dup2(cmd_fds[0],STDIN_FILENO);
                close(cmd_fds[1]);
                close(cmd_fds[0]);
                close(old_fds[idx - 1][0]);
                close(old_fds[idx - 1][1]); //
            }
            else if(bundle->getPred() == NULL && bundle->getInPath() != ""){
                int fdIn = open(bundle->getInPath().c_str(),O_RDONLY,0);
                dup2(fdIn,STDIN_FILENO);
                close(fdIn);
                close(cmd_fds[1]);
                close(cmd_fds[0]);
            }
            else if(bundle->getPred() == NULL && bundle ->getInPath() == ""){
                close(cmd_fds[0]);
                close(cmd_fds[1]);
            }
            
            if(bundle->getSucc() != NULL){
                dup2(old_fds[idx][1],STDOUT_FILENO);
                close(old_fds[idx][1]);
                close(old_fds[idx][0]);
                //close(cmd_fds[1]);
                //close(cmd_fds[0]);

            }
            else if(bundle->getSucc() == NULL && bundle->getOutPath() != ""){
                mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IRWXU;
                int fdOut = open(bundle->getOutPath().c_str(),O_APPEND | O_WRONLY,mode);
                if(fdOut == -1)
                    fdOut = open(bundle->getOutPath().c_str(), O_APPEND | O_WRONLY | O_CREAT,mode);
                
                dup2(fdOut,STDOUT_FILENO);
                close(cmd_fds[1]);
                close(cmd_fds[0]);
                close(old_fds[idx][0]);
                close(old_fds[idx][1]);
            }
            else if(bundle->getSucc() == NULL && bundle->getOutPath() == ""){
                close(cmd_fds[0]);
                close(cmd_fds[1]);
                old_fds[idx][0];
                old_fds[idx][1];
            }
            int exeControl = execvp(cmd,argsArr);
        }
        else{
            close(cmd_fds[0]);
            close(cmd_fds[1]);
        }
    }   
    while(wait(NULL) > 0);
    //close(old_fds[idx][0]);
    
    //close(old_fds[idx - 1][1]);
    return;
}

void getInput(string buffer,parsed_input* input,int & is_bundle_creation){
    
}
int main(){

    std::string buff;
    int is_bundle_creation = 0;
    unordered_map<string,processBundle *> umap;
    std::string currBundleName;

    while(true)
    {
        parsed_input inp = parsed_input();
        char line[MAXLEN];
        memset(line,0,sizeof(line));
        fgets(line,MAXLEN,stdin);
        parse(line,is_bundle_creation,&inp);

        if(inp.command.type == QUIT){
            break;
        }
        else if(inp.command.type == PROCESS_BUNDLE_CREATE){
            //pbc geldi
            is_bundle_creation = 1;
            processBundle *currBundle = new processBundle(inp.command.bundle_name);
            umap[currBundle->getName()] = currBundle;
            currBundleName = currBundle->getName();
        }   
        else if(inp.command.type == PROCESS_BUNDLE_EXECUTION){
            parsed_command currCmd = inp.command;
            int size = currCmd.bundle_count;

            if(size == 1){
                // Sadece bir adet process bundle execute ettirilecekse pipe uygulamaya gerek yok direk call ile çalıştır geç.
                string bundleCmd(currCmd.bundles->name);
                processBundle* cmd = umap[bundleCmd];
                bundle_execution currExec = *currCmd.bundles;

                //bundle name i hashmapten aldık ve bu processBundle ının input ve output pathlerini yazdık.
                if(currExec.input != NULL)
                    cmd->setInputPath(currExec.input);
                if(currExec.output != NULL)
                    cmd->setOutputPath(currExec.output);
                call(cmd);
                cmd->setInputPath("");
                cmd->setOutputPath("");
            }   
            else{
 
                setPredAndSucc(inp.command.bundles,umap,inp);
                bundle_execution * arr = inp.command.bundles;
                int size = inp.command.bundle_count;

                int **pipeHolders = new int *[size - 1];

                for(int i = 0; i < size - 1;i++)
                    pipeHolders[i] = new int[2];
                
                for(int i = 0;i < size;i++){
                    string name = arr[i].name;
                    processBundle * curr = umap[name]; 
                    
                    if(i < size - 1)
                        pipe(pipeHolders[i]);

                    pid_t forkControl = fork();
                    if(forkControl == 0){
                        createPipeAndExec(curr,pipeHolders,i,size);
                    }
                    else{
                        if(i < size - 1){
                            close(pipeHolders[i][0]);
                            close(pipeHolders[i][1]);
                        }
                    }
                }
                for(int i = 0; i < size - 1;i++)
                    delete pipeHolders[i];
                
                delete pipeHolders;
                while(wait(NULL) > 0);
            }
        }
        else if(inp.command.type == PROCESS_BUNDLE_STOP){
            //pbs geldi
            is_bundle_creation = 0;
        }
        else if(is_bundle_creation == 1){
            processBundle *currBundle = umap[currBundleName];
            string cmd(inp.argv[0]);
            vector<string> arguments;
            for(int i = 1;inp.argv[i] != NULL;i++){
                arguments.push_back(inp.argv[i]);
            }
            currBundle->insertCommand(cmd,arguments);
        }
    }
    return 0;
}