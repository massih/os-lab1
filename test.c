#include <stdio.h>
#include <stdlib.h>
int main()
{
	printf("test\n");
	const char* s = getenv("PATH");
	printf("PATH :%s\n",(s!=NULL)? s : "getenv returned NULL");
	printf("end test\n");
}
