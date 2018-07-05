#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

main()
{
	int	fds[300];
	int	i;

	struct rlimit	rlp;

	printf("limit: %d\n", getdtablesize());
	getrlimit(RLIMIT_NOFILE, &rlp);
	printf("soft: %d\n", rlp.rlim_cur);
	printf("hard: %d\n", rlp.rlim_max);
	rlp.rlim_cur = rlp.rlim_max;

	printf("\nsetting rlimit...\n");
	setrlimit(RLIMIT_NOFILE, &rlp);
	printf("limit: %d\n", getdtablesize());
	printf("soft: %d\n", rlp.rlim_cur);
	printf("hard: %d\n", rlp.rlim_max);

/*
	printf("hit a key: "); getchar();
	for (i = 0 ; i < 200 ; i++)
	{
		printf("%d\n", i);

		fds[i] = open("/dev/tty", O_RDONLY);
		if (fds[i] < 0)
		{
			perror("foo1");
			exit(1);
		}
	}
*/
}
