#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void child1();

int main ()
{
    int parentID = getpid();
    pid_t pid;
    int status;
	int count = 0;
		
    printf("Main Process ID : %d\n\n",parentID );
	
	while (count!=3)
	{
		pid = fork();
		if (pid < 0) 
			printf("fork error");
		else if (!pid)
		{
			++count;		
			printf("Fork %d. I'm the child %d, my parent is %d\n", count, getpid(), parentID);
			parentID = getpid();
		}
		else 
		{
			if (count == 1)
			{
				pid = fork();
				if (pid < 0)
					printf("fork error");
				else if (!pid)
				{
					++count;
					printf("Fork %d. I'm the child %d, my parent is %d\n", count, getpid(), parentID);
					parentID = getpid();
				}
				else 
				{
					wait(&status);
					break;
				}
				continue;
			}

			wait(&status);
			break;
		}
	}
    return 0;
}

