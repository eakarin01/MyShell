#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_COMMAND 1000


void consumenewline()
{
	int c = getchar();
	while(c!='\n'){}
}

char** splitArg(char* cmd)
{
	char **wsplit = (char**)malloc(strlen(cmd)*sizeof(char*));
	char *word = (char*)malloc(strlen(cmd)*sizeof(char));
	int count=0;
	//printf("%s[%d]\n",cmd,strlen(cmd));
	//ignore starting with space
	int j=0;
	while(cmd[j]==' ')
		j++;
	//split word
	int k=0;
	char *tmp;
	for(int i=j;i<strlen(cmd)+1;i++)
	{
		if (cmd[i]!=' ' && cmd[i]!='\0')
			word[k++]=cmd[i];
		else
		{
			if (cmd[i]==' ')
			{
				while(cmd[i]==' ')
					i++;
				i--;
			}
			word[k]='\0';
			k=0;
			//printf("%s\n",word);
			tmp = (char*)malloc(strlen(cmd)*sizeof(char));
			strcpy(tmp,word);
			wsplit[count++]=tmp;
			if (cmd[i+1]=='\0')
				break;
		}
	}
	wsplit[count++] = 0;
	/*for(int i=0;i<count;i++)
	{
		printf("%s\n",wsplit[i]);
	}*/
	return wsplit;
}	


int getConcurrent(char **cmd)
{
	int i=0;
	while(cmd[i]!=0)
	{
		if(!strcmp(cmd[i],";"))
			return i;
		i++;
	}
	return -1;
}

int checkQuit(char **cmd)
{
	int i=0;
	while(cmd[i]!=0)
	{
		if(!strcmp(cmd[i],"quit"))
			return 1;
		i++;
	}
	return 0;
}

void execute(char** cmd)
{
	/* check concurrent */
	int idx = getConcurrent(cmd);
	cmd[idx] = 0;

		// child exec
		if(!fork())
		{
			if (execvp(cmd[0],cmd)<0)
				printf("\e[1;31mNo command : %s\e[1;36m\n",cmd[0]);
			exit(0);
		}
		// check if have concurrent
		if (idx!=-1)
		{
			cmd=&cmd[idx+1];
			execute(cmd);
		}
		// parent exec
		wait(0);
}



void main()
{
	char *raw_cmd = (char*)malloc(MAX_COMMAND*sizeof(char));
	char **cmd;
	system("clear");
	while(1)
	{
		memset(raw_cmd,0,MAX_COMMAND);
		printf("\e[1;33mshell>\e[1;37m");
		scanf("%[^\n]",raw_cmd);
		consumenewline();
		int len = strlen(raw_cmd);
		printf("\e[1;36m\n");
		if (strlen(raw_cmd)!=0)
		{
			cmd = splitArg(raw_cmd);
			/*int i=0;
			while(cmd[i]!=0)
			{
				printf("[%s]\n",cmd[i++]);
			}*/
			if(checkQuit(cmd))
			{
				/* exit program */
				exit(0);
			}
			else if(!strcmp(cmd[0],"shell"))
			{
				// shell [batch file] 
				if(!fork())
				{
					int fd;
					if ((fd = open(cmd[1], O_RDONLY)) < 0) { 
						perror(cmd[1]); // open failed 
						exit(1);
					    }
					memset(raw_cmd,0,MAX_COMMAND);
					read(fd,raw_cmd,MAX_COMMAND);
					char *line;
					char **batchcmd;
					while( (line = strsep(&raw_cmd,"\n")) != NULL )
					{
						batchcmd = splitArg(line);
						//ignore blank command
						if (batchcmd[0][0]!=0)
						{
							execute(batchcmd);
						}
					}
					//printf("%s",raw_cmd);
				}
				wait(0);
			}
			else
			{
				/* interactive mode */
				execute(cmd);
			}
		}
	}
}