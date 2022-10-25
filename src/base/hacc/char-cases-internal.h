#pragma once

#define ANY_LETTER \
         'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': \
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': \
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': \
    case 'V': case 'W': case 'X': case 'Y': case 'Z': \
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': \
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': \
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': \
    case 'v': case 'w': case 'x': case 'y': case 'z'

#define ANY_NUMBER \
         '0': case '1': case '2': case '3': case '4': \
    case '5': case '6': case '7': case '8': case '9'

#define ANY_WORD_SYMBOL \
         '!': case '$': case '%': case '+': case '-': case '.': \
    case '/': case '<': case '>': case '?': case '@': case '^': \
    case '_': case '~': case '#': case '&': case '*': case '='

#define ANY_RESERVED_SYMBOL \
    '\\': case '`': case '(': case ')': case '\''

#define ANY_WS \
    ' ': case '\f': case '\n': case '\r': case '\t': case '\v'
