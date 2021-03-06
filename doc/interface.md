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

	/* required for type SIMPLE_OPT_STRING_SET, a NULL-terminated array of
	 * string possibilities against which an option's argument is matched */
	const char **string_set;
```

if `type` is `SIMPLE_OPT_FLAG`, this option may not accept arguments. if `type`
is `SIMPLE_OPT_END`, parsing and usage printing will return at this point when
iterating through the array and will not see any elements which may follow.
thus, in practice, the array definition should look something like this:

```
struct simple_opt options[] = {
	{ SIMPLE_OPT_<type>, <short_name>, <long_name>, <true|false> ...},
	{ SIMPLE_OPT_<type>, <short_name>, <long_name>, <true|false> ...},
	...
	{ SIMPLE_OPT_END }
};
```

`short_name` is optional, and it may be left undefined for this option by
passing '\0'. `long_name` is also optional and may be left undefined by passing
`NULL`. however, at fewest one of these two must be defined for every option.

the fields which are set by `simple_opt_parse` are:

```
	bool was_seen;
	bool arg_is_stored;

	union {
		bool v_bool;
		long v_int;
		unsigned long v_unsigned;
		double v_double;
		char v_char;
		char v_string[SIMPLE_OPT_OPT_ARG_MAX_WIDTH];
		int v_string_set_idx;
	} val;
```

`was_seen` indicates if this option was encountered during parsing,
`arg_is_stored` if an argument was passed to the option, and the `val.v_<type>`
fields contain the value passed (with the correct field to set being determined
by the `type` field shown above) for all but `SIMPLE_OPT_STRING_SET`, for which
`val.v_string_set_idx` is set, an index into the `string_set` field's array,
indicating which possibility was matched.

options of the following types:

```
	SIMPLE_OPT_BOOL,
	SIMPLE_OPT_INT,
	SIMPLE_OPT_UNSIGNED,
	SIMPLE_OPT_DOUBLE,
	SIMPLE_OPT_CHAR,
	SIMPLE_OPT_STRING,
	SIMPLE_OPT_STRING_SET,
```

take arguments. if the user passes a short option on the cli, that option's
argument is passed as the following cli argument, like so:

```
./a.out -x <arg_goes_here>
```

if the user passes a long option on the cli, that option's argument can be
passed either as the next cli argument or appended to a trailing `=`, like so:

```
./a.out --opt-x <arg_goes_here>
./a.out --opt-x=<arg_goes_here>
```

arguments acceptable to type `SIMPLE_OPT_BOOL` are `true`, `yes`, or `on`, all
of which result in a value of true, and `false`, `no`, or `off`, which result
in a value of false.

arguments acceptable to type `SIMPLE_OPT_INT` must be integers with an optional
leading sign indicator of '-' or '+'. they are assumed decimal unless given a
prefix to indicate otherwise ('0' for octal and '0x' for hexadecimal).
arguments with values too large (negative or positive) to be stored in a signed
integer `long` will also be rejected.

arguments acceptable to type `SIMPLE_OPT_UNSIGNED` are the same as those
acceptable to `SIMPLE_OPT_INT`, save that they cannot have a sign indicator and
are limited to the size of an `unsigned long`.

arguments acceptable to type `SIMPLE_OPT_DOUBLE` may be any representation of a
floating point number that can be read by the standard library `strtod`
function and stored in a `double` type. this includes arguments like "4.9",
"-1.2e20", "infinity", or "nan".

arguments acceptable to type `SIMPLE_OPT_CHAR` may be any single-byte character
or one of the following two-character escape sequences: `\a`, `\b`, `\f`, `\n`,
`\r`, `\t`, `\v`.

arguments acceptable to type `SIMPLE_OPT_STRING` may be any string of
characters the user passes.

arguments acceptable to type `SIMPLE_OPT_STRING_SET` may be any character
string which also appears in the programmer-defined NULL-terminated array of
strings in the `string_set` field. in practice, adding an argument of this type
would look something like this:

```
const char *set[] = { "choice_a", "choice_b", ..., NULL };

struct simple_opt options[] = {
	...
	{ SIMPLE_OPT_STRING_SET, <short_name>, <long_name>, <true|false>,
	[description], [custom_arg_string], set },
	...
};
```

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
`option` will be made to point to the option within the provided option array,
and the bad argument will be stored in `argument_string`.

if the type is `SIMPLE_OPT_MISSING_ARG`, `option_string`, `option`, and
`option_type` will be set.

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
`long_name`, an option has neither a `short_name` nor a `long_name`, an option
of type `SIMPLE_OPT_FLAG` is marked as requiring an argument, or an option of
type `SIMPLE_OPT_STRING_SET` has a NULL `string_set` field)


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
static struct simple_opt_result simple_opt_parse(int argc, char **argv,
		struct simple_opt *options);
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
static void simple_opt_print_usage(FILE *f, unsigned width,
		char *command_name, char *command_options, char *command_summary,
		struct simple_opt *options);
```

`f` is a file pointer to which the message should be printed.

`width` is the column width to which the output should be word-wrapped (passing
0 disables wrapping). a reasonable value here would be 80, but this could also
be used to allow more dynamic behaviour (e.g. using something like `ncurses` or
`ioctl` to get the users's current terminal width).

`command_name` is the name of the command as it will be printed in the usage
statement. easiest is just to pass `argv[0]` here.

`command_options` is a summary of what options the command takes (e.g.
something like `[OPTION]...`)

together, these two result in something that looks like:

```
Usage: ./a.out [OPTION]...
```

if both `command_name` and `command_options` are `NULL`, this initial line will
not be printed, allowing more flexibility to the programmer (if you wanted to,
for example, print multiple such lines on your own in order to represent
different use-cases)

`command_summary` is usually a one or two sentence overview summary of how the
command behaves. if this is left as `NULL`, no summary will be printed.

the final argument, `struct simple_opt *options`, is an array of options as
defined above.

*note:* usage printing's word wrap operates under the assumptions that your
language delimits words with spaces (i.e. "when i was a child..." vs.
"子供時代に..."), that the font used is fixed-width and that every character
occupies one column (that is, there are no ｗｉｄｅ characters, combining
diacritics, etc), and that all characters are one byte (no multi-byte utf-8
characters). if these assumptions do not apply to your use case, you should use
an alternative method for usage printing.


### simple_opt_print_error

`simple_opt_print_error` takes three arguments and prints a default error
message, if there is one to be printed:

```
static void simple_opt_print_error(FILE *f, unsigned width, char *command_name,
		struct simple_opt_result result);
```

`f` is a file pointer to which the message should be printed.

`width` is the width to which the output should be word-wrapped. if 0, no
wrapping will be performed. see `width` in `simple_opt_print_usage` above.

`command_name` is the name of the command as it will be printed in the usage
statement. easiest is just to pass `argv[0]` here. if NULL is passed, "err:"
will be printed instead.

`result` is a populated result from a call to `simple_opt_parse`.

if the result's type field contains `SIMPLE_OPT_RESULT_SUCCESS`, nothing will
be printed. otherwise, the output is a summary of the error.
