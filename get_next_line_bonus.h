#ifndef GET_NEXT_LINE_BONUS_H
# define GET_NEXT_LINE_BONUS_H

# include <stddef.h>
# include <stdlib.h>
# include <unistd.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 42
# endif

# ifndef FD_MAX
#  define FD_MAX 1024
# endif

# define FLAG_EOF 0
# define FLAG_ERROR 1
# define CAP_SHIFT 2

# define NEWLINE_BYTES 0x0A0A0A0A0A0A0A0AUL
# define ONE_BYTES 0x0101010101010101UL
# define HIGH_BITS 0x8080808080808080UL

typedef struct s_reader
{
	char	*buffer;
	size_t	length;
	size_t	scanned;
	int		flags;
}	t_reader;

char	*get_next_line(int fd);
int		ft_has_flag(t_reader *reader, int flag);
size_t	ft_get_capacity(t_reader *reader);
size_t	ft_load_word(const char *bytes);
int		ft_first_marked_byte(size_t word);
int		ft_grow_buffer(t_reader *reader);
size_t	ft_find_newline(t_reader *reader);
int		ft_read_chunk(int fd, t_reader *reader);
char	*ft_extract_line(t_reader *reader, size_t line_len);
char	*ft_reset_reader(t_reader *reader);

#endif
