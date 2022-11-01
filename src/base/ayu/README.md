AYU
===

#### All Your data is belong to yoU

This library includes:
 - A readable and writable structured data language
 - A serialization and reflection system for C++
 - A linked resource management system

#### AYU Data Language

The AYU data language is similar to JSON but with the following differences:
 - Commas are not required.
 - Quotes are not required around strings that don't contain whitespace or
   syntactic characters, excepting `null`, `true`, and `false`.
 - Comments are allowed starting with `#` and going to the end of the line.
 - There are shortcuts (like backreferences in YAML).  Preceding an item with
   &name will allow a copy of the same item to be inserted later with `*name`.
   Using `&name` followed by a `:` and then an item will declare a shortcut to
   that item without inserting it into the document at that point.  Shortcuts
   are not semantically visible; `[&foo 1 \*foo]` is exactly equivalent to
   `[1 1]`.  Shortcuts can be used as the keys of attributes in objects if they
   refer to strings.
 - Hexadecimal numbers are allowed starting with `0x`.
 - Special floating point numbers `+inf`, `-inf`, and `+nan` are available.  The
   sign is required; nan and inf by themselves are considered strings.
 - The order of attributes in objects is generally preserved, but should not be
   semantically significant.

Unlike some other data languages, AYU does not have type annotations.  The
typical way to serialize an object that can contain multiple types is to use an
array with two elements, the first of which is the type name, and the second
of which is the value.

See ayu-data-language.md for more details.

#### Serialization library

Documentation pending, but see describe.h

#### Resource management

Documentation pending, but see resource.h
