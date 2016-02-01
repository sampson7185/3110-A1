#include <unistd.h>

int main(int argc, char const *argv[])
{
	int ret;

	ret = execl("/bin/ps", "ps", "-l", (char *)0);
	return 0;
}