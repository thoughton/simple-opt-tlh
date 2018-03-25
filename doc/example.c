#include "../simple-opt.h"

int main(int argc, char **argv)
{
	const char *set[] = { "str_a", "str_b", NULL };

	/* array containing all options and their types / attributes */
	struct simple_opt options[] = {
		{ SIMPLE_OPT_FLAG, 'h', "help", false,
			"print this help message and exit" },
		{ SIMPLE_OPT_BOOL, 'b', "bool", false,
			"(optionally) takes a boolean arg!" },
		{ SIMPLE_OPT_INT, '\0', "int", true,
			"requires an integer. has no short_name!" },
		{ SIMPLE_OPT_UNSIGNED, 'u', "uns", true,
			"this one has a custom_arg_string. normally it would say" 
				" \"UNSIGNED\" rather than \"NON-NEG-INT\"",
			"NON-NEG-INT" },
		{ SIMPLE_OPT_DOUBLE, 'd', "double", true,
			"a floating point number" },
		{ SIMPLE_OPT_STRING, 's', NULL, true,
			"this one doesn't have a long_name version" },
		{ SIMPLE_OPT_STRING_SET, '\0', "set-choice", true,
			"a choice of one string from a NULL-terminated array",
			"(str_a|str_b)", set },
		{ SIMPLE_OPT_END },
	};

	/* contains an enum for identifying simple_opt_parse's return value, an
	 * array of the cli arguments which were not parsed as options, and
	 * information relevant for error handling */
	struct simple_opt_result result;

	int i;

	result = simple_opt_parse(argc, argv, options);

	/* catch any errors and print a default error message. you can do this bit
	 * yourself, if you'd like more control of the output */
	if (result.result_type != SIMPLE_OPT_RESULT_SUCCESS) {
		simple_opt_print_error(stderr, argv[0], result);
		return 1;
	}

	/* if the help flag was passed, print usage */
	if (options[0].was_seen) {
		simple_opt_print_usage(stdout, 80, argv[0],
				"[OPTION]... [--] [NON-OPTION]...",
				"This is where you would put an overview description of the "
				"program and its general functionality.", options);
		return 0;
	}

	/* print a summary of options passed */
	for (i = 0; options[i].type != SIMPLE_OPT_END; i++) {
		if (options[i].long_name != NULL)
			printf("--%s, ", options[i].long_name);
		else
			printf("-%c, ", options[i].short_name);

		printf("seen: %s", (options[i].was_seen ? "yes" : "no"));

		if (options[i].arg_is_stored) {
			switch (options[i].type) {
			case SIMPLE_OPT_BOOL:
				printf(", val: %s", options[i].val_bool ? "true" : "false");
				break;
				
			case SIMPLE_OPT_INT:
				printf(", val: %ld", options[i].val_int);
				break;

			case SIMPLE_OPT_UNSIGNED:
				printf(", val: %lu", options[i].val_unsigned);
				break;

			case SIMPLE_OPT_DOUBLE:
				printf(", val: %lf", options[i].val_double);
				break;

			case SIMPLE_OPT_CHAR:
				printf(", val: %c", options[i].val_char);
				break;

			case SIMPLE_OPT_STRING:
				printf(", val: %s", options[i].val_string);
				break;

			case SIMPLE_OPT_STRING_SET:
				printf(", val: %s",
						options[i].string_set[options[i].val_string_set_idx]);
				break;

			default:
				break;
			}
		}

		puts("");
	}

	/* if any non-option arguments were passed, print them */
	if (result.argc > 0) {
		printf("\nnon-options:");

		for (i = 0; i < result.argc; i++)
			printf(" %s", result.argv[i]);

		puts("");
	}

	return 0;
}
