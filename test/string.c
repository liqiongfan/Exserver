/**
 * File:string.c for project sockets.
 * Author: Josin
 * Email: xeapplee@gmail.com
 * Website: https://www.supjos.cn
 */

#include "../kernel/types.h"

int main(int argc, char *argv[])
{
    char *str = " hello world ";
    
    /* return " hello world " */
    char *trim_none_result = extrim(str, TRIM_NONE);
    printf("[%s]\n", trim_none_result);
    free(trim_none_result);
    
    /* return "hello world " */
    char *left_trim_result = extrim(str, TRIM_LEFT);
    printf("[%s]\n", left_trim_result);
    free(left_trim_result);
    
    /* return " hello world" */
    char *right_trim_result = extrim(str, TRIM_RIGHT);
    printf("[%s]\n", right_trim_result);
    free(right_trim_result);
    
    /* return "hello world" */
    char *both_trim_result = extrim(str, TRIM_BOTH);
    printf("[%s]\n", both_trim_result);
    free(both_trim_result);
    
    return 0;
}