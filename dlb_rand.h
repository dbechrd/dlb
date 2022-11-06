#ifndef DLB_RAND_H
#define DLB_RAND_H
//------------------------------------------------------------------------------
// Copyright 2021 Dan Bechard
//
// Contains 3rd party code, see additional copyright notices below.
//------------------------------------------------------------------------------

//-- header --------------------------------------------------------------------

//-- dlb_rand.h ----------------------------------------------------------------
#include <cinttypes>

struct pcg_state_setseq_64 {    // Internals are *Private*.
    uint64_t state;             // RNG state.  All values are possible.
    uint64_t inc;               // Controls which RNG sequence (stream) is
                                // selected. Must *always* be odd.
};
typedef struct pcg_state_setseq_64 dlb_rand32_t;

// Seed global RNG state (simple API, uses seed as sequence)
void dlb_rand32_seed(uint64_t seed);
// Seed global RNG state. Sequence should be unique for each RNG.
void dlb_rand32_seed_s(uint64_t seed, uint64_t sequence);
// Seed RNG state. Sequence should be unique for each RNG.
void dlb_rand32_seed_r(dlb_rand32_t *rng, uint64_t seed, uint64_t sequence);

// Return random uint in range [0, UINT_MAX]
uint32_t dlb_rand32u_r(dlb_rand32_t *rng);
// Return random uint in range [0, UINT_MAX]
uint32_t dlb_rand32u(void);

// Return random uint in range [min, max], inclusive max
uint32_t dlb_rand32u_range_r(dlb_rand32_t *rng, uint32_t min, uint32_t max);
// Return random uint in range [min, max], inclusive max
uint32_t dlb_rand32u_range(uint32_t min, uint32_t max);

// Return random int in range [INT_MIN, INT_MAX], inclusive max
int32_t dlb_rand32i_r(dlb_rand32_t *rng);
// Return random int in range [INT_MIN, INT_MAX], inclusive max
int32_t dlb_rand32i(void);

// Return random int in range [min, max], inclusive max
int32_t dlb_rand32i_range_r(dlb_rand32_t *rng, int32_t min, int32_t max);
// Return random int in range [min, max], inclusive max
int32_t dlb_rand32i_range(int32_t min, int32_t max);

// Return random float in range [0, 1), exclusive max
float dlb_rand32f_r(dlb_rand32_t *rng);
// Return random float in range [0, 1), exclusive max
float dlb_rand32f(void);

// Return random float in range [min, max), exclusive max
float dlb_rand32f_range_r(dlb_rand32_t *rng, float min, float max);
// Return random float in range [min, max), exclusive max
float dlb_rand32f_range(float min, float max);

// Return random float in range [-variance, +variance), exclusive +variance
float dlb_rand32f_variance_r(dlb_rand32_t *rng, float variance);
// Return random float in range [-variance, +variance), exclusive +variance
float dlb_rand32f_variance(float variance);

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

//-- pcg_basic.h ---------------------------------------------------------------
/*
* PCG Random Number Generation for C.
*
* Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* For additional information about the PCG random number generation scheme,
* including its license and other licensing options, visit
*
*     http://www.pcg-random.org
*/

/*
* This code is derived from the full C implementation, which is in turn
* derived from the canonical C++ PCG implementation. The C++ version
* has many additional features and is preferable if you can use C++ in
* your project.
*/

// pcg32_srandom(initstate, initseq)
// pcg32_srandom_r(rng, initstate, initseq):
//     Seed the rng.  Specified in two parts, state initializer and a
//     sequence selection constant (a.k.a. stream id)
void pcg32_srandom(uint64_t initstate, uint64_t initseq);
void pcg32_srandom_r(dlb_rand32_t *rng, uint64_t initstate, uint64_t initseq);

// pcg32_random()
// pcg32_random_r(rng)
//     Generate a uniformly distributed 32-bit random number
uint32_t pcg32_random(void);
uint32_t pcg32_random_r(dlb_rand32_t *rng);

// pcg32_boundedrand(bound):
// pcg32_boundedrand_r(rng, bound):
//     Generate a uniformly distributed number, r, where 0 <= r < bound
uint32_t pcg32_boundedrand(uint32_t bound);
uint32_t pcg32_boundedrand_r(dlb_rand32_t *rng, uint32_t bound);

//-- pcg_basic.c --------------------------------------------------------
/*
* PCG Random Number Generation for C.
*
* Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* For additional information about the PCG random number generation scheme,
* including its license and other licensing options, visit
*
*       http://www.pcg-random.org
*/

/*
* This code is derived from the full C implementation, which is in turn
* derived from the canonical C++ PCG implementation. The C++ version
* has many additional features and is preferable if you can use C++ in
* your project.
*/

typedef dlb_rand32_t pcg32_random_t;

// If you *must* statically initialize it, here's one.
#define PCG32_INITIALIZER { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL }

// state for global RNGs
static dlb_rand32_t pcg32_global = PCG32_INITIALIZER;

// pcg32_srandom(initstate, initseq)
// pcg32_srandom_r(rng, initstate, initseq):
//     Seed the rng.  Specified in two parts, state initializer and a
//     sequence selection constant (a.k.a. stream id)
void pcg32_srandom_r(pcg32_random_t* rng, uint64_t initstate, uint64_t initseq)
{
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    pcg32_random_r(rng);
    rng->state += initstate;
    pcg32_random_r(rng);
}

void pcg32_srandom(uint64_t seed, uint64_t seq)
{
    pcg32_srandom_r(&pcg32_global, seed, seq);
}

#pragma warning(push)
#pragma warning(disable: 4146)  // "unary minus operator applied to unsigned type"
#pragma warning(disable: 4244)  // "conversion from uint64_t to uint32_t"
// pcg32_random()
// pcg32_random_r(rng)
//     Generate a uniformly distributed 32-bit random number
uint32_t pcg32_random_r(pcg32_random_t* rng)
{
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}
#pragma warning(pop)

uint32_t pcg32_random(void)
{
    return pcg32_random_r(&pcg32_global);
}

#pragma warning(push)
#pragma warning(disable: 4146)  // "unary minus operator applied to unsigned type"
// pcg32_boundedrand(bound):
// pcg32_boundedrand_r(rng, bound):
//     Generate a uniformly distributed number, r, where 0 <= r < bound
uint32_t pcg32_boundedrand_r(pcg32_random_t* rng, uint32_t bound)
{
    // To avoid bias, we need to make the range of the RNG a multiple of
    // bound, which we do by dropping output less than a threshold.
    // A naive scheme to calculate the threshold would be to do
    //
    //     uint32_t threshold = 0x100000000ull % bound;
    //
    // but 64-bit div/mod is slower than 32-bit div/mod (especially on
    // 32-bit platforms).  In essence, we do
    //
    //     uint32_t threshold = (0x100000000ull-bound) % bound;
    //
    // because this version will calculate the same modulus, but the LHS
    // value is less than 2^32.

    uint32_t threshold = -bound % bound;

    // Uniformity guarantees that this loop will terminate.  In practice, it
    // should usually terminate quickly; on average (assuming all bounds are
    // equally likely), 82.25% of the time, we can expect it to require just
    // one iteration.  In the worst case, someone passes a bound of 2^31 + 1
    // (i.e., 2147483649), which invalidates almost 50% of the range.  In
    // practice, bounds are typically small and only a tiny amount of the range
    // is eliminated.
    for (;;) {
        uint32_t r = pcg32_random_r(rng);
        if (r >= threshold)
            return r % bound;
    }
}
#pragma warning(pop)

uint32_t pcg32_boundedrand(uint32_t bound)
{
    return pcg32_boundedrand_r(&pcg32_global, bound);
}

//-- dlb_rand.c ----------------------------------------------------------------

#include <climits>
#include <cmath>

// Seed RNG state. Sequence should be unique for each RNG.
void dlb_rand32_seed_r(dlb_rand32_t *rng, uint64_t seed, uint64_t sequence)
{
    pcg32_srandom_r(rng, seed, sequence);
}

// Seed global RNG state (simple API, uses seed as sequence)
void dlb_rand32_seed(uint64_t seed)
{
    // NOTE: It should be fine to just use seed as the sequence as well for
    // API simplicity even though it eliminates a bunch of entropy. User
    // can call dlb_rand32_seed_seq if they want to get the entropy back.
    dlb_rand32_seed_r(&pcg32_global, seed, seed);
}

// Seed global RNG state. Sequence should be unique for each RNG.
void dlb_rand32_seed_s(uint64_t seed, uint64_t sequence)
{
    dlb_rand32_seed_r(&pcg32_global, seed, sequence);
}

// Return random uint in range [0, UINT_MAX]
uint32_t dlb_rand32u_r(dlb_rand32_t *rng)
{
    const uint32_t randu = pcg32_random_r(rng);
    return randu;
}

// Return random uint in range [0, UINT_MAX]
uint32_t dlb_rand32u(void)
{
    const uint32_t randu = dlb_rand32u_r(&pcg32_global);
    return randu;
}

// Return random uint in range [min, max], inclusive max
uint32_t dlb_rand32u_range_r(dlb_rand32_t *rng, uint32_t min, uint32_t max)
{
    // Map signed ints to unsigned range before subtracting (max - min) to prevent overflow
    const uint32_t range = max - min;
    const uint32_t randu = pcg32_boundedrand_r(rng, range + 1) + min;
    return randu;
}

// Return random int in range [min, max], inclusive max
uint32_t dlb_rand32u_range(uint32_t min, uint32_t max)
{
    const uint32_t randu = dlb_rand32u_range_r(&pcg32_global, min, max);
    return randu;
}

// Return random int in range [INT_MIN, INT_MAX], inclusive max
int32_t dlb_rand32i_r(dlb_rand32_t *rng)
{
    const int32_t randi = (int32_t)pcg32_random_r(rng);
    return randi;
}

// Return random int in range [INT_MIN, INT_MAX], inclusive max
int32_t dlb_rand32i(void)
{
    const int32_t randi = dlb_rand32i_r(&pcg32_global);
    return randi;
}

// Return random int in range [min, max], inclusive max
int32_t dlb_rand32i_range_r(dlb_rand32_t *rng, int32_t min, int32_t max)
{
    // Map signed ints to unsigned range before subtracting (max - min) to prevent overflow
    const uint32_t u_zero = 1U + INT_MAX;
    const uint32_t range = (max + u_zero) - (min + u_zero);
    const int32_t randi = (int32_t)pcg32_boundedrand_r(rng, range + 1) + min;
    return randi;
}

// Return random int in range [min, max], inclusive max
int32_t dlb_rand32i_range(int32_t min, int32_t max)
{
    const int32_t randi = dlb_rand32i_range_r(&pcg32_global, min, max);
    return randi;
}

// Return random int in range [-variance, +variance), exclusive +variance
int dlb_rand32i_variance_r(dlb_rand32_t *rng, int variance)
{
    const int randi = dlb_rand32i_range_r(rng, -variance, variance);
    return randi;
}

// Return random int in range [-variance, +variance), exclusive +variance
int dlb_rand32i_variance(int variance)
{
    const int randi = dlb_rand32i_variance_r(&pcg32_global, variance);
    return randi;
}

// Return random float in range [0, 1), exclusive max
float dlb_rand32f_r(dlb_rand32_t *rng)
{
    const float randf = ldexpf((float)pcg32_random_r(rng), -32);
    return randf;
}

// Return random float in range [0, 1), exclusive max
float dlb_rand32f(void)
{
    const float randf = dlb_rand32f_r(&pcg32_global);
    return randf;
}

// Return random float in range [min, max), exclusive max
float dlb_rand32f_range_r(dlb_rand32_t *rng, float min, float max)
{
    const float randf = ldexpf((float)pcg32_random_r(rng), -32) * (max - min) + min;
    return randf;
}

// Return random float in range [min, max), exclusive max
float dlb_rand32f_range(float min, float max)
{
    const float randf = dlb_rand32f_range_r(&pcg32_global, min, max);
    return randf;
}

// Return random float in range [-variance, +variance), exclusive +variance
float dlb_rand32f_variance_r(dlb_rand32_t *rng, float variance)
{
    const float randf = dlb_rand32f_range_r(rng, -variance, variance);
    return randf;
}

// Return random float in range [-variance, +variance), exclusive +variance
float dlb_rand32f_variance(float variance)
{
    const float randf = dlb_rand32f_variance_r(&pcg32_global, variance);
    return randf;
}

#endif
#endif
//-- end of implementation -----------------------------------------------------

//-- tests ---------------------------------------------------------------------
#ifdef DLB_RAND_TEST

#include <cstdio>
#include <cassert>
#include <cinttypes>
#include <ctime>
#include <cfloat>

void dlb_rand_test(void)
{
    dlb_rand32_seed(time(0));

    int counts[4] = { 0 };
    for (int i = 0; i < 10000; i++) {
        int32_t one_thru_four = dlb_rand32i_range(1, 4);
        assert(one_thru_four >= 1);
        assert(one_thru_four <= 4);
        counts[one_thru_four - 1]++;
    }
#if 0
    printf("\ndlb_rand32i(1, 4) 10000 iterations:\n");
    for (int i = 0; i < 4; i++) {
        printf("[%2d] %2d\n", i + 1, counts[i]);
    }
#endif
    //memset(counts, 0, sizeof(counts));

#if 0
    printf("\ndlb_rand32f(1.0f, 4.0f) 10000 iterations\n");
    float min = FLT_MAX;
    float max = FLT_MIN;
    for (int i = 0; i < 10000; i++) {
        const float randf = dlb_rand32f_range(1.0f, 4.0f);
        if (randf < min) min = randf;
        if (randf > max) max = randf;
    }
    printf("min: %2f\n", min);
    printf("max: %2f\n", max);
#endif
}

#endif
//-- end of tests --------------------------------------------------------------
