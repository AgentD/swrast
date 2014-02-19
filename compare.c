#include "compare.h"

#include <stddef.h>



static int compare_always( int a, int b )
{
    (void)a; (void)b;
    return 1;
}

static int compare_never( int a, int b )
{
    (void)a; (void)b;
    return 0;
}

static int compare_equal( int a, int b )
{
    return a==b;
}

static int compare_not_equal( int a, int b )
{
    return a!=b;
}

static int compare_less( int a, int b )
{
    return a<b;
}

static int compare_less_equal( int a, int b )
{
    return a<=b;
}

static int compare_greater( int a, int b )
{
    return a>b;
}

static int compare_greater_equal( int a, int b )
{
    return a>=b;
}

/****************************************************************************/

compare_fun get_compare_function( int comparison )
{
    switch( comparison )
    {
    case COMPARE_ALWAYS:        return compare_always;
    case COMPARE_NEVER:         return compare_never;
    case COMPARE_EQUAL:         return compare_equal;
    case COMPARE_NOT_EQUAL:     return compare_not_equal;
    case COMPARE_LESS:          return compare_less;
    case COMPARE_LESS_EQUAL:    return compare_less_equal;
    case COMPARE_GREATER:       return compare_greater;
    case COMPARE_GREATER_EQUAL: return compare_greater_equal;
    }

    return NULL;
}

