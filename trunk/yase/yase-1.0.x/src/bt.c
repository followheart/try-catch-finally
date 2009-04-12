/* Dibyendu */
/* 16 April 2001 */
#include "stdio.h"
#include "stdlib.h"

#define BYTESIZE	8
#define BITS(n)		(sizeof(n)*BYTESIZE)

int countbits(unsigned char ch)
{
	unsigned mask = 1;
	int count = 0;
	int i;
	unsigned u = ch;

	for (i = 0, mask = 1; i < BITS(ch); i++) {
		if (mask & u)
			count++;
		mask <<= 1;
	}
	return count;
}

int main()
{
	int i, j;

	printf("static unsigned bitcount[] = {\n");
	for (i = 0, j = 0; i < 256; i++) {
		int n = countbits((unsigned char)i);
		if (i > 0)
			putc(',', stdout);
		if (++j > 30) {
			putc('\n', stdout);
			j = 0;
		}
		printf("%d", n);
			
	}
	printf("};\n");
	return 0;
}
	
