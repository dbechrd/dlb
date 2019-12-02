#ifndef DLB_RAND_H
#define DLB_RAND_H
//------------------------------------------------------------------------------
// Copyright 2018 Dan Bechard
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------
#include "dlb_types.h"

//-- mersenne-twister.h -------------------------------------------------------------
/*
 * The Mersenne Twister pseudo-random number generator (PRNG)
 *
 * This is an implementation of fast PRNG called MT19937, meaning it has a
 * period of 2^19937-1, which is a Mersenne prime.
 *
 * This PRNG is fast and suitable for non-cryptographic code.  For instance, it
 * would be perfect for Monte Carlo simulations, etc.
 *
 * For all the details on this algorithm, see the original paper:
 * http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/ARTICLES/mt.pdf
 *
 * Written by Christian Stigen Larsen
 * Distributed under the modified BSD license.
 * 2015-02-17, 2017-12-06
 */

/*
* Extract a pseudo-random unsigned 32-bit integer in the range 0 ... UINT32_MAX
*/
u32 mersenne_rand_u32();

/*
* Initialize Mersenne Twister with given seed value.
*/
void mersenne_seed(u32 seed_value);

//-- dlb_rand.h ----------------------------------------------------------------
void dlb_rand_seed();
u32 dlb_rand_u32();
#endif
//-- end of header -------------------------------------------------------------

#ifdef __INTELLISENSE__
/* This makes MSVC intellisense work. */
#define DLB_RAND_IMPLEMENTATION
#endif

//-- implementation ------------------------------------------------------------
#ifdef DLB_RAND_IMPLEMENTATION
#ifndef DLB_RAND_IMPL_INTERNAL
#define DLB_RAND_IMPL_INTERNAL

//-- mersenne-twister.c --------------------------------------------------------
/*
 * The Mersenne Twister pseudo-random number generator (PRNG)
 *
 * This is an implementation of fast PRNG called MT19937, meaning it has a
 * period of 2^19937-1, which is a Mersenne prime.
 *
 * This PRNG is fast and suitable for non-cryptographic code.  For instance, it
 * would be perfect for Monte Carlo simulations, etc.
 *
 * For all the details on this algorithm, see the original paper:
 * http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/ARTICLES/mt.pdf
 *
 * Written by Christian Stigen Larsen
 * Distributed under the modified BSD license.
 * 2015-02-17, 2017-12-06
 */

 // Better on older Intel Core i7, but worse on newer Intel Xeon CPUs (undefine
 // it on those).
 //#define MT_UNROLL_MORE

 /*
  * We have an array of 624 32-bit values, and there are 31 unused bits, so we
  * have a seed value of 624*32-31 = 19937 bits.
  */
#define MERSENNE_SIZE 624
#define MERSENNE_PERIOD 397
#define MERSENNE_DIFF (MERSENNE_SIZE - MERSENNE_PERIOD)

static const u32 MERSENNE_MAGIC = 0x9908b0df;

// State for a singleton Mersenne Twister. If you want to make this into a
// class, these are what you need to isolate.
typedef struct {
    u32 MT[MERSENNE_SIZE];
    u32 MT_TEMPERED[MERSENNE_SIZE];
    size_t index;
} MTState;

static MTState mt_state;

#define M32(x) (0x80000000 & x) // 32nd MSB
#define L31(x) (0x7FFFFFFF & x) // 31 LSBs

#define UNROLL(expr) \
  y = M32(mt_state.MT[i]) | L31(mt_state.MT[i+1]); \
  mt_state.MT[i] = mt_state.MT[expr] ^ (y >> 1) ^ ((((s32)(y) << 31) >> 31) & MERSENNE_MAGIC); \
  ++i;

static void mersenne_generate_numbers()
{
    /*
     * For performance reasons, we've unrolled the loop three times, thus
     * mitigating the need for any modulus operations. Anyway, it seems this
     * trick is old hat: http://www.quadibloc.com/crypto/co4814.htm
     */

    size_t i = 0;
    u32 y;

    // i = [0 ... 226]
    while (i < MERSENNE_DIFF) {
        /*
         * We're doing 226 = 113*2, an even number of steps, so we can safely
         * unroll one more step here for speed:
         */
        UNROLL(i + MERSENNE_PERIOD);

#ifdef MT_UNROLL_MORE
        UNROLL(i + MERSENNE_PERIOD);
#endif
    }

    // i = [227 ... 622]
    while (i < MERSENNE_SIZE - 1) {
        /*
         * 623-227 = 396 = 2*2*3*3*11, so we can unroll this loop in any number
         * that evenly divides 396 (2, 4, 6, etc). Here we'll unroll 11 times.
         */
        UNROLL(i - MERSENNE_DIFF);

#ifdef MT_UNROLL_MORE
        UNROLL(i - MERSENNE_DIFF);
        UNROLL(i - MERSENNE_DIFF);
        UNROLL(i - MERSENNE_DIFF);
        UNROLL(i - MERSENNE_DIFF);
        UNROLL(i - MERSENNE_DIFF);
        UNROLL(i - MERSENNE_DIFF);
        UNROLL(i - MERSENNE_DIFF);
        UNROLL(i - MERSENNE_DIFF);
        UNROLL(i - MERSENNE_DIFF);
        UNROLL(i - MERSENNE_DIFF);
#endif
    }

    {
        // i = 623, last step rolls over
        y = M32(mt_state.MT[MERSENNE_SIZE - 1]) | L31(mt_state.MT[0]);
        mt_state.MT[MERSENNE_SIZE - 1] = mt_state.MT[MERSENNE_PERIOD - 1] ^ (y >> 1) ^ ((((s32)(y) << 31) >>
            31) & MERSENNE_MAGIC);
    }

    // Temper all numbers in a batch
    for (size_t i = 0; i < MERSENNE_SIZE; ++i) {
        y = mt_state.MT[i];
        y ^= y >> 11;
        y ^= y << 7 & 0x9d2c5680;
        y ^= y << 15 & 0xefc60000;
        y ^= y >> 18;
        mt_state.MT_TEMPERED[i] = y;
    }

    mt_state.index = 0;
}

static void mersenne_seed(u32 value)
{
    /*
     * The equation below is a linear congruential generator (LCG), one of the
     * oldest known pseudo-random number generator algorithms, in the form
     * X_(n+1) = = (a*X_n + c) (mod m).
     *
     * We've implicitly got m=32 (mask + word size of 32 bits), so there is no
     * need to explicitly use modulus.
     *
     * What is interesting is the multiplier a.  The one we have below is
     * 0x6c07865 --- 1812433253 in decimal, and is called the Borosh-Niederreiter
     * multiplier for modulus 2^32.
     *
     * It is mentioned in passing in Knuth's THE ART OF COMPUTER
     * PROGRAMMING, Volume 2, page 106, Table 1, line 13.  LCGs are
     * treated in the same book, pp. 10-26
     *
     * You can read the original paper by Borosh and Niederreiter as well.  It's
     * called OPTIMAL MULTIPLIERS FOR PSEUDO-RANDOM NUMBER GENERATION BY THE
     * LINEAR CONGRUENTIAL METHOD (1983) at
     * http://www.springerlink.com/content/n7765ku70w8857l7/
     *
     * You can read about LCGs at:
     * http://en.wikipedia.org/wiki/Linear_congruential_generator
     *
     * From that page, it says: "A common Mersenne twister implementation,
     * interestingly enough, uses an LCG to generate seed data.",
     *
     * Since we're using 32-bits data types for our MT array, we can skip the
     * masking with 0xFFFFFFFF below.
     */

    mt_state.MT[0] = value;
    mt_state.index = MERSENNE_SIZE;

    for (u32 i = 1; i < MERSENNE_SIZE; ++i)
        mt_state.MT[i] = 0x6c078965 * (mt_state.MT[i - 1] ^ mt_state.MT[i - 1] >> 30) + i;
}

static u32 mersenne_rand_u32()
{
    if (mt_state.index == MERSENNE_SIZE) {
        mersenne_generate_numbers();
        mt_state.index = 0;
    }

    return mt_state.MT_TEMPERED[mt_state.index++];
}

//-- dlb_rand.c ----------------------------------------------------------------
void dlb_rand_seed(u32 value)
{
    mersenne_seed(value);
}

u32 dlb_rand_u32()
{
    return mersenne_rand_u32();
}

#endif
#endif
//-- end of implementation -----------------------------------------------------

//-- tests ---------------------------------------------------------------------
#ifdef DLB_RAND_TEST

static void dlb_rand_test() {
    // TODO: Write some tests
}

#endif
//-- end of tests --------------------------------------------------------------
