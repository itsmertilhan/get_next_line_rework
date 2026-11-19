/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: merilhan <merilhan@student.42kocaeli.com.t +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/19 14:55:31 by merilhan          #+#    #+#             */
/*   Updated: 2026/11/19 14:55:32 by merilhan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

char	*get_next_line(int fd)
{
	static t_reader	reader;
	size_t			line_len;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	line_len = ft_find_newline(&reader);
	while (line_len == 0 && !ft_has_flag(&reader, FLAG_EOF))
	{
		ft_read_chunk(fd, &reader);
		if (ft_has_flag(&reader, FLAG_ERROR))
			return (ft_reset_reader(&reader));
		line_len = ft_find_newline(&reader);
	}
	if (line_len > 0)
		return (ft_take_line(&reader, line_len));
	if (reader.length > 0)
		return (ft_take_line(&reader, reader.length));
	return (ft_reset_reader(&reader));
}

int	ft_has_flag(t_reader *reader, int flag)
{
	return ((reader->flags >> flag) & 1);
}

size_t	ft_get_capacity(t_reader *reader)
{
	int	cap_exp;

	if (!reader->buffer)
		return (0);
	cap_exp = reader->flags >> CAP_SHIFT;
	return ((size_t)1 << cap_exp);
}

size_t	ft_load_word(const char *bytes)
{
	size_t	word;
	int		i;

	word = 0;
	i = 7;
	while (i >= 0)
	{
		word = (word << 8) | (unsigned char)bytes[i];
		i--;
	}
	return (word);
}

int	ft_first_marked_byte(size_t word)
{
	int	i;

	i = 0;
	while (((word >> (i << 3)) & 0x80) == 0)
		i++;
	return (i);
}
