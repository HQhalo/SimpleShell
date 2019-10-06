#include<stdio.h>
#include<unistd.h>
#include<string.h>

#define MAX_LINE 80

#define FLAG_HISTORY 0
#define FLAG_REDIRECTION_NONE 5
#define FLAG_REDIRECTION_L 1
#define FLAG_REDIRECTION_R 2
#define FLAG_PIPE 3
#define FLAG_PIPE_NONE 4
#define FLAG_WAIT  6
#define FLAG_WAIT_NONE 7
#define FLAG_HISTORY 8
#define FLAG_HISTORY_EMPTY 9

char *bufferConst;
char *history;
int flagIniHis = 0;
int bufsize;

void __ini__(){
    bufsize  = 100;
    bufferConst = (char *)malloc(bufsize * sizeof(char));
    history = (char *)malloc(bufsize * sizeof(char));
}

char *getInput(int *FlagHistory){
    getline(&bufferConst,&bufsize,stdin);
    bufferConst[strlen(bufferConst)-1] = '\0'; // remove enter character
    
    // check history
    if(strcmp(bufferConst,"!!")==0){
        if(flagIniHis == 0){
            printf("No commands in history"); 
            *FlagHistory = FLAG_HISTORY_EMPTY;
            return NULL;        
        }
        *FlagHistory =FLAG_HISTORY;
        return history;
    }
    memcpy(history,bufferConst,strlen(bufferConst));
    
    flagIniHis = 1;
    FlagHistory = -1;
    return bufferConst;
}

//
// return type of command: FLAG_PIPE or FLAG_NONE_PIPE 
// matrix commands has 1 or 2 string,  
int processLine(char *line,char **commands){
    commands[0] = strtok(line,"|");
    commands[1] = strtok(NULL,"|");
    if(commands[1] != NULL){
        return FLAG_PIPE;
    }
    else {
        return FLAG_PIPE_NONE;
    }
    return -1;
}
int separateSpace(char *command,char **args){
    int i =0;
    char *token;
    token = strtok(command," ");
    while(token != NULL){
        args[i] = token;
        i++;
        token = strtok(NULL," ");
    }
    
    if(strcmp(args[i-1],"&")==0){
       
        args[i-1] = NULL;
        return FLAG_WAIT_NONE;
    }
    else {
        args[i] = NULL;
        return FLAG_WAIT;
    }
    return -1;
}
//
// return FLAG_REDIRECTION_L or FLAG_REDIRECTION_R or FLAG_REDIRECTION_NONE
// separte command 
// if command has redirection: redir contain output or input name file
// flagWait = 1 if command end by &
//
int processCommand(char *command,char **args, char **redir,int *flagWait){
    char *tempCommand;
    char *tempDir;
    if(strchr(command,'<')){
        tempCommand = strtok(command,"<");
        tempDir = strtok(NULL,"<");
        
        separateSpace(tempCommand,args);
        *flagWait = separateSpace(tempDir,redir);

        return FLAG_REDIRECTION_L;
    }
    else if(strchr(command,'>')){
        tempCommand = strtok(command,">");
        tempDir = strtok(NULL,">");
        
        separateSpace(tempCommand,args);
        *flagWait = separateSpace(tempDir,redir);
        return FLAG_REDIRECTION_R;
    }
    else {
        *flagWait = separateSpace(command,args);
        return FLAG_REDIRECTION_NONE;
    }

    *flagWait = -1;
    return -1;
}

void execRedirR(char** args,char **redir, int flagWait){

}
void execRedirL(char** args,char **redir,int flagWait){

}
void execPipe(char **commands){

    //creat pipe in and out
    int fd[2];
    pipe(fd);

    int pid;
    pid = fork();
    

    //
    if (pid < 0){
        printf("*** ERROR: Creat pipe fail because can't fork");
        return;
    }
    if (pid != 0)  // parent 
    {   

    }
    else if (pid == 0) // child 
    {

        int pidChild;
        pidChild = fork();
        if (pidChild < 0) {
            printf("*** ERROR: Creat pipe fail because can't fork");
        }
        else 
        if (pidChild != 0)  //Child F1 call Right arg
        {
            close(fd[0]);

            dup2(fd[1], STDOUT_FILENO);
            processHelper(commands[0]);

            close(fd[1]);

            exit(1);
        }
        else // child F2 call Left arg
        {
            close(fd[1]);

            dup2(fd[0], STDIN_FILENO);
            processHelper(commands[1]);
            close(fd[0]);
            
            exit(1);
        }
    }
    
}
void execArgs(char** args,int flagWait) 
{ 
    pid_t  pid;
    int status;

    if ((pid = fork()) < 0) {
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    }
    else if (pid == 0) { 
        //sleep(2);    
        if (execvp(*args, args) < 0) {     
            printf("*** ERROR: exec failed\n");
            exit(1);
        }
    }
    else {  
        if(flagWait == FLAG_WAIT){
            printf("Waiting for Finish child");
            while (wait(&status) != pid);
        }
        return;
    }
} 

void processHelper(char *commandline){
    char * args[MAX_LINE/2+1];
    char *commands[2];
    char *redir[MAX_LINE/2+1];
    
    char *buffer = NULL;
    int shouldRun = 1;

    int flagHistory = -1;
    int flagPipe = -1;
    int flagRedir = -1;
    int flagWait = -1;


    flagPipe = processLine(commandline,commands);
    
    int r = processCommand(commands[0],args,redir,&flagWait);
            if(r == FLAG_REDIRECTION_NONE){
                //
                if(flagWait == FLAG_HISTORY){
                   
                    flagHistory = FLAG_HISTORY;
                    buffer = history;
                    return ;
                }
                else {
                    //printf("%s_%s_",args[0],args[1]);
                    execArgs(args,FLAG_WAIT_NONE);
                }

            }else if (r == FLAG_REDIRECTION_R){
                //printf("%s",redir[0]);
                execRedirR(args,redir,flagWait);
            }else if(r == FLAG_REDIRECTION_L){
                //printf("%s",redir[0]);
                execRedirL(args,redir,flagWait);
            }
}

int main(){
    char * args[MAX_LINE/2+1];
    char *commands[2];
    char *redir[MAX_LINE/2+1];
    
    char *buffer = NULL;
    int shouldRun = 1;

    int flagHistory = -1;
    int flagPipe = -1;
    int flagRedir = -1;
    int flagWait = -1;
    __ini__();

    while (shouldRun == 1){
        printf("\nosh>");  
        fflush(stdin);
        
        buffer = getInput(&flagHistory);

        if(flagHistory == FLAG_HISTORY_EMPTY) {
            continue;
        }
        flagPipe = processLine(buffer,commands);
        if(flagPipe == FLAG_PIPE){
            
            execPipe(commands);
        } 
        else if(flagPipe == FLAG_PIPE_NONE){
            // Redirection
            int r = processCommand(commands[0],args,redir,&flagWait);
            if(r == FLAG_REDIRECTION_NONE){
                //
                if(flagWait == FLAG_HISTORY){
                   
                    flagHistory = FLAG_HISTORY;
                    buffer = history;
                    continue;
                }
                else {
                    //printf("%s_%s_",args[0],args[1]);
                    execArgs(args,flagWait);
                }

            }else if (r == FLAG_REDIRECTION_R){
                //printf("%s",redir[0]);
                execRedirR(args,redir,flagWait);
            }else if(r == FLAG_REDIRECTION_L){
                //printf("%s",redir[0]);
                execRedirL(args,redir,flagWait);
            }
            

        }
        
    }
    
    return 0;
}