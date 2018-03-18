simple-opt
==========

simple-opt.h is a single header file which implements a simple, flexible and
portable version of command line option parsing for programs written in C. it
is designed to be (hopefully) intuitive while also being (hopefully) more
powerful than traditional getopt or similar, while having no dependencies
outside the standard library and remaining C99 compatible.

what follows is a simple example usage. refer to
[interface.md](doc/interface.md) for more detail.


example
-------

the following example file is available as [example.c](doc/example.c)

```C
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

	/* contains an enum for identifying simple_opt_parse's return value as well
	 * as the argv index of the first non-option and information relevant for
	 * error handling */
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
```

options are stored in an array of `struct simple_opt`, which contains fields
for the option's type, an (optional) short option alias, an (optional) long
option alias, a boolean indicating whether this option's argument is required
or optional, an (optional) string describing what the option does, and an
(optional) string describing how the option's argument type should be printed
by `simple_opt_print_usage`. the end of the array must be indicated with an
option of type `SIMPLE_OPT_END`.

this array is passed to `simple_opt_parse` and, if the parsing is successful,
relevant values (`was_seen`, `arg_is_stored`, `val_<type>`) are in-place stored
in the options. otherwise, the returned `struct simple_opt_result` will have a
type other than `SIMPLE_OPT_RESULT_SUCCESS`, in which case error reporting
occurs.

```
$ ./a.out -y
err: unrecognised option `-y`
```

```
$ ./a.out --int
err: argument expected for option `--int`
```

```
$ ./a.out --bool fake
err: bad argument `fake` passed to option `--bool`
```

if one of the options passed is the help flag (`-h` or `--help`), this example
uses `simple_opt_print_usage` to print out a neatly-formatted usage message
(using the description strings stored in the array earlier). that message looks
like this:

```
$ ./a.out --help
Usage: ./a.out [OPTION]... [--] [NON-OPTION]...

  This is where you would put an overview description of the program and it's
  general functionality.

  -h --help            print this help message and exit
     --int=INT         this thing needs an integer!
  -u --uns=NON-NEG-INT this one has a custom_arg_string. normally it would say
                       "UNSIGNED" rather than "NON-NEG-INT"
  -s STRING            this one doesn't have a long opt version
  -b --bool[=BOOL]     (optionally) takes a boolean arg!
```

note that the output is word-wrapped. it wraps to a maximum of 80 columns
because 80 is passed as the second argument to `simple_opt_print_usage`
(allowing printing to adapt to the user's terminal width if you want to add
support for that, via ncurses or something).

finally, if parsing was successful and usage not printed, this program prints a
quick summary of which options it accepts, which it saw, and what arguments
were passed to those options, and what non-option arguments were passed to the
command itself:

```
$ ./a.out 
--help, seen: no
--int, seen: no
--uns, seen: no
-s, seen: no
--bool, seen: no
```

```
$ ./a.out --int=-1 -s test_string -b -- trailing args passed
--help, seen: no
--int, seen: yes, val: -1
--uns, seen: no
-s, seen: yes, val: test_string
--bool, seen: yes

non-options: trailing args passed
```

```
$ ./a.out --bool=false -u 3
--help, seen: no
--int, seen: no
--uns, seen: yes, val: 3
-s, seen: no
--bool, seen: yes, val: false
```

```
$ ./a.out no options, just arguments
--help, seen: no
--int, seen: no
--uns, seen: no
-s, seen: no
--bool, seen: no

non-options: no options, just arguments
```

```
$ ./a.out non-options --int=+1 can -b on be --uns 0 interleaved
--help, seen: no
--int, seen: yes, val: 1
--uns, seen: yes, val: 0
-s, seen: no
--bool, seen: yes, val: true

non-options: non-options can be interleaved
```

