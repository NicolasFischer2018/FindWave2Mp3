/*
 ** my_strncmp.c for piscine in 
 ** 
 ** Made by Charlie Root
 ** Login   <boulon@epita.fr>
 ** 
 ** Started on  Mon Aug 14 14:23:01 2000 Charlie Root
** Last update Tue Oct 17 15:41:37 2000 assistant c unix
 */

int	my_strncmp(char *s1, char *s2, unsigned int n)
{
  while (*s1 && *s2 && n != 0)
    {
      if (*s1 != *s2)
	return (*s1 - *s2);
      s1++;
      s2++;
      n--;
    }
  if (n == 0)
    return (0);
  return (*s1 - *s2);
}
