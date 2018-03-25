simple-opt
==========

[simple-opt.h](simple-opt.h) is a single header file which implements a
simple, flexible, and portable version of command line option parsing for
programs written in C. it is designed to be (hopefully) intuitive while also
being (hopefully) more powerful than traditional getopt or similar. it has no
dependencies outside the standard library.

what follows is a simple example usage. refer to
[interface.md](doc/interface.md) for more detail.


example
-------

the following example file is available as [example.c](doc/example.c), which
you can compile and test with yourself.

```C
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
		simple_opt_print_error(stderr, 80, argv[0], result);
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
./a.out: unrecognised option `-y`
```

```
$ ./a.out --int
./a.out: argument expected for option `--int`
```

```
$ ./a.out --bool fake
./a.out: bad argument `fake` passed to option `--bool`
         expected a boolean: (yes|true|on) or (no|false|off)
```

if one of the options passed is the help flag (`-h` or `--help`), this example
uses `simple_opt_print_usage` to print out a neatly-formatted usage message
(using the description strings stored in the array earlier). that message looks
like this:

```
$ ./a.out --help
Usage: ./a.out [OPTION]... [--] [NON-OPTION]...

  This is where you would put an overview description of the program and its
  general functionality.

  -h --help                   print this help message and exit
  -b --bool[=BOOL]            (optionally) takes a boolean arg!
     --int=INT                requires an integer. has no short_name!
  -u --uns=NON-NEG-INT        this one has a custom_arg_string. normally it
                                would say "UNSIGNED" rather than "NON-NEG-INT"
  -d --double=DOUBLE          a floating point number
  -s STRING                   this one doesn't have a long_name version
     --set-choice=(str_a|str_b)  a choice of one string from a NULL-terminated
                                array
```

note that the output is word-wrapped. it wraps to a maximum of 80 columns
because 80 is passed as the second argument to `simple_opt_print_usage`
(allowing printing to adapt to the user's terminal width if you want to add
support for that, via ncurses or something). also note that the indentation of
the option descriptions is dependent on the width of the `long_name` column. if
the lengthy `set-choice` line was removed, for example, the output would
become:

```
Usage: ./a.out [OPTION]... [--] [NON-OPTION]...

  This is where you would put an overview description of the program and its
  general functionality.

  -h --help             print this help message and exit
  -b --bool[=BOOL]      (optionally) takes a boolean arg!
     --int=INT          requires an integer. has no short_name!
  -u --uns=NON-NEG-INT  this one has a custom_arg_string. normally it would say
                          "UNSIGNED" rather than "NON-NEG-INT"
  -d --double=DOUBLE    a floating point number
  -s STRING             this one doesn't have a long_name version
```

finally, if parsing was successful and usage not printed, this program prints a
quick summary of which options it accepts, which it saw, and what arguments
were passed to those options, and what non-option arguments were passed to the
command itself:

```
$ ./a.out 
--help, seen: no
--bool, seen: no
--int, seen: no
--uns, seen: no
--double, seen: no
-s, seen: no
--set-choice, seen: no
```

```
$ ./a.out --int=-1 -s test_string -b -- trailing --args -passed
--help, seen: no
--bool, seen: yes
--int, seen: yes, val: -1
--uns, seen: no
--double, seen: no
-s, seen: yes, val: test_string
--set-choice, seen: no

non-options: trailing --args -passed
```

```
$ ./a.out --bool=false -u 3 --set-choice "str_b"
--help, seen: no
--bool, seen: yes, val: false
--int, seen: no
--uns, seen: yes, val: 3
--double, seen: no
-s, seen: no
--set-choice, seen: yes, val: str_b
```

```
$ ./a.out no options, just arguments
--help, seen: no
--bool, seen: no
--int, seen: no
--uns, seen: no
--double, seen: no
-s, seen: no
--set-choice, seen: no

non-options: no options, just arguments
```

```
$ ./a.out non-options --int=+1 can -d 3.9 be --uns 0 interleaved
--help, seen: no
--bool, seen: no
--int, seen: yes, val: 1
--uns, seen: yes, val: 0
--double, seen: yes, val: 3.900000
-s, seen: no
--set-choice, seen: no

non-options: non-options can be interleaved
```

changelog
---------

v1.2: add optional error printing function

v1.0: initial release
