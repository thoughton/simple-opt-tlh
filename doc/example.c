#include "../src/simple-opt.h"

int main(int argc, char **argv)
{
	/* array containing all options and their types / attributes */
	struct simple_opt options[] = {
		{ SIMPLE_OPT_FLAG, 'h', "help", false,
			"print this help message and exit" },
		{ SIMPLE_OPT_INT, '\0', "int", true,
			"this thing needs an integer!" },
		{ SIMPLE_OPT_UNSIGNED, 'u', "uns", true,
			"this one has a custom_arg_string. normally it would say" 
				" \"UNSIGNED\" rather than \"NON-NEG-INT\"",
			"NON-NEG-INT" },
		{ SIMPLE_OPT_STRING, 's', NULL, true,
			"this one doesn't have a long opt version" },
		{ SIMPLE_OPT_BOOL, 'b', "bool", false,
			"(optionally) takes a boolean arg!" },
		{ SIMPLE_OPT_END },
	};

	/* contains an enum for identifying simple_opt_parse's return value, an
	 * array of the cli arguments which were not parsed as options, and
	 * information relevant for error handling */
	struct simple_opt_result result;

	int i;

	result = simple_opt_parse(argc, argv, options);

	/* handle errors */
	switch (result.result_type) {
	case SIMPLE_OPT_RESULT_UNRECOGNISED_OPTION:
		fprintf(stderr, "err: unrecognised option `%s`\n",
				result.option_string);
		return 1;

	case SIMPLE_OPT_RESULT_BAD_ARG:
		fprintf(stderr, "err: bad argument `%s` passed to option `%s`\n",
				result.argument_string, result.option_string);
		return 1;

	case SIMPLE_OPT_RESULT_MISSING_ARG:
		fprintf(stderr, "err: argument expected for option `%s`\n",
				result.option_string);
		return 1;

	case SIMPLE_OPT_RESULT_OPT_ARG_TOO_LONG:
		fprintf(stderr, "internal err: argument passed to option `%s` is too long\n",
				result.option_string);
		return 1;

	case SIMPLE_OPT_RESULT_TOO_MANY_ARGS:
		fprintf(stderr, "internal err: too many cli arguments passed\n");
		return 1;

	case SIMPLE_OPT_RESULT_MALFORMED_OPTION_STRUCT:
		fprintf(stderr, "internal err: malformed option struct\n");
		return 1;

	default:
		break;
	}

	/* if the help flag was passed, print usage */
	if (options[0].was_seen) {
		simple_opt_print_usage(stdout, 80, argv[0],
				"[OPTION]... [--] [NON-OPTION]...",
				"This is where you would put an overview description of the "
				"program and it's general functionality.", options);
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
			case SIMPLE_OPT_INT:
				printf(", val: %d", options[i].val_int);
				break;

			case SIMPLE_OPT_UNSIGNED:
				printf(", val: %u", options[i].val_unsigned);
				break;

			case SIMPLE_OPT_STRING:
				printf(", val: %s", options[i].val_string);
				break;

			case SIMPLE_OPT_BOOL:
				printf(", val: %s", options[i].val_bool ? "true" : "false");
				break;
				
			default:
				break;
			}
		}

		puts("");
	}

	/* if any non-option arguments were passed, print them */
	if (result.argc > 0) {
		printf("\nnon-options:", result.argc);

		for (i = 0; i < result.argc; i++)
			printf(" %s", result.argv[i]);

		puts("");
	}

	return 0;
}
