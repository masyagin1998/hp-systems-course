/*
    The new function allocates complex_struct with poor memory locality:
    the large buffer and the struct itself are located in separate memory regions.
    As a result, moving the structure in memory is inefficient and cache-unfriendly.

    Task: improve memory locality by ensuring that all struct fields, including the buffer,
    are stored in a single contiguous memory block instead of being spread across allocations.
    
    Code Author: Vladislav Shpilevoy 
*/


#include <stdlib.h>

struct complex_struct {
    int id;
    double a;
    long d;
    char buf[10];
    char *long_buf;
};

struct complex_struct *
complex_struct_new(int long_buf_len) 
{
    struct complex_struct *ret =
        (struct complex_struct *) malloc(sizeof(*ret));
    ret->long_buf = (char *) malloc(long_buf_len);
    return ret; 
}