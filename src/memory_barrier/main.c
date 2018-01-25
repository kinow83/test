#include <string.h>
#include <unistd.h>
#include "mm.h"

int main(void)
{
	int c = 3;

	func(4);

	c = func2(c);

	return 0;
}
