/* vec.c
 *
 * Author:
 * Date  :
 *
 * Description
 */

/* Standard C includes */
#include <stdlib.h>
#include <math.h> // For mathematical constants and functions

/* AVX header file */
#include <immintrin.h>

/* Include common headers */
#include "common/macros.h"
#include "common/types.h"
#include "common/vmath.h"
/* Include application-specific headers */
#include "include/types.h"

/* Forward declarations of custom AVX functions */

/* Alternative Implementation */
#pragma GCC push_options
#pragma GCC optimize("O1")
#define inv_sqrt_2xPI 0.39894228040143270286

__m256 cdfnorm256_ps(__m256 x)
{
    // Coefficients for the polynomial approximation
    const __m256 a1 = _mm256_set1_ps(0.319381530f);
    const __m256 a2 = _mm256_set1_ps(-0.356563782f);
    const __m256 a3 = _mm256_set1_ps(1.781477937f);
    const __m256 a4 = _mm256_set1_ps(-1.821255978f);
    const __m256 a5 = _mm256_set1_ps(1.330274429f);
    const __m256 p = _mm256_set1_ps(0.2316419f);
    // Save the sign of x
    // Create a mask where each element is -1.0f (all bits set) if negative, 0.0f (all bits clear) otherwise
    __m256 zero = _mm256_setzero_ps();
    __m256 mask_negative = _mm256_cmp_ps(x, zero, _CMP_LT_OS); // Less Than
    __m256 sign = _mm256_and_ps(mask_negative, _mm256_set1_ps(-1.0f));

    x = _mm256_and_ps(x, _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffffff))); // |x|

    __m256 x2 = _mm256_mul_ps(x, x);
    __m256 exp_term = _mm256_exp_ps(_mm256_mul_ps(x2, _mm256_set1_ps(-0.5f)));
    __m256 xNPrimeofX = _mm256_mul_ps(exp_term, _mm256_set1_ps(inv_sqrt_2xPI));
    __m256 xK2 = _mm256_div_ps(_mm256_set1_ps(1.0f), _mm256_add_ps(_mm256_mul_ps(p, x), _mm256_set1_ps(1.0f)));
    __m256 xK2_2 = _mm256_mul_ps(xK2, xK2);
    __m256 xK2_3 = _mm256_mul_ps(xK2_2, xK2);
    __m256 xK2_4 = _mm256_mul_ps(xK2_3, xK2);
    __m256 xK2_5 = _mm256_mul_ps(xK2_4, xK2);
    __m256 xLocal_1 = _mm256_add_ps(_mm256_add_ps(_mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(xK2_2, a2), _mm256_mul_ps(xK2_3, a3)), _mm256_mul_ps(xK2_4, a4)), _mm256_mul_ps(xK2_5, a5)), _mm256_mul_ps(xK2, a1));
    __m256 xLocal = _mm256_sub_ps(_mm256_set1_ps(1.0f), _mm256_mul_ps(xLocal_1, xNPrimeofX));
    __m256 output = _mm256_blendv_ps(xLocal, _mm256_sub_ps(_mm256_set1_ps(1.0f), xLocal), sign);
    return output;
}

void *impl_vector(void *args)
{
    /* Get the argument struct */
    args_t *parsed_args = (args_t *)args;

    /* Get all the arguments */
    // output
    float *dest = (float *)(parsed_args->output);
    // stock price
    const float *vSptPrice = (const float *)(parsed_args->sptPrice);
    // option strike price
    const float *vStrike = (const float *)(parsed_args->strike);
    // option rate
    const float *vRate = (const float *)(parsed_args->rate);
    // option volatility
    const float *vVolatility = (const float *)(parsed_args->volatility);
    // option time
    const float *vOtime = (const float *)(parsed_args->otime);
    // type of option P for put, C for call
    const char *vOtype = (const char *)(parsed_args->otype);
    // Other arguments like cpu, nthreads are not used in this snippet

    size_t size = parsed_args->num_stocks / 8; // 8 floats per __m256
    size_t total_size = parsed_args->num_stocks; // 8 floats per __m256
    size_t remainder = parsed_args->num_stocks % 8; // Remainder elements

    for (size_t i = 0; i < size; ++i)
    {
        //printf("=====================\n");
        // //printf("Step 1 Iteration %zu\n", i);
        __m256 S = _mm256_load_ps(&vSptPrice[i * 8]);
        //_mm256_store_ps(test_result, S);
        //printf("sptPrice: %f\n", test_result[0]);
        // //printf("Step 2 Iteration %zu\n", i);
        __m256 K = _mm256_load_ps(&vStrike[i * 8]);
        //_mm256_store_ps(test_result, K);
        //printf("strike: %f\n", test_result[0]);
        // //printf("Step 3 Iteration %zu\n", i);
        __m256 r = _mm256_load_ps(&vRate[i * 8]);
        // //printf("Step 4 Iteration %zu\n", i);
        __m256 v = _mm256_load_ps(&vVolatility[i * 8]);
        // //printf("Step 5 Iteration %zu\n", i);
        __m256 T = _mm256_load_ps(&vOtime[i * 8]);
        // //printf("Step 6 Iteration %zu\n", i);
        // __m256 Y = _mm256_load_ps(&vOtype[i * 8]);
        // //printf("Step 7 Iteration %zu\n", i);

        // Black-Scholes calculations
        __m256 sqrtT = _mm256_sqrt_ps(T);
        //_mm256_store_ps(test_result, sqrtT);
        //printf("xSqrtTime: %f\n", test_result[0]);
        // //printf("Step 8 Iteration %zu\n", i);
        __m256 S_over_K = _mm256_div_ps(S, K);
        __m256 log_result = _mm256_log_ps(S_over_K);
        //_mm256_store_ps(test_result, log_result);
        //printf("logValues: %f\n", test_result[0]);
        __m256 v_squared = _mm256_mul_ps(v, v);
        //_mm256_store_ps(test_result, v_squared);
        //printf("xPowerTerm: %f\n", test_result[0]);
        __m256 v_squared_5 = _mm256_mul_ps(v_squared, _mm256_set1_ps(0.5f));
        //_mm256_store_ps(test_result, v_squared_5);
        //printf("xPowerTerm: %f\n", test_result[0]);
        __m256 r_plus_v_squared = _mm256_add_ps(r, v_squared_5);
        //_mm256_store_ps(test_result, r_plus_v_squared);
        //printf("xD1: %f\n", test_result[0]);
        __m256 r_plus_v_squared_times_T = _mm256_mul_ps(r_plus_v_squared, T);
        //_mm256_store_ps(test_result, r_plus_v_squared_times_T);
        //printf("xD1: %f\n", test_result[0]);
        __m256 numerator = _mm256_add_ps(log_result, r_plus_v_squared_times_T);
        //_mm256_store_ps(test_result, numerator);
        //printf("xD1: %f\n", test_result[0]);
        __m256 v_times_sqrtT = _mm256_mul_ps(v, sqrtT);
        //_mm256_store_ps(test_result, v_times_sqrtT);
        //printf("xDen: %f\n", test_result[0]);
        __m256 d1 = _mm256_div_ps(numerator, v_times_sqrtT);
        //_mm256_store_ps(test_result, d1);
        //printf("d1: %f\n", test_result[0]);
        __m256 d2 = _mm256_sub_ps(d1, v_times_sqrtT);
        //_mm256_store_ps(test_result, d2);
        //printf("d2: %f\n", test_result[0]);

        // Calculate option price
        __m256 cndD1 = cdfnorm256_ps(d1);
        //_mm256_store_ps(test_result, cndD1);
        //printf("NofXd1: %f\n", test_result[0]);
        __m256 cndD2 = cdfnorm256_ps(d2);
        //_mm256_store_ps(test_result, cndD2);
        //printf("NofXd2: %f\n", test_result[0]);

        __m256 expRT = _mm256_exp_ps(_mm256_mul_ps(_mm256_sub_ps(_mm256_setzero_ps(), r), T));
        // __m256 expRT = _mm256_mul_ps(K,_mm256_exp_ps(_mm256_mul_ps(_mm256_sub_ps(_mm256_setzero_ps(), r), T)));
        //_mm256_store_ps(test_result, expRT);
        //printf("FutureValueX: %f\n", test_result[0]);
        __m256 callResult = _mm256_sub_ps(_mm256_mul_ps(S, cndD1),
                                          _mm256_mul_ps(K, _mm256_mul_ps(expRT, cndD2)));
        // __m256 callResult = _mm256_sub_ps(_mm256_mul_ps(S, cndD1), _mm256_mul_ps(expRT,cndD2)) ;
        __m256 putResult = _mm256_sub_ps(_mm256_mul_ps(K, _mm256_mul_ps(expRT, _mm256_sub_ps(_mm256_set1_ps(1.0f), cndD2))),
                                         _mm256_mul_ps(S, _mm256_sub_ps(_mm256_set1_ps(1.0f), cndD1)));

        // Store result
        // __m256 result = _mm256_blendv_ps(callResult, putResult, Y);

        // float result_c_arr[8];
        // float result_p_arr[8];
        // _mm256_store_ps(result_c_arr, callResult);
        // _mm256_store_ps(result_p_arr, putResult);

        //printf("Result C: %f\n", result_c_arr[0]);
        //printf("Result P: %f\n", result_p_arr[0]);
        //printf("Otype: %c\n", vOtype[i]);
        // if otype == "p" then output = putResult else output = callResult
        // Create a mask based on the otype
        int mask[8];
        for (int j = 0; j < 8; j++)
        {
            // //printf("mask[%d]: %c\n", j, vOtype[j]);
            mask[j] = (vOtype[i * 8 + j] == 'P') ? 0xFFFFFFFF : 0x00000000; // All bits set for 'P', all bits clear otherwise
        }

        // Blend results: if mask is 1, choose putResult, else choose callResult
        // __m256 result = _mm256_blendv_ps(callResult, putResult);
        // Convert integer mask to __m256
        // Inside your function

         __m256 mask_vec = _mm256_castsi256_ps(_mm256_loadu_si256((__m256i *)mask));

        // Blend results: if mask bit is set, choose putResult, else choose callResult

        __m256 result = _mm256_blendv_ps(callResult, putResult, mask_vec);

        //printf("=====================\n");
        _mm256_store_ps(&dest[i * 8], result);
    }
    
    for (remainder; remainder > 0; remainder =0)
    {
        int i = total_size - remainder;
        __m256 S = _mm256_load_ps(&vSptPrice[i * 8]);
        __m256 K = _mm256_load_ps(&vStrike[i * 8]);
        __m256 r = _mm256_load_ps(&vRate[i * 8]);
        __m256 v = _mm256_load_ps(&vVolatility[i * 8]);
        __m256 T = _mm256_load_ps(&vOtime[i * 8]);

        // Black-Scholes calculations
        __m256 sqrtT = _mm256_sqrt_ps(T);
        __m256 S_over_K = _mm256_div_ps(S, K);
        __m256 log_result = _mm256_log_ps(S_over_K);
        __m256 v_squared = _mm256_mul_ps(v, v);
        __m256 v_squared_5 = _mm256_mul_ps(v_squared, _mm256_set1_ps(0.5f));
        __m256 r_plus_v_squared = _mm256_add_ps(r, v_squared_5);
        __m256 r_plus_v_squared_times_T = _mm256_mul_ps(r_plus_v_squared, T);
        __m256 numerator = _mm256_add_ps(log_result, r_plus_v_squared_times_T);
        __m256 v_times_sqrtT = _mm256_mul_ps(v, sqrtT);
        __m256 d1 = _mm256_div_ps(numerator, v_times_sqrtT);
        __m256 d2 = _mm256_sub_ps(d1, v_times_sqrtT);
        // Calculate option price
        __m256 cndD1 = cdfnorm256_ps(d1);
        __m256 cndD2 = cdfnorm256_ps(d2);
        __m256 expRT = _mm256_exp_ps(_mm256_mul_ps(_mm256_sub_ps(_mm256_setzero_ps(), r), T));
        __m256 callResult = _mm256_sub_ps(_mm256_mul_ps(S, cndD1),
                                          _mm256_mul_ps(K, _mm256_mul_ps(expRT, cndD2)));
        __m256 putResult = _mm256_sub_ps(_mm256_mul_ps(K, _mm256_mul_ps(expRT, _mm256_sub_ps(_mm256_set1_ps(1.0f), cndD2))),
                                         _mm256_mul_ps(S, _mm256_sub_ps(_mm256_set1_ps(1.0f), cndD1)));

        int mask[8];
        for (int j = 0; j < 8; j++)
        {
            mask[j] = (vOtype[i * 8 + j] == 'P') ? 0xFFFFFFFF : 0x00000000; // All bits set for 'P', all bits clear otherwise
        }

        // Convert integer mask to __m256
        // Inside your function

         __m256 mask_vec = _mm256_castsi256_ps(_mm256_loadu_si256((__m256i *)mask));

        // Blend results: if mask bit is set, choose putResult, else choose callResult

        __m256 result = _mm256_blendv_ps(callResult, putResult, mask_vec);
        _mm256_store_ps(&dest[i * 8], result);
    }
    
    return NULL;
}
#pragma GCC pop_options
