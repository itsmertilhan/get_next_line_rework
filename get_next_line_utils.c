#include "get_next_line.h"

int	ft_grow_buffer(t_reader *reader)
{
	char	*new_buffer;
	int		cap_exp;
	size_t	i;

	cap_exp = reader->flags >> CAP_SHIFT;
	while (((size_t)1 << cap_exp) - reader->length < BUFFER_SIZE)
		cap_exp++;
	new_buffer = malloc((size_t)1 << cap_exp);
	if (!new_buffer)
		return (0);
	i = 0;
	while (i < reader->length)
	{
		new_buffer[i] = reader->buffer[i];
		i++;
	}
	free(reader->buffer);
	reader->buffer = new_buffer;
	reader->flags &= (1 << CAP_SHIFT) - 1;
	reader->flags |= cap_exp << CAP_SHIFT;
	return (1);
}

size_t	ft_find_newline(t_reader *reader)
{
	size_t	word;

	while (reader->scanned + 8 <= reader->length)
	{
		word = ft_load_word(reader->buffer + reader->scanned);
		word = word ^ NEWLINE_BYTES;
		word = (word - ONE_BYTES) & ~word & HIGH_BITS;
		if (word != 0)
			return (reader->scanned + ft_first_marked_byte(word) + 1);
		reader->scanned += 8;
	}
	while (reader->scanned < reader->length)
	{
		if (reader->buffer[reader->scanned] == '\n')
			return (reader->scanned + 1);
		reader->scanned++;
	}
	return (0);
}

int	ft_read_chunk(int fd, t_reader *reader)
{
	ssize_t	bytes_read;
	size_t	free_space;

	free_space = ft_get_capacity(reader) - reader->length;
	if (free_space < BUFFER_SIZE && !ft_grow_buffer(reader))
	{
		reader->flags |= 1 << FLAG_ERROR;
		return (-1);
	}
	bytes_read = read(fd, reader->buffer + reader->length, BUFFER_SIZE);
	if (bytes_read == 0)
		reader->flags |= 1 << FLAG_EOF;
	else if (bytes_read < 0)
		reader->flags |= 1 << FLAG_ERROR;
	else
		reader->length += bytes_read;
	return ((int)bytes_read);
}

char	*ft_extract_line(t_reader *reader, size_t line_len)
{
	char	*line;
	size_t	i;

	line = malloc(line_len + 1);
	if (!line)
		return (ft_reset_reader(reader));
	i = 0;
	while (i < line_len)
	{
		line[i] = reader->buffer[i];
		i++;
	}
	line[line_len] = '\0';
	i = 0;
	while (line_len + i < reader->length)
	{
		reader->buffer[i] = reader->buffer[line_len + i];
		i++;
	}
	reader->length -= line_len;
	reader->scanned = 0;
	return (line);
}

char	*ft_reset_reader(t_reader *reader)
{
	free(reader->buffer);
	reader->buffer = NULL;
	reader->length = 0;
	reader->scanned = 0;
	reader->flags = 0;
	return (NULL);
}
