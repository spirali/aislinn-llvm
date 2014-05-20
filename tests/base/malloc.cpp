#include <stdlib.h>

int main(int argc, char **argv) {
	if (argc != 2) {
		return 2;
	}
	size_t size = atol(argv[1]);
	void *mem = malloc(size);
	return mem == NULL;
}
