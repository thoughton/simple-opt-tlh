simple-opt
==========

data types
----------

### struct simple_opt

an array of `struct simple_opt`, terminating in an element with a `type` field
equal to `SIMPLE_OPT_END`, must be passed when parsing or usage printing. the
fields which should be defined in these elements are:

```
	enum simple_opt_type type;
	const char short_name;
	const char *long_name;
	bool arg_is_required;

	/* optional, used for usage printing */
	const char *description;

	/* optional, a custom string describing the arg, used for usage printing */
	const char *custom_arg_string;
```

if `type` is `SIMPLE_OPT_FLAG`, this option may not accept arguments. if `type`
is `SIMPLE_OPT_END`, parsing and usage printing will return at this point when
iterating through the array and will not see any elements which may follow.

`short_name` is optional, and may be undefined for this option by passing '\0'.
`long_name` is also optional, and may be left undefined for this option by
passing `NULL`. however, at fewest one of these two must be defined for every
option.

the fields which are set by `simple_opt_parse` are:

```
	bool was_seen;
	bool arg_is_stored;

	union {
		bool val_bool;
		int val_int;
		unsigned val_unsigned;
		char val_string[SIMPLE_OPT_ARG_MAX_WIDTH];
	};
```

`was_seen` indicates if this option was encountered during parsing,
`arg_is_stored` if an argument was passed to the option, and the `val_<type>`
fields contain the value passed (with the correct field to set being determined
by the `type` field shown above).


### struct simple_opt_result

```
struct simple_opt_result {
	enum simple_opt_result_type result_type;
	enum simple_opt_type option_type;
	char option_string[SIMPLE_OPT_OPT_MAX_WIDTH];
	char argument_string[SIMPLE_OPT_OPT_ARG_MAX_WIDTH];
	int argc;
	char *argv[SIMPLE_OPT_MAX_ARGC];
};
```

`simple_opt_parse` returns a `struct simple_opt_result`. upon successful
parsing, its `result_type` field will contain `SIMPLE_OPT_RESULT_SUCCESS`.
otherwise, it will contain an error which should be handled by the caller. the
first three are user-caused errors:

```
	SIMPLE_OPT_RESULT_UNRECOGNISED_OPTION,
	SIMPLE_OPT_RESULT_BAD_ARG,
	SIMPLE_OPT_RESULT_MISSING_ARG,
```

in the case of `SIMPLE_OPT_RESULT_UNRECOGNISED_OPTION`, `option_string` will
contain the unrecognised option which was passed.

if the type is `SIMPLE_OPT_BAD_ARG`, then `option_string` will be set,
`option_type` will be set to the type of that option (`SIMPLE_OPT_BOOL` etc),
and the bad argument will be stored in `argument_string`.

if the type is `SIMPLE_OPT_MISSING_ARG`, `option_string` and `option_type` will
be set.

the remaining result types are internal errors:

```
	SIMPLE_OPT_RESULT_ARG_TOO_LONG,
	SIMPLE_OPT_RESULT_TOO_MANY_ARGS,
	SIMPLE_OPT_RESULT_MALFORMED_OPTION_STRUCT,
```

`SIMPLE_OPT_RESULT_OPT_ARG_TOO_LONG` will be returned if an option argument,
passed on the command line by a user, was too long for the internal buffer. the
internal buffer can be resized by defining `SIMPLE_OPT_OPT_MAX_WIDTH` at some
point before `simple-opt.h` is included.

`SIMPLE_OPT_RESULT_TOO_MANY_ARGS` is returned if the user passed too many
non-option arguments to the command for its internal argv filter buffer. this
limit can also be resized by defining `SIMPLE_OPT_MAX_ARGC`

finally, `SIMPLE_OPT_RESULT_MALFORMED_OPTION_STRUCT` is returned if the
programmer has passed a `struct simple_opt` array which contains disallowed
option configurations (that is, two options share a `short_name` or
`long_name`, an option has neither a `short_name` nor a `long_name`, or an
option of type `SIMPLE_OPT_FLAG` is marked as requiring an argument)


functions
---------

`simple-opt.h` defines two functions for external use, `simple_opt_parse` and
`simple_opt_print_usage`. any other functions are prefixed `sub_simple_opt` and
should not be called directly (their visibility is a by-product of simple-opt's
single header file nature)


### simple_opt_parse

`simple_opt_parse` takes three arguments and returns a `struct
simple_opt_result`:

```
struct simple_opt_result simple_opt_parse(int argc, char **argv, struct
		simple_opt *options);
```

`argc` is the number of arguments contained in `argv`, and `argv` is an array
of character string pointers. normally here the programmer would just pass on
the `argc` and `argv` received as arguments from the `main` function.

`struct simple_opt` is an array of the options available to be parsed
(described above) and `struct simple_opt_result` contains a set of results
about that parsing (also described above).


### simple_opt_print_usage

`simple_opt_print_usage` takes six arguments and prints a neatly-formatted
usage message, similar to those typical of GNU cli commands:

```
void simple_opt_print_usage(FILE *f, unsigned width, char *usage_name,
		char *usage_options, char *usage_summary, struct simple_opt *options)
```

`f` is a file pointer to which the message should be printed

`width` is the column width to which the output should be word-wrapped (passing
0 disables wrapping). a reasonable value here would be 80, but this could also
be used to allow more dynamic behaviour (e.g. using something like `ncurses` or
`ioctl` to get the users's current terminal width)

`usage_name` is the name of the command as it will be printed in the usage
statement. easiest is just to pass `argv[0]` here.

`usage_options` is a summary of what options the command takes (e.g. something
like `[OPTION]...`)

together, these two result in something that looks like:

```
Usage: ./a.out [OPTION]...
```

if both `usage_name` and `usage_options` are `NULL`, this initial line will not
be printed, allowing more flexibility to the programmer (if you wanted to, for
example, print multiple such lines on your own in order to represent different
use-cases)

`usage_summary` is usually a one or two sentence overview summary of how the
command behaves. if this is left as `NULL`, no summary will be printed.

the final argument, `struct simple_opt *options`, is an array of options as
defined above.

*note:* usage printing's word wrap operates under the assumptions that your
language delimits words with spaces (i.e. "when i was a child..." vs.
"子供時代に..."), that the font used is fixed-width, and that every character
occupies one column (that is, there are no ｗｉｄｅ characters, combining
diacritics, etc). if these assumptions do not apply to your use case, you
should use an alternative method for usage printing.
