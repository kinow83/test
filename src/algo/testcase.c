#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 

int check_exe_file(const char* exe)
{
	struct stat st;
	if (stat(exe, &st)) {
		return -1;
	}
	if (st.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH )) {
		return 0;
	}
	return -1;
}

char* get_input(const char* input)
{
	FILE *fp = fopen(input, "r");
	char line[1024];
	char preline[1024];
	int len, c = 0;
//	printf("get_input = %s\n", input);

	while (fgets(line, sizeof(line), fp)) {
		c++;
		if (line[0] == '\0' || line[0] == '\r' || line[0] == '\n') {
			break;
		}
		strcpy(preline, line);
	}
	len = strlen(preline);
	if (line[len-1] == '\n') preline[len-1] = 0;
	else if (line[len-2] == '\r') preline[len-2] = 0;

	fclose(fp);
	return strdup(preline);
}

int main(int argc, char **argv)
{
	char cwd[1024];
	char pdir[1024];
	char pname[1024];
	char input[1024];
	DIR *dir;
	struct dirent *entry;

	if (argc != 2 || argv[1] == 0) {
		printf("usage: exefile");
		exit(0);
	}
	
	getcwd(cwd, sizeof(cwd));
	snprintf(pdir, sizeof(pdir), "%s/%s", cwd, argv[1]);
	snprintf(pname, sizeof(pname), "%s/%s", pdir, argv[1]);

	if (check_exe_file(pname)) {
		fprintf(stderr, "%s is not exe\n", pname);
		exit(1);
	}

	dir = opendir(pdir);
	if (!dir) {
		fprintf(stderr, "%s program not found\n", cwd);
		exit(1);
	}
	while (entry = readdir(dir)) {
		if (strncmp(entry->d_name, "input", strlen("input")) == 0) {
			snprintf(input, sizeof(input), "%s/%s", pdir, entry->d_name);
			char *answer = get_input(input);
			if (answer) {
				char cmd[1024], output[1024];
				snprintf(cmd, sizeof(cmd), "%s < %s", pname, input);
				FILE *fp = popen(cmd, "r");
				if (!fp || !fgets(output, sizeof(output), fp)) {
					fprintf(stderr, "cmd: '%s' fail.\n", cmd);
					free(answer);
					if (fp) fclose(fp);
					continue;
				}
				//printf("output: %s, answer: %s\n", output, answer);
				if (strncmp(answer, output, strlen(answer))) {
					printf("mismatch!!! input=%s, answer=%s, but output=%s\n", 
						input, answer, output);
				}
				fclose(fp);
				free(answer);
			}
		}
	}


	closedir(dir);

	return 0;
}
