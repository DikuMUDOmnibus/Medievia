#include <fcntl.h> 
#include <stdio.h> 
#include <sys/time.h> 
#include <sys/resource.h>
#include <sys/types.h>


void pr_limits(char *text, int resource)
{

struct rlimit limit;

   if(getrlimit(resource, &limit)<0){
	printf("getrlimit error for %s",text);
	exit(0);
   }
   printf("%-14s  ", text);
   if(limit.rlim_cur==RLIM_INFINITY)
	printf("(infinite)");
   else
	printf("%10ld   ", limit.rlim_cur);
   if(limit.rlim_max==RLIM_INFINITY)
	printf("(infinite)\n");
   else
	printf("%10ld\n", limit.rlim_max);

}

main()
{
   pr_limits("RLIMIT_CORE",RLIMIT_CORE);
   pr_limits("RLIMIT_CPU",RLIMIT_CPU);
   pr_limits("RLIMIT_DATA",RLIMIT_DATA);
   pr_limits("RLIMIT_FSIZE",RLIMIT_FSIZE);
   pr_limits("RLIMIT_NOFILE",RLIMIT_NOFILE);
   pr_limits("RLIMIT_RSS",RLIMIT_RSS);
   pr_limits("RLIMIT_STACK",RLIMIT_STACK);
   exit(0);
}

