
#include <stdlib.h>

int main() {
	void *mem;
	for (int i = 0; i < 10000; ++i) {
		mem = malloc(i * 100);
		if (mem == NULL) {
			return 1;
		}
		free(mem);
	}

	for (int i = 0; i < 10000; ++i) {
		mem = malloc(480000);
		if (mem == NULL) {
			return 1;
		}
		free(mem);
	}
	return 0;
}
