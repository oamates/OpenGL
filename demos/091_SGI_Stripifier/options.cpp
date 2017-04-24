//=======================================================================================================================================================================================================================
// STRIPE : options.cpp
// This file contains routines that are used to determine the options that were specified by the user
//=======================================================================================================================================================================================================================

#include <cstdio>
#include <cstdlib>
#include "options.hpp"
#include "global.hpp"

static int power_10(int power)                                                                              /* Raise 10 to the power */
{
    int p = 1;
    for (int i = 1; i <= power; ++i)
        p = p * 10;
    return p;
}

static float power_negative(int power)                                                                      /* Raise 10 to the negative power */
{
    float p = 1.0f;
    for (int i = 1; i <= power; i++)
        p = p * 0.1f;
    return p;
}

static float convert_array(int num[],int stack_size)
{
    int c, counter;
    float temp = 0.0f;                                                                                      /* Convert an array of characters to an integer */
    for (c = stack_size - 1, counter = 0; c >= 0; c--, counter++)
    {
        if (num[c] == -1)                                                                                   /* We are at the decimal point, convert to decimal less than 1 */
        {
            counter = -1;
            temp = power_negative(stack_size - c - 1) * temp;
        }
        else 
            temp += power_10(counter) * num[c];
    }
    return(temp);
}

void print_usage(void) 
{
    printf("Usage: stripe [abfglopqrw] file_name\n");
    printf("Options:\n");
    printf(" -b\tSpecifies that the input file is in binary\n");
    printf(" -g\tFind triangle strips only within the groups specified in the data file\n");
    printf(" -o\tTurn off orientation checking\n\n");
    printf("Tie Breaking:\n");
    printf(" -a\tAlternate left-right direction in choosing next polygon\n");
    printf(" -f\tChoose the first polygon that it found\n");
    printf(" -l\tLook ahead one level in choosing the next polygon\n");
    printf(" -q\tChoose the polygon which does not produce a swap\n");
    printf(" -r\tRandomly choose the next polygon\n\n");
    printf("Triangulation:\n");
    printf(" -p\tPartially triangulate faces\n");
    printf(" -w\tFully triangluate faces\n");
}


float get_options(int argc, char **argv, int *f, int *t, int *tr, int *group, int *orientation)
{
    char c;
    int count = 0;
    int buffer[MAX1];
    int next = 0;
    enum tie_options tie = FIRST;                                           /* tie variable */
    enum triangulation_options triangulate = WHOLE;                         /* triangulation variable */
    float norm_difference = 360.0f;                                         /* normal difference variable (in degrees) */
    enum file_options file_type = ASCII;                                    /* file-type variable */
    *orientation = 1;
  
    if ((argc > 6) || (argc < 2))                                           /* User has the wrong number of options */
    {
        print_usage();
        exit(0);
    }
  
    while (--argc > 0 && (*++argv)[0] == '-')                               /* Interpret the options specified */
    {
        next = 1;                                                           /* At the next option that was specified */
        while ((c = *++argv[0]))
            switch (c)
            {
                case 'f': tie = FIRST; break;                               /* Use the first polygon we see. */
                case 'r': tie = RANDOM; break;                              /* Randomly choose the next polygon */
                case 'a': tie = ALTERNATE; break;                           /* Alternate direction in choosing the next polygon */
                case 'l': tie = LOOK; break;                                /* Use lookahead to choose the next polygon */
                case 'q': tie = SEQUENTIAL; break;                          /* Try to reduce swaps */
                case 'p': triangulate = PARTIAL; break;                     /* Use partial triangulation of polygons */
                case 'w': triangulate = WHOLE; break;                       /* Use whole triangulation of polygons */
                case 'b': file_type = BINARY; break;                        /* Input file is in binary */
                case 'g': *group = 1; break;                                /* Strips will be grouped according to the groups in the data file. We will have to restrict strips to be in the grouping of the data file. */
                case 'o': *orientation = 0;
                                                                            /*  Get each the value of the integer. We have an integer */
                default:
                    if ((c >= '0') && (c <= '9'))                           /* More than one normal difference specified, use the last one */
                    {
        
                        if (next == 1)
                        {
                            count = 0;
                            next = 0;
                        }
                        buffer[count++] = ATOI(c);
                    }
                    else if (c == '.')                                      /* At the decimal point */
                    {
        
                        if (next == 1)                                      /* More than one normal difference specified, use the last one */
                        {
                            count = 0;
                            next = 0;
                        }
                        buffer[count++] = -1;
                    }
                    else 
                        break;
            }
    }
    if (count != 0) norm_difference = convert_array(buffer, count);         /* Convert the buffer of characters to a floating pt integer */
    *f = file_type;
    *t = tie;
    *tr = triangulate;
    return norm_difference;
}
