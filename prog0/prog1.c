#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUFSIZE 1024

int cmp(const void *p1, const void *p2)
{
	const char **a = (const char **) p1;
	const char **b = (const char **) p2;
	return strcmp(*a, *b);

}

int main()
{
	char buf[BUFSIZE + 1];
	char **str;
	int i, n, len;
	printf("prompts a user for an integer n\n");
	fflush(stdout);
	fgets(buf, BUFSIZE, stdin);
	n = atoi(buf);
	str = malloc(n * sizeof(char));
	for (i = 0; i < n; i++)
	{
		printf("Enter string # %d.\n", i + 1);
		fgets(buf, BUFSIZE - 1, stdin);
		len = strlen(buf);
		if (buf[len - 1] == '\n')
		{
			buf[len - 1] = 0;
		}
		str[i] = malloc((strlen(buf) + 1) * sizeof(char));
		strcpy(str[i], buf);
	}

	qsort(str, n, sizeof(char*), cmp);
	printf("Sorted strings:\n");
	for (i = 0; i < n; i++)
	{
		printf("%s\n", str[i]);
	}
}