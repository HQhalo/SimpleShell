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
char *bufferConst;
char *history;
int bufsize;
void __ini__(){
    bufsize  = 100;
    bufferConst = (char *)malloc(bufsize * sizeof(char));
    history = (char *)malloc(bufsize * sizeof(char));
}

char *getInput(){
    getline(&bufferConst,&bufsize,stdin);
    bufferConst[strlen(bufferConst)-1] = '\0'; // remoce enter character
    memcpy(history,bufferConst,strlen(bufferConst));
    return bufferConst;
}

//
// return type of command: FLAG_PIPE or FLAG_NONE_PIPE 
// matrix commands has 1 or 2 string,  
int processLine(char *line,char **commands){

}

//
// return FLAG_REDIRECTION_L or FLAG_REDIRECTION_R or FLAG_REDIRECTION_NONE
// separte command 
// if command has redirection: redir contain output or input name file
// flagWait = 1 if command end by &, = FLAG_HISTORY if command == "!!"
int processCommand(char *command,char **args, char *redir,int *flagWait){

}

void execRedirR(char** args,int *flagWait){

}
void execRedirL(char** args,int *flagWait){

}
void execPipe(char **commands){

}
void execArgs(char** args,int *flagWait) 
{ 
    pid_t pid = fork();  
    if (pid == -1) { 
        printf("\nFailed forking child.."); 
        return; 
    } else if (pid == 0) { 
        if (execvp(args[0], args) < 0) { 
            printf("\nCould not execute command..\n"); 
        } 
        exit(0); 
    } else {
        if(flagWait == 1){ 
            wait(NULL);  
        }
        return; 
    } 
} 

int main(){
    char * args[MAX_LINE/2+1];
    char *commands[2];
    char redir[MAX_LINE];
    
    char *buffer = NULL;
    int shouldRun = 1;

    int flagHistory = -1;
    int flagPipe = -1;
    int flagRedir = -1;
    int flagWait = -1;
    __ini__();

    while (shouldRun == 1){
        printf("\nosh>");  
        fflush(stdout);
        if(flagHistory == FLAG_HISTORY){
            buffer = history;
            flagHistory = -1;
        }
        else {
            buffer = getInput();
        }
        flagPipe = processLine(buffer,commands);

        if(flagPipe == FLAG_PIPE){
            // deo can care Redirection
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
                }
                else {
                    execArgs(args,flagWait);
                }

            }else if (r == FLAG_REDIRECTION_R){
                //
                execRedirR(args,flagWait);
            }else if(r == FLAG_REDIRECTION_L){
                //
                exceRedirL(args,flagWait);
            }

        }
        
    }
    
    return 0;
}