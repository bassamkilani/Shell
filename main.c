//
//  main.c
//  shell
//
//  Created by Bassam Kilani on 22/11/2019.
//  Copyright © 2019 Bassam Kilani. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define BACKGROUND_EXECUTION 0
#define FOREGROUND_EXECUTION 1
#define BUFSIZE 1024
#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
#define LESS_THAN 1
#define GREATER_THAN 2
#define PIPE 3

int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);

char *builtin_str[] = {"cd", "help", "exit", "jobs"};

int (*builtin_func[])(char **) = {&sh_cd, &sh_help, &sh_exit};

int num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int sh_cd(char **args)
{
    if (args[1] == NULL)
    {
        chdir("/Users/bassamkilani/");
    }
    else
    {
        if (chdir(args[1]) != 0)
        {
            perror("shell");
        }
    }
    return 1;
}

int sh_help(char **args)
{
    int i;
    printf("Bassam Kilani's Shell\n");
    printf("Reg. No.: 11610417\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following commands are built in:\n");
    
    for (i = 0; i < num_builtins(); i++)
    {
        printf("  %s\n", builtin_str[i]);
    }
    
    printf("Use the \"man\" command for information on other programs.\n");
    return 1;
}

int sh_exit(char **args)
{
    return 0;
}

int sh_jobs(char**** jobs){
    int i = 0;
    int stat = 9988;
    while(strcmp((*jobs)[i][0], "") != 0) {
        waitpid(atoi((*jobs)[i][0]), &stat, WNOHANG);
        if(stat == 0){
            strcpy((*jobs)[i][2], "done");
        }
        if(strcmp((*jobs)[i][2], "done") != 0){
            printf("[%d]   %s   %s   %s\n", i, (*jobs)[i][0], (*jobs)[i][1], "running");
        }else{
            printf("[%d]   %s   %s   %s\n", i, (*jobs)[i][0], (*jobs)[i][1], (*jobs)[i][2]);
        }
        i++;
    }
    return 1;
}

char *read_line(void)
{
    int bufsize = BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;
    
    if (!buffer)
    {
        printf("memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    while (1)
    {
        c = getchar();
        
        if (c == EOF || c == '\n')
        {
            buffer[position] = '\0';
            return buffer;
        }
        else
        {
            buffer[position] = c;
        }
        position++;
        
        if (position >= bufsize)
        {
            bufsize += BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer)
            {
                printf("memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **parse_line(char *line)
{
    int bufsize = TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;
    
    if (!tokens)
    {
        printf("memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(line, TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;
        
        if (position >= bufsize)
        {
            bufsize += TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                printf("memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        
        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int launch(char **args, int mode, int *i, char**** jobs)
{
    pid_t pid, wpid;
    int idd;
    int status;
    char id[20];
    
    pid = fork();
    idd = pid;
    if (pid == 0)
    {
        // Child process
        if (execvp(args[0], args) == -1)
        {
            printf("command not found\n");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        perror("fork error");
    }
    else
    {
        // Parent process
        if(mode == BACKGROUND_EXECUTION){
            sprintf(id, "%d", idd);
            strcpy((*jobs)[*i][0], id);
            strcpy((*jobs)[*i][1], args[0]);
            *i += 1;
        }else{
            do
            {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status)); // wait for child until it's exited or been killed
        }
    }
    return 1;
}

#define READ 0
#define WRITE 1
int exec_pipe(char **args, int index, int *i, char**** jobs){
    
   x
    return 1;
}

int execute(char **args, int *i, char**** jobs)
{
    int mode = FOREGROUND_EXECUTION;
    int size;
    int symbol = 0;
    int index = 0;
    
    if (args[0] == NULL)
    {
        return 1;
    }
    
    for(size = 0; args[size] != NULL; size++);
    
    for(int i = 0 ; i < size ; i++){
        if(strcmp(args[i], "<") == 0){
            symbol = LESS_THAN;
            index = i;
        }else if(strcmp(args[i], ">") == 0){
            symbol = GREATER_THAN;
            index = i;
        }else if(strcmp(args[i], "|") == 0){
            symbol = PIPE;
            index = i;
        }
    }
    
    if(symbol == GREATER_THAN){
        if(args[index + 1] == NULL){
            printf("there must be arguments after the symbol\n");
        }else{
            int savedStdOut = dup(1);
            int fd = open(args[index + 1], O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            dup2(fd, 1);
            close(fd);
            
            args[index] = NULL;
            size--;
            args[index + 1] = NULL;
            size--;
            
            if(strcmp(args[0], "jobs") == 0){
                return sh_jobs(jobs);
            }else{
                for (int i = 0; i < num_builtins(); i++)
                {
                    if (strcmp(args[0], builtin_str[i]) == 0)
                    {
                        return (*builtin_func[i])(args);
                    }
                }
            }
            
            if(strcmp(args[size - 1], "&") == 0){
                mode = BACKGROUND_EXECUTION;
                args[size - 1] = NULL;
            }
            
            int ret = launch(args, mode, i, jobs);
            dup2(savedStdOut, 1);
            close(savedStdOut);
            return ret;
        }
    }else if(symbol == LESS_THAN){
        if(args[index + 1] == NULL){
            printf("there must be arguments after the symbol\n");
        }else{
            int savedStdIn = dup(fileno(stdin));
            if(freopen(args[index + 1], "r", stdin) == NULL){
                printf("file not found\n");
                return 1;
            }
            else{
                args[index] = NULL;
                size--;
                args[index + 1] = NULL;
                size--;
                
                if(strcmp(args[0], "jobs") == 0){
                    return sh_jobs(jobs);
                }else{
                    for (int i = 0; i < num_builtins(); i++)
                    {
                        if (strcmp(args[0], builtin_str[i]) == 0)
                        {
                            return (*builtin_func[i])(args);
                        }
                    }
                }
                
                if(strcmp(args[size - 1], "&") == 0){
                    mode = BACKGROUND_EXECUTION;
                    args[size - 1] = NULL;
                }
                
                int ret = launch(args, mode, i, jobs);
                fclose(stdin);
                stdin = fdopen(savedStdIn, "r");
                return ret;
            }
        }
    }else if(symbol == PIPE){
        if(args[index + 1] == NULL){
            printf("there must be arguments after the symbol\n");
        }else{
            for (int i = 0; i < size; i++) {
                printf("args[%d]: %s, ", i, args[i]);
            }
            printf("Index: %d\n", index);
            exec_pipe(args, index, i, jobs);
        }
    }else{
        if(strcmp(args[0], "jobs") == 0){
            return sh_jobs(jobs);
        }else{
            for (int i = 0; i < num_builtins(); i++)
            {
                if (strcmp(args[0], builtin_str[i]) == 0)
                {
                    return (*builtin_func[i])(args);
                }
            }
        }
        
        if(strcmp(args[size - 1], "&") == 0){
            mode = BACKGROUND_EXECUTION;
            args[size - 1] = NULL;
        }
        return launch(args, mode, i, jobs);
    }
    return 1;
}

void loop(int *i, char**** jobs)
{
    char *line;
    char **args;
    int status;
    char cwd[256];
    
    do
    {
        getcwd(cwd, sizeof(cwd));
        printf("%s> ", cwd);
        line = read_line();
        args = parse_line(line);
        status = execute(args, i, jobs);
        
        free(line);
        free(args);
    } while (status);
}

int main(int argc, char const *argv[])
{
    chdir("/Users/bassamkilani/");
    int i = 0;
    char*** jobs = calloc(30, sizeof(char **));
    for(int z = 0; z < 30; z++) {
        jobs[z] = calloc(30, sizeof(char*));
        for(int i = 0; i < 30; i++) {
            jobs[z][i] = calloc(30, sizeof(char));
        }
    }
    loop(&i, &jobs);
    
    return 0;
}
