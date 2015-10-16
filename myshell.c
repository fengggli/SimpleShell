#include "myshell.h"

#define TURE 1
#define FALSE 0
#define ERROR -1
#define READ_BUFF_LEN 2
#define READ_BUFF_INC 5
#define TOKEN_LEN 10// default length for one token
#define TOKEN_LEN_INC 10 

#define NUM_TOKENS 10 //defaut token numbers in one command
#define NUM_TOKENS_INC 10 //used for reallocate the space for tokens

#define NUM_COMMANDS 10 //default commands in one read
#define NUM_COMMANDS_INC 10 //used for reallocate the space for commands

#define NUM_PIPE 5 // default pipe numbers
#define NUM_PIPE_INC 5// used for reallocate space for pipes


int main(int argc, char ** argv){
	printf("start myshell\nCorpiright@fengli\n");
	int ret;
	ret = do_loop();
	if(ret == ERROR)
		printf("error, exit mainloop\n");
	return 1;
}

/* name: do_loop
usage: do the main loop
input: none
out put:none
return: error number
*/
int do_loop(){
	int ret = 0, ret2 = 0;
	Tuple mytuple;
	char * buffer;
	
	do{
		printf("$mysh~");
		buffer = getinput();
		mytuple = sparseline(buffer);
		
		//make sure it is correct coomand input
		if(mytuple.flag < 0 ){
			if(buffer)
				free(buffer);	
			continue;
		}
		
		if(mytuple.flag == 1){
			// it background flag is set then the follow items will excuted in child process	
			int ret3 = fork();
			if(ret3 == 0){
				//for the child process
				ret = run_this_line(mytuple.cmds);

				//ret = execute_commands(tokens);
				if(ret <=0){
					fprintf(stderr,"backgound child error\n");
					exit(-1);
				ret2 = myfree(mytuple.cmds);
				exit(0);		 		
				}
			}
			else{
				printf("[1] %d\n",ret3);
			}
		}

		//if is excuted in forgound
		else{
		
			ret = run_this_line(mytuple.cmds);
			//ret = execute_commands(tokens);
			if(ret <= 0){
				printf("this line is not excuted correctly");
			}
			ret2 = myfree(mytuple.cmds);
		}
		if(buffer)
			free(buffer);
	}while(ret != ERROR);
	return ret;
}


/* name: getinput
usage: read the input from the user
input: none
out put:none
return: the line read ended with '\0'
*/
char *getinput(void ){
	int buff_length = READ_BUFF_LEN;
	char * buffer = (char *)(malloc(buff_length*sizeof(char)));
	//to-do:should expand buffer if needed.
	if(buffer == NULL){
		printf("can't allocate space for read buffer\n");
		exit(-1);
	}
	// get every input in the buffer.
	int iter = 0;
	while(1){
		char tmp = getchar();
		//replace \n with \0
		//printf("%c %d %dis read\n",tmp,tmp,'\n');
		if(iter >= buff_length){
			buff_length += READ_BUFF_INC;// some problems here
			buffer = (char *)realloc(buffer,buff_length);
			if(!buffer){
				printf("realloc buffer error when getting a line\n");
				exit(-1);
			}
		}
		if(tmp == '\n'){
			//mark the end 
			*(buffer + iter) = '\0';
			return buffer;
		}
		*(buffer + iter) = tmp;
		iter ++;
			
	}
	//char ** tokens = split_sentence(buffer);
	//strcpy(buffer,"  this is a test\0");
	return buffer;
}

/* name: sparseline
** usage: sparse the context in read buffer, into several tokens
** input: linebuffer
** return: see the typedef of Tuple in the myshell.h 
** note: for space delimitered only
*/

Tuple sparseline(char *buffer){
	if(buffer == NULL){
		printf("spase empty line\n");
		exit(-2);
	}
	char* p = buffer;

	int num_commands = NUM_COMMANDS;
	char *** commands = (char ***)malloc(num_commands*sizeof(char**));
	if(commands == NULL){
		printf("commands allocated failed\n");
		exit(-3);
		if(commands)
			free(commands);
	}

	int flag_bkgrd = 0; // indicate should be excuted in backgroud
	int count_cmd = 0;//current cmd read from the buffer
	int count_token = 0;//current tokens in one cmd
	char ** tokens = NULL;
	int max_num_tokens = 0;
	while(1){
		// jump all the space before a token
		if(*p == ' '){
			p++;
			continue;
		}

		if(* p == '&'){
			flag_bkgrd = 1;
			p++;
			continue;
		}
		// support for pipe
		if(*p == '|' | *p == '\0'){
			//for fear that the command end with'|'
			if(count_token ==0){
				printf("command not corrent, input angaion\n");
				Tuple tup_ret;
				tup_ret.flag = -1;
				return tup_ret;
			}

			//save the last the cmd
			*(tokens + count_token) = NULL;
			*(commands + count_cmd) = tokens;
			count_cmd ++;
			
			if(count_cmd-1 >= num_commands){
				num_commands += NUM_COMMANDS_INC;
				commands = (char ***)realloc(commands,num_commands*sizeof(char ***));
				
				if(commands == NULL){
					printf("commands reallocated space failed\n");
					exit(-3);
				}
			}

			
			//!!!if read the end of the buffer, this is the ony exit of while loop
			if(*p == '\0'){
				break;
			}
			// that must be a pipe! start a new cmd
			else{
				count_token = 0;
				p++;
				continue;
			}
		}
		
		//start of one cmd
		if(count_token ==0){	
			max_num_tokens = NUM_TOKENS;
			tokens = (char **)malloc(max_num_tokens*sizeof(char *));
			if(tokens == NULL){
				printf("tokens allocated faild\n");
				exit(-3);
			}
		}
		
		
		//save all the tokens
		//start a token
		count_token += 1;
		if(count_token - 1 >= max_num_tokens){//leave the last one to mark the end of tokens
			max_num_tokens += NUM_TOKENS_INC;
			tokens = (char **)realloc(tokens, max_num_tokens*sizeof(char*));
			if(!tokens){
				printf("reallocation for tokens failed");
				exit(-3);
			}
		        	
		}
		
		//store the token context
		int token_max_len = TOKEN_LEN;
		char * token = (char *)malloc(token_max_len*sizeof(char));
		if(token == NULL){
			printf("token allocated failed");
			exit(-4);
		}
		int len_token = 0;
		
		if(*p == '<' | *p== '>'){
			token[0] = *p;
			token[1] = '\0';
			*(tokens + count_token - 1) = token;
			p++;
			continue;
		}		
	
		//main part of read a token
		while(*p != '\0'&& *p != ' ' && *p!= '|' && *p != '<' && *p != '>' && *p!= '&'){
			*(token + len_token) = *p;
			p++;
			len_token++;
			if(len_token >= token_max_len){
				token_max_len += TOKEN_LEN_INC;
				token = (char*)realloc(token, token_max_len*sizeof(char));
				if(!token){
					printf("reallocation for one token failed\n");
					exit(-4);
				}
			}
		}
		// mark the end of a token
		*(token + len_token) = '\0';
		*(tokens + count_token -1) = token;
	}

	///printf("cmd count is %d\n", count_cmd);
	//mark end of all the cmds
	commands[count_cmd] = NULL;	
	Tuple ret_tuple;
	ret_tuple.flag = flag_bkgrd;
	ret_tuple.cmds = commands;
	return ret_tuple;
}






/* name: execute_commands 
** usage: run a single command
** input: command args : eg , pipfd, pipdirect, 0 read 1 write
** output:
** return: if can executed corrcectly, returns 1  
*/
int execute_commands(char ** tokens){
	int i = 0;
	//printf("cmd:%sexcuted\n",tokens[0]);
	char ** ptoken;
	// used for redirecting '<' and '>'
	for(ptoken=tokens;*ptoken!=NULL;ptoken++){
//		fprintf(stderr,"%s",*ptoken);

		if(strcmp(*ptoken,"<") == 0 ){
			if(*(ptoken+1) != NULL && *(ptoken+2) == NULL){
			// if the '<' occurs this command should have fixed format, file name and END
				int fd = open(*(ptoken+1),O_RDONLY);	
				if(fd <0){
					perror("open1");
					exit(-5);
				}
				if(dup2(fd,0) <0 ){
					perror("dup2");
					exit(-5);
				}
				*ptoken = NULL;	
			}
			else{
				fprintf(stderr,"error format, try again\n");
				exit(-5);
			}
		}
		else if(strcmp(*ptoken,">") == 0){
			if(*(ptoken+1) != NULL && *(ptoken+2) == NULL){
				int fd = open(*(ptoken+1), O_CREAT|O_TRUNC|O_WRONLY,0644);	
				if(fd < 0 ){
					perror("open2");
					exit(-5);
				}
				if(dup2(fd,1) <0){
					perror("dup2");
					exit(-5);
				}
				*ptoken = NULL;	
			}
			//also, the '>' commands also should have fixed format
			else{
				fprintf(stderr,"error format\n");
				exit(-5);
			}
	
		}
		


	
	}
//	fprintf(stderr, "\n");

	int ret2;
	ret2 = execvp(tokens[0],tokens);
	if(ret2 == -1){
				perror("execv");
				//child proess terminates
				exit(-5);
	}
}





/* name: run this line 
** usage: run multiple commands which was splitted by pipeline
** input: several commands % the former command 's result will be the input of the latter command
** output:
** return: number of commands which have been excuted.
*/

int run_this_line(char *** cmds){
	//whther it will be excuted in background
	char *** pcmd = cmds;

	int command_count = 0;
	
	int ret;
	int *pipefd;
	int *old_pipefd;

	

	int i;
	int myout;

	for(pcmd = cmds;*pcmd != NULL;pcmd ++){
		command_count++;
	}

	int * pipelist = (int *)malloc(command_count*sizeof(int)*2);


	command_count = 0;
	 
	for(pcmd = cmds, pipefd = pipelist;*pcmd != NULL;pcmd ++,pipefd +=2){
	// try 1 pipeline
		//if it is not the last part
		command_count ++; 	

		if(command_count >1){
			old_pipefd = pipefd -2;
		}
		
		
		i = pipe(pipefd);
		if(i < 0){
			perror("pipe"); 
			myfree(cmds);
			exit(-6);
		}
		ret = fork();
		if(ret == 0){

			// use the output of last cmd as input
			if(command_count > 1){
				if(dup2(old_pipefd[0],0) < 0 ){
					perror("du2_1");
					exit(-5);
				}	
				//fprintf(myout,"in cmd %dinput file is%d\n", command_count,old_pipefd[1]);
			}

			//write the result to next cmd
			if(*(pcmd+1) !=NULL){
				if(dup2(pipefd[1],1) < 0){
					perror("dup2_2");
					exit(-5);
				}
			//	printf("outfrom cmd %d is %d", command_count,pipefd[0]);
			}
			int *p;
			for(p = pipefd;p>= pipelist;p-=2){
			close(p[0]);
			close(p[1]);
			}

			char **cmd = *pcmd; 
			execute_commands(cmd);		
		}
		else{
			// if closed here, there can't used anymore
			//close(pipefd[0]);
			//close(pipefd[1]);
		//	printf("parent process%d\n", getpid());
		}
	}
	
	int j = 0, status;
	for(; j<command_count;j++){	
	// for the parent process
//			close(pipefd_set[j][0]);
			close(*(pipelist+2*j));//add the number to pipefd
			close(*(pipelist+2*j+1));
			wait(&status);
		//	printf("parent %d completd,pid = %d\n", j,getpid());
			if(WEXITSTATUS(status)){
			//	fprintf(stderr, "Abnormal exit\n");
			}
	}
	free(pipelist);
	return command_count;
}



/* name: 
** usage: 
** input: 
** output:
** return: 
*/
int myfree(char ***cmds){
	char ***pcmd;
	char **ptoken;
	for(pcmd = cmds; *pcmd !=NULL; pcmd++){
		for(ptoken = *pcmd; *ptoken != NULL; ptoken++){

			if(*ptoken)
				free(*ptoken);
		}
		if(*pcmd)
			free(*pcmd);
	}
	if(cmds)
		free(cmds);
}


/* name: 
** usage: 
** input: 
** output:
** return: 
*/
