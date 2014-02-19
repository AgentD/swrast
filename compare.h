#ifndef COMPARE_H
#define COMPARE_H



#define COMPARE_ALWAYS 0x00
#define COMPARE_NEVER 0x01
#define COMPARE_EQUAL 0x02
#define COMPARE_NOT_EQUAL 0x03
#define COMPARE_LESS 0x04
#define COMPARE_LESS_EQUAL 0x05
#define COMPARE_GREATER 0x06
#define COMPARE_GREATER_EQUAL 0x07



typedef int (* compare_fun )( int a, int b );



/**
 * \brief Get a pointer to an implementation of a comparison function
 *
 * \param comparison A comparison function identifyer (e.g. COMPARE_ALWAYS)
 *
 * \return A pointer to a function that implements the comparison
 */
compare_fun get_compare_function( int comparison );



#endif /* COMPARE_H */

