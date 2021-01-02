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

char const *const keys[] = {
	"byr",
	"iyr",
	"eyr",
	"hgt",
	"hcl",
	"ecl",
	"pid",
	"cid"
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
		for (j = 0; j < 7; j++) {
			//The cid field is optional, which is why this loop only
			//goes up to 7
			if (passports[i].fields[j] == NULL) {
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
