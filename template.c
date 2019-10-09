#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include <sys/wait.h> 

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
    *FlagHistory = -1;
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
    pid_t  pid;
    int status;

    if ((pid = fork()) < 0) {
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    }
    else if (pid == 0) { 
        //open file redir
        int fd = open(redir[0], O_CREAT | O_WRONLY | O_TRUNC, 0777);
        if (fd < 0) {
            printf("*** ERROR: open output file failed\n");
            exit(1);
        }   
        //replace STDOUT descriptor by fd descriptor
        if (dup2(fd, STDOUT_FILENO) < 0) {
            close(fd);
            printf("** ERROR: dup failed\n");
            exit(1);
        }
        close(fd);
        //exec as usual
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

void execRedirL(char** args,char **redir,int flagWait){
        pid_t  pid;
    int status;

    if ((pid = fork()) < 0) {
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    }
    else if (pid == 0) { 
        //open file redir
        int fd = open(redir[0], O_RDONLY, 0777);
        if (fd < 0) {
            printf("*** ERROR: open input file failed\n");
            exit(1);
        }   
        //replace STDIN descriptor by fd descriptor
        if (dup2(fd, STDIN_FILENO) < 0) {
            close(fd);
            printf("*** ERROR: dup failed\n");
            exit(1);
        }
        close(fd);
        //exec as usual
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
void execArgs(char** args,int flagWait, int isWrite )    
{ 
    pid_t  pid;
    int status;
    pid = fork();
    if (isWrite== 1 && flagWait == FLAG_WAIT)
        printf("Waiting for Finish child");

    if (pid < 0) {
        printf("*** ERROR: forking child process failed\n");
        exit(1);
    }
    else if (pid == 0) { 
        //sleep(2);    
        if (execvp(*args, args) < 0) {     
            printf("*** ERROR: exec failed\n");
            exit(1);
        }
        exit(0);
    }
    else { 
        if (flagWait == FLAG_WAIT) 
            wait(NULL);
        //waitpid(pid, &status, 0);
        /*if(flagWait == FLAG_WAIT){
            
            while (wait(&status) != pid);
        }*/
        return;
    }
} 

void processHelper(char *commandline){

    //printf(commandline);
    //return;

    char * args[MAX_LINE/2+1];
    char *commands[2];
    char *redir[MAX_LINE/2+1];
    
    char *buffer = NULL;
    int shouldRun = 1;

    int flagHistory = -1;
    int flagPipe = -1;
    int flagRedir = -1;
    int flagWait = -1;

    fflush(stdout);
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
                    int a = 0;
                    execArgs(args,FLAG_WAIT, a);
                }

            }else if (r == FLAG_REDIRECTION_R){
                //printf("%s",redir[0]);
                execRedirR(args,redir,flagWait);
            }else if(r == FLAG_REDIRECTION_L){
                //printf("%s",redir[0]);
                execRedirL(args,redir,flagWait);
            }
}
void execPipe(char **commands){

    int parent = fork();

    if (parent > 0){
        wait(NULL);
    }

    if (parent == 0)
    {
        int pipe1[2];
        pipe(pipe1);

        int pid = fork();

        if (pid < 0){
            printf("ERROR forking\n");
            fflush(stdout);
            return;
        }
        
        if (pid == 0){

            int pidChild = fork();

            if (pidChild < 0){
                exit(1);
            }

            if (pidChild == 0){
                close(pipe1[1]);
                dup2(pipe1[0], 0);

                processHelper(commands[1]);
                //close(pipe1[0]);
                //close(pipe1[1]);
            
                exit(0);
            }
            dup2(pipe1[1], 1);
            processHelper(commands[0]);
            exit(0);
        }

        if (pid > 0){
            wait(NULL);
            fflush(stdout);
        }
        exit(0);
    }

    //fflush(stdout);
    return;
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
        fflush(stdout);
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

                    execArgs(args,flagWait , 1);
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