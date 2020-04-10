#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

const int MAXCOM = 1000; // max number of letters to be supported
const int MAXLIST = 100; // max number of commands to be supported

// Clearing the shell using escape sequences
inline void clear()
{
	std::cout << "\033[H\033[J";
}

// Greeting the shell during startup
void init_shell()
{
	clear();
	std::cout << "************\n"
		      << "    cush    \n"
		      << "************\n";
	std::vector<char *> username;
	username.push_back(getenv("USER"));
	std::cout << "\nUSER is " << std::string(std::begin(username), std::end(username)) << "\n";
	sleep(1);
	clear();
}

// Function to take input
int takeInput(std::string str)
{
	std::string buf;

	buf = (readline("\n>>> "));
	if (buf.size())
	{
		const char* temp = new char;
		temp = buf.c_str();
		add_history(temp);
		str = buf;
		delete temp;
		return 0;
	}
	else
	{
		return 1;
	}
}

//  Function to print current directory
void printDir()
{
	std::string cwd;
	char *temp = new char;
	getcwd(temp, sizeof(temp));
	cwd = temp;
	delete temp;
	std::cout << "\nDir: " << cwd;
}

// Function where the system command is executed
void execArgs(std::vector<char **>parsed)
{
	// Forking a child
	pid_t pid = fork();

	if (pid == -1)
	{
		std::cout << "\nFailed forking child...";
		return ;
	}
	else if (pid == 0)
	{
		if (execvp(*parsed[0], parsed[0]) < 0)
		{
			std::cout << "\nCould not execute command...";
		}
		exit(0);
	}
	else
	{
		// waiting for child to terminate
		wait(NULL);
		return ;
	}
}

// Function where piped system commands is executed
//void execArgsPiped(char** parsed, char** parsedpipe)
void execArgsPiped(std::vector<char **>parsed, std::vector<char **>parsedpipe)
{
	// 0 is read end, 1 is write end
	int pipefd[2];
	pid_t p1, p2;

	if (pipe(pipefd) < 0)
	{
		printf("\nPipe could not be initialized");
		return ;
	}
	p1 = fork();
	if (p1 < 0)
	{
		printf("\nCould not fork");
		return ;
	}

	if (p1 == 0)
	{
		// Child 1 is executing..
		// It only needs to write at thw write end
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[1]);
		if (execvp(parsed[0], parsed) < 0)
		{
			printf("\nCould not execute command 1...");
			exit(0);
		}
	}
	else
	{
		// Parent executing
		p2 = fork();

		if (p2 < 0)
		{
			printf("\nCould not fork");
			return ;
		}

		// Child 2 is executing
		// It only needs to read at the read end
		if (p2 == 0)
		{
			close(pipefd[1]);
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[0]);
			if (execvp(parsedpipe[0], parsedpipe) < 0)
			{
				printf("\nCould not execute command 2...");
				exit(0);
			}
		}
		else
		{
			// parent executing, waiting for two children
			wait(NULL);
			wait(NULL);
		}
	}
}

// Help command built in
void openHelp()
{
	puts("\n**********HELP**********"
	     "\nList of commands supported: "
		 "\n> cd"
		 "\n> ls"
		 "\n> exit"
		 "\n> all other general commands are available in UNIX shell"
		 "\n> pipe handling"
		 "\n> improper space handling");

	return ;
}

// Function to execute builtin commands
int ownCmdHandler(char** parsed)
{
	int NoOfOwnCmds = 4, i, switchOwnArg = 0;
	char* ListOfOwnCmds[NoOfOwnCmds];
	char* username;

	ListOfOwnCmds[0] = "exit";
	ListOfOwnCmds[1] = "cd";
	ListOfOwnCmds[2] = "help";
	ListOfOwnCmds[3] = "hello";

	for (i = 0; i < NoOfOwnCmds; ++i)
	{
		if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0)
		{
			switchOwnArg = i + 1;
			break;
		}
	}

	switch (switchOwnArg)
	{
		case 1: 
			printf("\nGoodbye\n");
			exit(0);
		case 2:
			chdir(parsed[1]);
			return 1;
		case 3:
			openHelp();
			return 1;
		case 4: 
			username = getenv("USER");
			printf("\nHello %s. \nPlease report errors if you find any. \nUse help to know more.\n", username);
			return 1;
		default: break;
	}
	return 0;
}

// function for finding pipe
int parsePipe(char* str, char** stripped)
{
	int i;
	for (i = 0; i < 2; ++i)
	{
		stripped[i] = strsep(&str, "|");
		if (stripped[i] == NULL)
			break;
	}

	if (stripped[1] == NULL)
		return 0; // returns zero if no pipe is found
	else
	{
		return 1;
	}
}

// function for parsing command words
void parseSpace(char* str, char** parsed)
{
	int i;
	for (i = 0; i < MAXLIST; i++)
	{
		parsed[i] = strsep(&str, " ");

		if (parsed[i] == NULL)
			break;
		if (strlen(parsed[i]) == 0)
			i--;
	}
}

int processString(char* str, char** parsed, char** parsedpipe)
{
	char* stripped[2];
	int piped = 0;

	piped = parsePipe(str, stripped);

	if (piped)
	{
		parseSpace(stripped[0], parsed);
		parseSpace(stripped[1], parsedpipe);
	}
	else
	{
		parseSpace(str, parsed);
	}

	if (ownCmdHandler(parsed))
		return 0;
	else
		return 1 + piped;
}

int main()
{
	char inputString[MAXCOM], *parsedArgs[MAXLIST];
	//std::vector<char**>parsedArgs[MAXLIST];
	char* parsedArgsPiped[MAXLIST];
	int execFlag = 0;
	init_shell();

	while (1)
	{
		// print shell line
		printDir();
		// take input
		if (takeInput(inputString))
			continue;
		// process
		execFlag = processString(inputString, parsedArgs, parsedArgsPiped);
		// execFlag returns 0 if there is no command
		// or it is a builtin command,
		// 1 if it is a simple commmand
		// 2 if it is including a pipe.

		// execute
		if (execFlag == 1)
			execArgs(parsedArgs);

		if (execFlag == 2)
			execArgsPiped(parsedArgs, parsedArgsPiped);
	}
	return 0;
}
