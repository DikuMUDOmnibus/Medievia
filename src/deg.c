#include <math.h>
#include <stdio.h>

	
main()
{
 int x1, x2, y1, y2, fin, dist;
 
  printf("Enter: x1 y1 x2 y2:");
  scanf("%d %d %d %d",&x1, &y1, &x2, &y2);
  fin = deg(x1, y1, x2, y2, &dist);
  printf("Answer: %d Dist: %d\r\n", fin, dist);
  
 }	
	
