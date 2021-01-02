#include <stdio.h>
#include <stdlib.h>
#include <vector.h>
#include <fast_fail.h>
#include <util.h>

typedef struct {
	int min, max;
	char c;
} rule;

typedef char password[81]; //Is it safe to assume no password exceeds 80 chars?

int main() {
	VECTOR_DECL(rule, rules);
	vector_init(rules);
	
	VECTOR_DECL(password, passwords);
	vector_init(passwords);
	
	FILE *fp = fopen("input.txt", "rb");
	if (!fp) {
		perror("Could not open input.txt");
		FAST_FAIL("Cannot continue");
	}
	
	while (!feof(fp)) {
		rule *r = vector_lengthen(rules);
		password *p = vector_lengthen(passwords);
		int rc = fscanf(fp, "%d-%d %c: %s", &r->min, &r->max, &r->c, *p);
		if (rc < 4) {
			if (feof(fp)) {
				if (rc > 0) {
					FAST_FAIL("Read an incomplete record");
				}
				vector_pop(rules);
			} else if (ferror(fp)) {
				perror("Error reading input.txt");
				FAST_FAIL("Cannot continue");
			} else {
				FAST_FAIL("Bizarre error");
			}
		}
	}
	
	int num_valid = 0, num_bad = 0;
	
	int i;
	for (i = 0; i < rules_len; i++) {
		char c = rules[i].c;
		int count = 0;
		char *p = passwords[i];
		while(*p) {
			if (*p++ == c) count++;
		}
		
		if (count < rules[i].min || count > rules[i].max) {
			//printf("Bad password: %s\n", passwords[i]);
			num_bad++;
		} else {
			num_valid++;
		}
	}
	
	printf("There are %d valid passwords and %d bad ones\n", num_valid, num_bad);
	
	vector_free(passwords);
	vector_free(rules);
	fclose(fp);
}
