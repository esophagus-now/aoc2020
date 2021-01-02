#include <util.h>

int intcomp(void const *a, void const *b) {
	int const *pa = (int const *)a;
	int const *pb = (int const *)b;
	
	return *pa - *pb;
}

int charcomp(void const *a, void const *b) {
	char const *pa = (char const *)a;
	char const *pb = (char const *)b;
	
	return *pa - *pb;
}
