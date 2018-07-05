#include <stdio.h>
#include <stdlib.h>

main()
{
int price, dist,x,o,s;
	while(1){
		printf("Enter in Distance: ");
		scanf("%d",&dist);
		printf("Enter in Worth: ");
		scanf("%d",&price);
		printf("Enter in the stock 0,100: ");
		scanf("%d",&s);
		o=price;
		o+=price*dist/5;
		s=100-s;
		o+=o*(s/4);		
	    printf("\nSold for %d.\n\n",o);
	}
}
