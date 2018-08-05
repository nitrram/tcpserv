#include "c_io.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int print_mem_stats(char *out, size_t out_len) {

	FILE * fp;
	char * line = NULL;
	size_t n = 0;
	ssize_t br;

	//read the 1st line at stats in order to get the cpu info
	fp = fopen("/proc/meminfo", "r");
	if (fp == NULL)
		return -1;

	int map[5] = {0,0,0,0,0};
	for(size_t i=0; i < 5; i++) {
		if((br = getline(&line, &n, fp)) != -1) {
			char *start = line;
			line = line + br - 5; /* fetch it backwards skipping " kB\0"*/
			int reading = 0;
			while(*line >= 0x30 && *line <= 0x39) {
				reading++;
				line--;
			}

			if(reading) {
				char *stat = (char*)(malloc(sizeof(char) * reading +1));
				memcpy(stat, line+1, reading+1);
				map[i] = atoi(stat);
				free(stat);
				reading = 0;
			}

			line = start;
		}
	}

	// used = total - free - buffers - cached
	int nout = snprintf(out, out_len, "%.2f [MB]\n",(float)(map[0]-map[1]-map[3]-map[4]) / 1024.f);
	fclose(fp);
	if(line)
		free(line);


	return nout;
}

int print_cpu_stats(char *out, size_t out_len) {

	int result = 0;
	int prev_map[10];

	if((result = print_cpu_stage0(prev_map)))
		return result;

	// delay 400ms
	usleep(400000);

	return print_cpu_stage1(prev_map, out, out_len);
}


int print_cpu_stage0(int *prev_map) {

	return get_cpu_stats(prev_map);
}

int print_cpu_stage1(const int *const prev_map, char *out, size_t out_len) {
	int result = 0;
	int map[10];

	if((result = get_cpu_stats(map)))
		return result;

	// idle - iowait
	int prev_idle = prev_map[3] + prev_map[4];
	int idle = map[3] + map[4];

	// user + nice + system + irq + softirq + steal
	int prev_non_idle = prev_map[0] + prev_map[1] + prev_map[2] + prev_map[5] + prev_map[6] + prev_map[7];
	int non_idle = map[0] + map[1] + map[2] + map[5] + map[6] + map[7];

	// differences: total - prevTotal, resp. idle - prevIdle
	int totald = (idle + non_idle) - (prev_idle + prev_non_idle);
	int idled = (idle - prev_idle);

	int n = snprintf(out, out_len, "%.8f [%%]\n", (float)(totald-idled) / (float)(totald)*100.0f);

	return n;
}



// the arg. map is an array of the specific number of int values representing particular cpu stat
// values, namely: user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice
int get_cpu_stats(int * const map) {
	FILE * fp;
	char * line = NULL;
	size_t n = 0;
	ssize_t br;

	// read the 1st line at stats in order to get the cpu info
	fp = fopen("/proc/stat", "r");
	if (fp == NULL)
		return -1;

	if ((br = getline(&line, &n, fp)) != -1) {
		char *start = line;
		size_t map_index = 0;
		int reading = 0;
		// grep numbers out of the line into the map
		while(*line != '\0') {
			if(*line >= 0x30 && *line <= 0x39) {
				reading++;
			} else if(reading)
			{
				char *stat = (char*)(malloc(sizeof(char) * reading +1));
				memcpy(stat, line-reading, reading+1);
				map[map_index++] = atoi(stat);
				free(stat);
				reading = 0;
			}

			line++;
		}

		// finalize the loop reading the last number
		if(reading) {
			char *stat = (char*)(malloc(sizeof(char) * reading +1));
			memcpy(stat, line-reading, reading+1);
			map[map_index++] = atoi(stat);
			free(stat);
		}

		// set it back (so that the free() doesn't crash)
		line = start;
	}

	fclose(fp);
	if (line)
		free(line);

	return 0;
}
