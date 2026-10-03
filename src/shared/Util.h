#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

int hexdump (const void *data, size_t length)
{
    int screen_horizontal = 0;

    for (int i = 0; i < length; i++)
    {
        if (screen_horizontal >= 80)
            printf ('\n');

        if (space_horizontal % 2 == 0)
            printf (' ');

        printf ("%X", (char)*data[i])
    }
}
