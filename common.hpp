/*
 * common.hpp
 * Author: Eyal Cohen
 *
 * Some functions and macros common to many files
 */

#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))

#define MIN(A, B) (A) < (B) ? (A) : (B)
#define MAX(A, B) (A) > (B) ? (A) : (B)

#define ERR_BYTES 32

#endif
