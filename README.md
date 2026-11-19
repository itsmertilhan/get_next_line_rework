*This project has been created as part of the 42 curriculum by merilhan.*

# get_next_line

## Description

`get_next_line` is a C function that reads a file descriptor **one line at a time**.
Each call returns the next line (including the trailing `\n`, if any) as a newly
allocated string, and returns `NULL` when there is nothing left to read or an error
occurs.

```c
char	*get_next_line(int fd);
```

The goal of the project is to learn how to work with **static variables**, manage
memory carefully across successive calls, and read from a file descriptor with a
fixed-size buffer (`BUFFER_SIZE`) without knowing in advance how long a line will be.

The bonus part adds two things:
- support for **multiple file descriptors at the same time** (each fd keeps its own state),
- using **a single static variable**.

## Instructions

### Files

| File | Content |
|---|---|
| `get_next_line.c` / `.h` | Mandatory part |
| `get_next_line_utils.c` | Helper functions (mandatory) |
| `get_next_line_bonus.c` / `.h` | Bonus part (multiple fds) |
| `get_next_line_utils_bonus.c` | Helper functions (bonus) |

### Compilation

The function is meant to be compiled together with your own program. `BUFFER_SIZE`
can be set at compile time (default: `42`).

```sh
# mandatory
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 main.c get_next_line.c get_next_line_utils.c -o gnl

# bonus
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 main.c get_next_line_bonus.c get_next_line_utils_bonus.c -o gnl_bonus
```

For the bonus, include `get_next_line_bonus.h` in `main.c` instead of `get_next_line.h`.

### Usage example

```c
#include <fcntl.h>
#include <stdio.h>
#include "get_next_line.h"

int	main(void)
{
	int		fd;
	char	*line;

	fd = open("file.txt", O_RDONLY);
	while ((line = get_next_line(fd)))
	{
		printf("%s", line);
		free(line);
	}
	return (0);
}
```

```sh
./gnl
```

## Algorithm

### Overview

All the state for one file descriptor is kept in a small struct:

```c
typedef struct s_reader
{
	char	*buffer;   // bytes read but not yet returned
	size_t	length;    // number of valid bytes in buffer
	size_t	scanned;   // how many bytes were already searched for '\n'
	int		flags;     // EOF bit, ERROR bit and buffer capacity
}	t_reader;
```

- **Mandatory:** one `static t_reader reader;`
- **Bonus:** one `static t_reader readers[FD_MAX];`, indexed by `fd`. This is still a
  single static variable, and every fd has its own independent buffer.

Each call to `get_next_line` does the following:

1. **Search** the current buffer for a `\n` (`ft_find_newline`).
2. If no newline is found and EOF has not been reached, **read** `BUFFER_SIZE` more
   bytes and append them to the buffer (`ft_read_chunk`), then search again.
   The search continues from `scanned`, so bytes that were already checked are never
   checked again.
3. When a newline is found, **extract** the line (`ft_take_line`): copy it into a new
   string, move the remaining bytes to the start of the buffer and reset `scanned`.
4. At EOF, return whatever is left in the buffer as the last line (a line without a
   `\n`). If nothing is left, free everything and return `NULL`.
5. On any error (`read` returns `-1` or `malloc` fails), free the buffer, reset the
   state and return `NULL` (`ft_reset_reader`).

### Growing buffer

Instead of concatenating strings with a `strjoin` on every read (which copies the
whole accumulated line each time, so it is O(n²) for long lines), the buffer is a
**growable array**:

- The capacity is always a power of two and stored as an exponent in `flags`.
- When there is less than `BUFFER_SIZE` free space, `ft_grow_buffer` increases the
  exponent until the new bytes fit, allocates the new buffer and copies the old
  content once.
- Because the capacity at least doubles each time it grows, the total copy cost is
  **amortized O(n)** for a line of length n, whatever `BUFFER_SIZE` is.
- Bytes that remain after a line are kept in the same buffer, so a single `read`
  can serve several lines without re-reading.

### State flags

`flags` packs three pieces of information into one `int`:

```
 bits 2..   | bit 1 | bit 0
 capacity   | ERROR | EOF
 exponent   |       |
```

- `reader->flags |= 1 << FLAG_EOF;` marks that `read` returned `0`, so the next calls
  do not call `read` again.
- `reader->flags |= 1 << FLAG_ERROR;` marks a `read` or `malloc` failure.
  `get_next_line` checks this bit after every read and cleans up immediately.
- `ft_has_flag` reads a bit with `(flags >> flag) & 1`.

The function cannot print errors (it can only return a line or `NULL`), so the error
is recorded in the state and handled in one place.

### Newline search (word at a time)

`ft_find_newline` checks **8 bytes at once** using a well-known bit trick
(SWAR, "SIMD within a register"):

```c
word = ft_load_word(buffer + scanned);              // load 8 bytes into a size_t
word = word ^ 0x0A0A0A0A0A0A0A0A;                   // every '\n' byte becomes 0x00
word = (word - 0x0101010101010101) & ~word & 0x8080808080808080;
```

After these steps, only the bytes that were `\n` have their high bit (`0x80`) set.
`ft_first_marked_byte` then finds the lowest marked byte, which is the position of
the first `\n`. The last bytes (fewer than 8) are checked one by one.

`ft_load_word` builds the word byte by byte, so it works no matter how the buffer is
aligned and no matter what the machine's endianness is.

### Why this approach

- **Correctness:** works with any `BUFFER_SIZE` (1, 42, 10 000 000…), with lines
  of any length, with files that do not end with `\n`, and with stdin.
- **Performance:** each byte is read once, searched once and copied a constant
  number of times (amortized), so a call costs time proportional to the length of
  the line.
- **Memory safety:** there is a single owner (the `t_reader`) and a single cleanup
  function, so every error path frees the buffer the same way.
- **Bonus for free:** since all the state lives in `t_reader`, supporting multiple
  fds only means keeping an array of them.

## Resources

- `man 2 read`, `man 3 malloc`, `man 3 free`
- [GNU C Manual – Static variables](https://www.gnu.org/software/c-intro-and-ref/manual/html_node/Static-Local-Variables.html)
- [File descriptor – Wikipedia](https://en.wikipedia.org/wiki/File_descriptor)
- [Dynamic array & amortized analysis – Wikipedia](https://en.wikipedia.org/wiki/Dynamic_array)
- [Bit Twiddling Hacks – Determine if a word has a zero byte](https://graphics.stanford.edu/~seander/bithacks.html#ZeroInWord)
- [SWAR – Wikipedia](https://en.wikipedia.org/wiki/SWAR)

### AI usage

An AI assistant was used only as a helper: to explain the bit constants used in the
newline search (`0x0A0A0A0A0A0A0A0A`, `0x0101010101010101`, `0x8080808080808080`)
