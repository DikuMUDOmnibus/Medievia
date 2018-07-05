#include <stdio.h>
#include <ctype.h>

main()
{
int ch;
   for(ch=0;ch<=0x7f;ch++){
	printf("%.2x ", ch);
	printf(" %c", isprint(ch) ? ch : '\0');
	printf("%3s",ispunct(ch) ? "PU" : "");
	printf("\n");
   }


}
