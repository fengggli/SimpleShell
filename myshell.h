#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/**return value of sparseline
flag < = error command
flag = 0 foregound command
flag =1 backgound command
**/
typedef struct tuple{
	int flag;
	char *** cmds;
}Tuple;

int do_loop(void);
char * getinput(void);
Tuple sparseline(char* buffer);
int run_this_line(char *** cmds);
int execute_commands(char ** tokens);
int myfree(char ***cmds);

