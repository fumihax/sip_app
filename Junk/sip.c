
#include <sys/time.h>
#include <osip2/osip.h>

int main()
{

	int i;
	osip_t *osip;

 	i = osip_init(&osip);
	if (i!=0) {
		fprintf(stderr, "ERROR!!\n");
		return -1;
	}
	return 0;
}
