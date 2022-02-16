
char	**copy_tabtab(char **res)
{
	int		i;
	int		j;
	char	**dest;

	i = 0;
	j = 0;
	while (res[i])
		i++;
	dest = malloc(sizeof(char *) * (i + 1));
	if (!(dest))
		return (NULL);
	while (res[j])
	{
		dest[j] = ft_strdup(res[j]);
		j++;
	}
	dest[j] = NULL;
	return (dest);
}

int	chrtabtab(char **res, char *str)
{
	int		i;

	i = 0;
	if (!res)
		return (-1);
	while (res[i])
	{
		if (ft_strcmp(res[i], str) == 0)
			return (i);
		i++;
	}
	return (-1);
}

char	**replace_tabtab(char **tabl, int i, char *str)
{
	char	**buf;
	int		j;

	j = 0;
	buf = malloc(sizeof(char *) * (count_tabs(tabl) + 1));
	if (!(buf))
		return (NULL);
	while (tabl[j] && j < i)
	{
		buf[j] = ft_strdup(tabl[j]);
		j++;
	}
	buf[j] = ft_strdup(str);
	j++;
	while (tabl[j])
	{
		buf[j] = ft_strdup(tabl[j]);
		j++;
	}
	buf[j] = NULL;
	free_tabtab(tabl);
	return (buf);
}

void	print_tabtab(char **res)
{
	int		i;

	i = 0;
	while (res[i])
	{
		ft_putstr_fd(res[i], 2);
		i++;
	}
}

void	free_tabtab(char **res)
{
	int		i;

	i = 0;
	if (!res)
		return ;
	while (res[i] != NULL)
		free(res[i++]);
	free(res);
	res = NULL;
}
