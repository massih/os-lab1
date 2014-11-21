#include <stdio.h>
#include <stdlib.h>
int main()
{
	// int n = 11;
	// int* ptr = &n;
	// int **pptr = &ptr;
	// printf("address of n = 0x %x \n", *pptr);
	// printf("value of n = %d\n", **pptr);
	// printf("value of *PTR = %d\n", *ptr);
	// printf("value of PTR = %x\n", ptr);

	// n = n+11;
	// printf("2 - address of n = 0x %x \n", *pptr);
	// printf("2 - value of n = %d\n", **pptr);
	// printf("2 - value of *PTR = %d\n", *ptr);
	// printf("2 - value of PTR = %x\n", ptr);
	char str[5] = "Nope";
	char *s = str;
	int i = 0;
	while(i <= 4){
		printf ("%s\n", str++);
		i++;
	}


}
