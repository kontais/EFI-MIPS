#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHSIZE 101

struct nlist {
  struct nlist *next;
  char *keys;
  char *value;
};

static struct nlist *hashtable[HASHSIZE];

unsigned hash(char *s)
{
  unsigned hashval;

  for (hashval = 0; *s != '\0'; s++)
    hashval = *s + 31 * hashval;
  return hashval % HASHSIZE;
}

struct nlist *hashtable_search(char *s)
{
  struct nlist *np;

  for (np = hashtable[hash(s)]; np != NULL; np = np->next)
    if (strcmp(s, np->keys) == 0)
      return np;
  return NULL;
}

struct nlist *hashtable_insert(char *keys, char *value)
{
  struct nlist *np;
  unsigned hashval;

  if ((np = hashtable_search(keys)) == NULL) {
    np = (struct nlist *)malloc(sizeof(*np));
    if (np == NULL || (np->keys = strdup(keys)) == NULL)
      return NULL;
    hashval = hash(keys);
    np->next = hashtable[hashval];
    hashtable[hashval] = np;
  } else
    free((void *)np->value);
  if ((np->value = strdup(value)) == NULL)
    return NULL;
  return np;
}

char *hashtable_getvalue(char *keys)
{
  struct nlist *np;

  if ((np = hashtable_search(keys)) == NULL)
    return NULL;
  else
    return np->value;
}

int main(int argc, char *argv[])
{
  char *ret;

  if (argc < 2) {
    printf("Usage: %s string\n",argv[0]);
    return -1;
  }

  hashtable_insert("INT_MAX", "32767");
  hashtable_insert("INT_MIN", "-32768");
  hashtable_insert("LONG_MAX", "2147483647");
  hashtable_insert("LONG_MIN", "-2147483647");

  if ((ret = hashtable_getvalue(argv[1])) == NULL)
    printf("%s not found\n", argv[1]);
  else
    printf("%s = %s\n", argv[1], ret);
}
