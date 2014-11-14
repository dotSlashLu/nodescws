#include "cJSON.h"
#include <stdio.h>

char *keys[] = {
        "attrs",
        "stopwords",
        "nostats",
        "fuck",
        "you"
};

int main(int argc, char **argv)
{
        int i;
        for (i = 0; i < (sizeof(keys) / sizeof(char *)); i++)
                printf("%s\n", keys[i]);
        return 0;
}

