#include <stdio.h>
#include <stdlib.h>

int main(int nchar, char *schar[]) {

	char *map,map2[3] = { "ahoj", "hello", "turbo"};

	int *m_ptr = (int*)malloc(sizeof(int)*10);
	printf("%d\n", sizeof(m_ptr));


	//strcat(map[0], 'p');
	for(int i=0; i < 3; ++i)
		printf("%s\n", map[i]);


	return 0;
}
