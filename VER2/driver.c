#include "PTree.h"
#include<stdio.h>
#include<stdlib.h>
int main(int argc, char *argv[])
{
	int N, ret;
	printf("Enter an Integer Number N\n");
	scanf("%d",&N);
	ret=createProcessTree(N);
	if(ret==-1)
		printf("value of N too small\n");
	else if(ret==-2)
		printf("value of N too large\n");
	else if(ret==0)
		printf("Successfully Executed\n");
	else
		printf("Undefined result\n");
	return 0;
}
