#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define ARGS_END  0
#define ARGS_FUNC 1
#define ARGS_ADD  2
#define ARGS_MULTI 3


typedef void (*print_fp)(const char *);

void test_args_api(int name, ...)
{
	va_list ap;
	print_fp fp = NULL;
	int add = 0;
	int mul = 1;

	for (	va_start(ap, name);
			name != ARGS_END;
			name = va_arg(ap, int)) {
		switch (name) {
		case ARGS_FUNC:
			fp = va_arg(ap, print_fp);
			break;
		case ARGS_ADD:
			add += va_arg(ap, int);
			break;
		case ARGS_MULTI:
			mul *= va_arg(ap, int);
			break;
		default:
			break;
		}
	}

	if (fp) fp("call\n");
	printf("add: %d\n", add);
	printf("mul: %d\n", mul);
}

int main(int argc, char **argv)
{
	test_args_api(
ARGS_ADD, 1,
ARGS_ADD, 2,
ARGS_ADD, 3,
ARGS_END);


}
