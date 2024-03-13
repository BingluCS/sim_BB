#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <sys/time.h>


int Prefetch(const char *filename);

int Read_file_cache(const char *filename);

int Write_file_cache(const char *filename);

char *demotion();

int show_function(const char *filename);