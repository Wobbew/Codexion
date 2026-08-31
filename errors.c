
#include "codexion.h"

bool	input_error(void)
{
	fprintf(stderr, "Usage: codexion <num_coders> <time_to_burnout> ");
	fprintf(stderr, "<time_to_compile> <time_to_debug> <time_to_refactor> ");
	fprintf(stderr, "<num_compiles_required> <dongle_cooldown> <scheduler>\n");
	return (false);
}

bool	input_error_int(void)
{
	fprintf(stderr, "Invalid integer value provided.\n");
	return (false);
}

bool	input_error_schedul(void)
{
	fprintf(stderr, "Invalid scheduler value. Please use 'fifo' or 'edf'.\n");
	return (false);
}