#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <vector.h>
#include <string.h>
#include <fast_fail.h>
#include <util.h>

#define SPACE " \t\v"
#define LNEND "\r\n"
#define DIGITS "0123456789"
#define HEXDIGITS "abcdef"

#define key_ids \
	X(byr), \
	X(iyr), \
	X(eyr), \
	X(hgt), \
	X(hcl), \
	X(ecl), \
	X(pid), \
	X(cid)

char const *const keys[] = {
#define X(x) #x
key_ids
#undef X
};

enum {
#define X(x) x
key_ids
#undef X
};

//Returns index on success, -1 on fail
int key_to_idx(char const *key) {
	int i;
	for (i = 0; i < sizeof(keys)/sizeof(*keys); i++) {
		if (!strcmp(key, keys[i])) return i;
	}
	return -1;
}

typedef struct {
	char const* fields[sizeof(keys)/sizeof(*keys)];
} passport;

void passport_init(passport *p) {
	memset(p, 0, sizeof(passport));
}

int validate_num(char const *str, int min, int max) {
	if (str == NULL) return 0;
	
	int yr;
	int rc = sscanf(str, "%d", &yr);
	if (rc < 1) return 0;
	else if (yr < min) return 0;
	else if (yr > max) return 0;
	else return 1;
}

int validate_byr(char const *str) {return validate_num(str, 1920, 2002);}
int validate_iyr(char const *str) {return validate_num(str, 2010, 2020);}
int validate_eyr(char const *str) {return validate_num(str, 2020, 2030);}

int validate_hgt(char const *str) {
	if (str == NULL) return 0;
	
	int numlen = strspn(str, DIGITS);
	
	char const *unit = str + numlen;
	if (!strcmp(unit, "cm")) {
		return validate_num(str, 150, 193);
	} else if (!strcmp(unit, "in")) {
		return validate_num(str, 59, 76);
	} else {
		return 0;
	}
}

int validate_hcl(char const *str) {
	if (str == NULL) return 0;
	
	if (*str != '#') return 0;
	if (strspn(str + 1, DIGITS HEXDIGITS) != 6) return 0;
	
	return 1;
}

int validate_ecl(char const *str) {
	static char const *const cols[] = {
		"amb", "blu", "brn", "gry", "grn", "hzl", "oth"
	};
	
	if (str == NULL) return 0;
	
	int i;
	for (i = 0; i < sizeof(cols)/sizeof(*cols); i++) {
		if (!strcmp(str, cols[i])) return 1;
	}
	
	return 0;
}

int validate_pid(char const *str) {
	if (str == NULL) return 0;
	
	return strspn(str, DIGITS) == 9;
}

int validate_cid(char const *str) {
	return 1;
}

typedef int validator(char const *);

validator *const validators[] = {
#define X(x) &validate_##x
key_ids
#undef X
};

int main() {
	VECTOR_DECL(passport, passports);
	vector_init(passports);
	
	FILE *fp = fopen("input.txt", "rb");
	if (!fp) {
		perror("Could not open input.txt");
		FAST_FAIL("Cannot continue");
	}
	
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	rewind(fp);
	
	char *data = malloc(len+1);
	if (!data) FAST_FAIL("out of memory");
	fread(data, len, 1, fp);
	data[len] = 0; //NUL-temrinate for safety
	
	int pos = 0;
	
	//Some unnecessary copying, but my only goal was to have simple code
	passport cur;
	passport_init(&cur);
	int cur_vld = 0; //Not required by problem, but I don't consider the
	                 //current passport valid until we've actually seen
	                 //at least one key
	
	char *str = data;
	while (*str) {
		//Skip past all space characters
		int skip = strspn(str, SPACE);
		str += skip;
		
		//Read until the next ':' character to get the key. 
		//Special case: if this is a blank line, this is the end of the
		//current passport
		if ((*str == '\n' || *str == '\r') && cur_vld) {
			vector_push(passports, cur); //Unnecessary copy, but w/e
			passport_init(&cur);
			cur_vld = 0;
			
			//Skip past all whitespace
			str += strspn(str, SPACE LNEND);
		}
		
		char *key = str;
		int keylen = strcspn(str, ":");
		str[keylen] = 0; //Terminate key string by clobbering ':' char
		int idx = key_to_idx(key);
		str += keylen + 1;
		
		//Skip spaces after ':' character
		str += strspn(str, SPACE);
		//Read until the next ' ' or '\n' character to get the value
		int vallen = strcspn(str, SPACE LNEND);
		char *val = str;
		char *one_past_end_val = str + vallen; //Can't clobber the next
		//character with a NUL just yet.
		
		str += vallen;
		//Now the ugly business: need to skip all spaces and only the
		//first line terminator (if it is there). In a regex, this would
		//be '[ \t\v]*(\r|(\r\n)|\n)?[ \t\v]*
		str += strspn(str, SPACE);
		if (*str == '\r') str++;
		if (*str == '\n') str++;
		str += strspn(str, SPACE);
		
		
		//Now that we have advanced str to the proper place, we can now
		//put the terminating NUL on our value string
		*one_past_end_val = 0;
		
		//Write this KV pair into the current passport
		if (idx < 0) {
			printf("Warning: unknown key [%s]\n", data + pos);
		} else {
			cur.fields[idx] = val;
			cur_vld = 1;
		}
	}
	
	//Ugly corner case: the last passport may not be terminated with a blank
	//line. So if the current passport is valid, push it to the vector
	if (cur_vld) {
		vector_push(passports, cur);
	}
	
	int num_bad = 0;
	
	int i;
	for (i = 0; i < passports_len; i++) {
		int j;
		for (j = 0; j < sizeof(keys)/sizeof(*keys); j++) {
			//The cid field is optional, which is why this loop only
			//goes up to 7
			if (!validators[j](passports[i].fields[j])) {
				num_bad++;
				break;
			}
		}
	}
	
	printf("Out of %d passports, %d are valid and %d are invalid\n", passports_len, passports_len - num_bad, num_bad);
	
	vector_free(passports);
	free(data);
	fclose(fp);
	
	return 0;
}
