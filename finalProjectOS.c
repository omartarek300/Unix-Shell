/*
code idea:
    1st we read string command from user using read_str function
    2nd we parse this string and get the command and the rest parameters using parsing function in which we check if there
    is a '&' parameter to enable or disaple the parent wait using flag
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

void interrupt_handler(int sig);
void parsing(char line[]);
void operation();
void read_str();

char *command;
char *parameters[10];
char line[100] = {0};
FILE *fp;
int flag = 0;
int spaces = 0;

// ##########################  main  #####################################
int main()
{
    signal(SIGCHLD, interrupt_handler);
    remove("data.log");          // delete old log file
    fp = fopen("data.log", "a"); // open log file
    while (1)                   // terminal loop
    {
        operation();
    }
    return 0;
}

void operation()
{
    read_str(); // gets user input and counts the spaces denoting the number of arguments
    parsing(line);

    if ((strcmp(command, "exit") == 0))
    {
        fclose(fp);
        exit(EXIT_SUCCESS);
    }

    else if (strcmp(command, "cd") == 0)
    {
        if (chdir(parameters[1]) == 0)
        {
        }
        else
        {
            printf("\n");
            printf("\033[0;31m"); //print in red color
            printf("path error\n\n");
        }
    }
    else
    {
        pid_t child_pid = fork(); // create a child process

        if (child_pid == 0)
        {
            // child continues here
            int status_code = execvp(command, parameters); // execute the command with the given paramters
            if (status_code == -1)                         // check exit code
            {
                printf("\033[0;31m"); //print in red color
                printf("Terminated Incorrectly\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (child_pid > 0)
        {
            // parent continues here
            // printf("\n");
            // printf("from parent: pid=%d child_pid=%d\n",(int)getpid(), (int)child_pid);

            // Wait until child process exits or terminates. The return value of waitpid() is PID of the child process, while its argument is filled with exit code and termination reason.

            printf("\033[0;36m"); //print in Cyan color
            printf("\n");

            if (flag == 0)
            {
                int status;
                pid_t waited_pid = waitpid(child_pid, &status, 0); // wait for child to terminate

                if (waited_pid < 0) // means terminated incorrectly
                {
                    fprintf(fp, "child process (%d) terminated.\n", child_pid);
                    perror("waitpid() failed");
                    exit(EXIT_FAILURE);
                }
                else if (waited_pid == child_pid) // means terminated correctly
                {
                    if (WIFEXITED(status))
                    {
                        /* WIFEXITED(status) returns true if the child has terminated
                 * normally. In this case WEXITSTATUS(status) returns child's
                 * exit code.
                 */
                        fprintf(fp, "child process (%d) terminated.\n", child_pid);
                        printf("\033[0m"); // reset color
                        printf("\n");
                        //printf("from parent: child exited with code %d\n",
                              // WEXITSTATUS(status));
                    }
                }
            }
            else
            {
                flag = 0;
                 operation();
            }
        }
    }
}

void read_str()
{
    int i = 0;
    spaces = 0;

    printf("\033[0;32m"); //print in green color
    printf("Enter command \n");

    printf("\033[0;36m"); //print in Cyan color
    printf("~$ ");
    printf("\033[0m"); //print in reset color

    fgets(line, 100, stdin); // gets 100 cahracters from stdin
    while (line[i] != '\0')  // loop until end of line
    {
        if (line[i] == ' ') // count spaces
        {
            spaces++;
        }
        i++;
    }
}

void parsing(char line[])
{
    if (spaces == 0) // means single parameter is passed without arguments
    {
        char *parse = strtok(line, "\n"); // take the first parameter and remove the '\n' character
        command = parse;
        parameters[0] = parse;
        parameters[1] = NULL;
        parameters[2] = NULL;
        //strcat(line,'\n');
    }
    else // means a command is run with arguments
    {
        char *parse = strtok(line, " "); // returns the string before the delimiter ' ' (space)

        int i = 0;
        while (parse != NULL)
        {
            if (!(strcmp(parse, "&"))) // if ampersand is found set "flag" to one to indicate background task
            {
                flag = 1; // indicates background task
                parameters[i] = NULL;
            }
            else // means this is a parameter or an argument
            {
                parameters[i] = parse;
            }
            parse = strtok(NULL, " \n");
            i++;
        }
        command = parameters[0];
        parameters[i] = NULL;
    }
    //###########################################################################3
}

void interrupt_handler(int sig)
{
    int status;
    pid_t pid;									// in waitpid the 1st argument is -1 to make it wait the child process to end
										// WNOHANG gets the status information immediately
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)				// waitpid returns child pid
    {
        // executed if the child is terminated
        if (WIFEXITED(status))							//WIFEXITED evaluates ture  if the child process ended normally
        {

            //fp = fopen("log.txt","a");
            fprintf(fp, "child process (%d) terminated.\n", pid);		//prints the child process terminated with its pid
        }
    }
}

