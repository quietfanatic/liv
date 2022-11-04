AYU DATA LANGUAGE
=================

The AYU data language represents structured data in a simple readable format.

### File format

An AYU file is UTF-8 text, with LF (unix-style) newlines.  Writers should not
emit a byte-order mark, but readers should accept one.

### Forms (AKA types)

Items in AYU can have these forms:

##### Null

Represented by the keyword `null`.

##### Bool

Represented by the keywords `true` and `false`.

##### Number

An integer or floating-point number.

Numbers can start with a `+` or a `-`.  They can be decimal numbers with the
digits `0` through `9`.  They can also be hexadecimal numbers starting with `0x`
or `0X` and containing `0`-`9`, `a`-`b`, and `A`-`B`.  No other bases are
supported.  Numbers may start with leading 0s, and are interpreted as decimal
numbers, NOT as octal numbers.

Numbers may be followed by a decimal point (`.`) and then more digits of their
base.  They may then be followed by an `e` or `E` for decimal numbers and a `p`
or `P` for hexadecimal numbers, followed by an optional sign and then decimal
digits.  The exponent is still in decimal, even if the rest of the number is in
hexadecimal.

Numbers must not start or end with a `.`.  `.5` and `5.` are not valid numbers.

Three special numeric values are supported: `+inf`, `-inf`, and `+nan`.  The
sign is required on all of these.  Without the sign, they would be considered
strings.

The reference implementation supports floating-point numbers of double precision
and all integers between -2^63 and 2^63-1.

##### String

A UTF-8 string.  Double quotes (`"`) delimit a string.  Strings that don't
contain any whitespace or syntactic characters do not require quotes, except for
the words `null`, `true`, and `false`.  Quoted strings may contain multiple
lines and any UTF-8 characters except for `"` and `\\`.  The following escape
sequences are supported in quoted strings.
- `\\b` = Backspace
- `\\f` = Form Feed (nobody uses this)
- `\\n` = Newline (LF)
- `\\r` = Carriage Return (CR)
- `\\t` = Tab
- `\\"` = literal quote character
- `\\\\` = literal backslash
- `\\/` = literal slash
- `\\xXX` = UTF-8 byte with a two-digit hexadecimal value, which may be part of
  a multibyte UTF-8 sequence, but must not be an unmatched leading or
  continuation byte. This is not yet implemented by the reference implementation.
- `\uXXXX` = A UTF-16 code unit, which may be part of a surrogate pair, but
  must not be a lone surrogate.  Surrogate pairs are translated into UTF-8.
  This is not yet implemented by the reference implementation.

Some of these escape sequences may be obscure or useless, but they're included
for compatibility with JSON.

A string does not have to be quoted if it starts with a letter or underscore
(`_`) or hash (`#`), is not one of the words `null`, `true`, or `false`, and
only contains the following:
- letters, numbers, or underscores
- Any of these symbols: `!`, `$`, `%`, `+`, `-`, `.`, `/`, `<`, `>`, `?`, `@`,
  `^`, `_`, `~`, `#`, `&`, `*`, `=`, `;`
- The sequences `::` (for C++ namespaces) and `:/` (for URLs)

The following characters are reserved and are not valid for either syntax or
unquoted strings
- `\\`, `\`` (backtick), `(`, `)`, `'` (single quote), `;` (semicolon)

##### Array

Arrays are delimited by square brackets (`[` and `]`) and can contain multiple
items, called elements.  Commas are allowed but not required between items.

Note: Elements in arrays can be prefixed by an integer followed by a colon, in a
syntax similar to objects, but this is just for documentation purposes, it does
not let you reorder or skip elements.
```
[0:a 1:b] # Allowed
[1:a 0:b] # Not allowed
[0:a 2:b] # Not allowed
[0:a b 2:c] # Allowed
```

##### Object

Objects are delimited by curly braces (`{` and `}`) and contain key-value pairs,
called attributes.  A key-value pair is a string (following the above rules for
strings), followed by a colon (`:`), followed by an item.  Commas are allowed
but not required between attributes.  Keys named `null`, `true`, `false` must
be quoted, just as with strings.

### Other Syntax

##### Comments

Comments start with `//` and continue to the end of the line.  There are no
multi-line comments, but you can fake them with a string in an unused shortcut.
Readers must not allow comments to change the sematics of the document.

##### Shortcuts

Shortcuts (known as backreferences in some other DLs) can be declared with an
ampersand (`&`), followed by a string, followed by either an item or a colon and
an item.  If there is no colon, a copy of the item is left in the declaration's
place (like in YAML).  If there is a colon, the whole declaration is discarded
at that point.  (This is so you can declare shortcuts ahead of time at the
beginning of a file.)
```
[1 &a 2 3] # Equivalent to [1 2 3]
[1 &a:2 3] # Equivalent to [1 3]
```
Shortcuts can be used later with an asterisk (`\*`) followed by a string.
Whatever item the shortcut was declared with earlier will be used in its
place.  Shortcuts can be used as the keys in objects if they refer to strings.
Shortcuts are a syntax only; they are semantically invisible and cannot be
recursive.  Shortcut names are global to the file or string being parsed, and
can only be used after they are declared.

### Notes

AYU is designed to be compatible with JSON.  A valid JSON document is also a
valid AYU document, and a valid AYU document can be mechanistically transformed
into a valid JSON document, with the following caveats:
- JSON does not support `+nan`.  Serializers should use `null` instead, and
  deserializers should accept `null` as a floating point number equivalent
  to `+nan`.
- JSON does not support `+inf` and `-inf`.  Instead, these should be written
  with a number outside of double floating-point range for JSON (canonically
  `1e999` and `-1e999`).
- The `\uXXXX` escape sequence in strings is not yet supported.

Unlike some other data languages, AYU does not have type annotations.  The
typical way to represent an item that could have multiple types is to use an
array of two elements, the first of which is a type name and the second of which
is the value.
```
[float 3.5]
[app::Settings {foo:3 bar:4}]
[std::vector<int32> [408 502]]
```

AYU does not have a form for binary data.  To represent binary data, you can use
an array of integers, or a string of hexadecimal digits, or a filename pointing
to a separate binary file.

