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
   syntactic characters, excepting null, true, and false.
 - Comments are allowed starting with # and going to the end of the line.
 - There are backreferences like in YAML.  Preceding an item with &name will
   allow a copy of the same item to be inserted later with \*name.  Using &name
   followed by a : and then an item will declare a backreference to that item
   without inserting it into the document at that point.  Backreferences are
   not semantically visible; [&foo 1 \*foo] is exactly equivalent to [1 1].
   Backreferences can be used as the keys of attributes in objects if they
   refer to strings.
 - Hexadecimal numbers are allowed starting with 0x.
 - Special floating point numbers +inf, -inf, and +nan are available.  The sign
   is required; nan and inf by themselves are considered strings.
 - The order of attributes in objects is generally preserved, but should not be
   semantically significant.
////// TODO
// - Long strings can be written as Perl-style heredocs, starting with << followed
//   by a word, and ending with that same word on its own line.  Indentation will
//   be removed equal to the indentation of the terminating word.
// - The \uXXXX character escape is currently not available.  There is a \xXX
//   character escape that translates to one byte; to embed unicode in a string,
//   either embed it directly as UTF-8 or use multiple \xXX sequences.  If you
//   have a binary string, consider representing it as a hexadecimal string or,
//   if it's large, storing it in a separate file.

Unlike some other data languages, ayu does not contain type annotations.  The
typical way to serialize an object that can contain multiple types is to use an
array with two elements, the first of which is the type name, and the second
of which is the value.

#### Serialization library

Documentation pending, but see describe.h

#### Resource management

Documentation pending, but see resource.h
