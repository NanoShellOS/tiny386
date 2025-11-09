#include <stdint.h>
#include "nanoshell_math.h"

// TODO: Implement a complete math library and do this properly.
int signbit(double d) {
	union {
		double f;
		int64_t i;
	} z;
	z.f = d;
	return z.i & (1ULL << 63);
}

double copysign(double val, double sgn) {
	union {
		double f;
		int64_t i;
	} z, v;
	z.f = val;
	v.f = sgn;
	z.i = (z.i & ~(1ULL << 63)) | (v.i & (1ULL << 63));
	return z.f;
}

double trunc(double f) {
    return (double)((int64_t)f);
}

double floor(double f) {
	int64_t i = (int64_t)f;
	if (f < 0.0 && f != (double)i)
		i -= 1;
	return (double)i;
}

double ceil(double f) {
	int64_t i = (int64_t)f;
	if (f > 0.0 && f != (double)i)
		i += 1;
	return (double)i;
}

double round(double f) {
	if (f >= 0.0)
		return floor(f + 0.5);
	else
		return ceil(f - 0.5);
}

double sqrt(double x) {
    double result;
    __asm__ volatile("fsqrt" : "=t"(result) : "0"(x));
    return result;
}

double sin(double x) {
    double result;
    __asm__ volatile("fsin" : "=t"(result) : "0"(x));
    return result;
}

double cos(double x) {
    double result;
    __asm__ volatile("fcos" : "=t"(result) : "0"(x));
    return result;
}

double tan(double f) {
	double c = cos(f);
	if (!c) return INFINITY;
	return sin(f) / c;
}

double atan2(double y, double x) {
	double res;
	__asm__ __volatile__ (
		"fldl %1\n\t"    /* ST0 = y */
		"fldl %2\n\t"    /* ST0 = x, ST1 = y */
		"fpatan\n\t"     /* ST0 = atan(y/x) with quadrant, pop -> result in ST0 */
		"fstpl %0\n\t"   /* store result -> res, pop */
		: "=m" (res)
		: "m" (y), "m" (x)
		: "st"
	);
	return res;
}

double log2(double x) {
	double res;
	__asm__ __volatile__ (
		"fld1\n\t"       /* ST0 = 1.0 */
		"fldl %1\n\t"    /* ST0 = x, ST1 = 1.0 */
		"fyl2x\n\t"      /* ST0 := ST1 * log2(ST0) ; pops -> result in ST0 */
		"fstpl %0\n\t"   /* store result into res */
		: "=m" (res)
		: "m" (x)
		: "st"
	);
	return res;
}

double frexp(double x, int *exp) {
	union {
		double d;
		uint64_t u;
	} v = { x };

	uint64_t sign = v.u & 0x8000000000000000ULL;
	uint64_t exponent = (v.u >> 52) & 0x7FFULL;
	uint64_t mantissa = v.u & 0xFFFFFFFFFFFFFULL; // 52-bit mantissa

	if (exponent == 0) {  // Zero or subnormal
		if (mantissa == 0) {
			*exp = 0;
			return 0.0;
		}

		// Normalize subnormal
		int shift = 0;
		while ((mantissa & 0x10000000000000ULL) == 0) { // MSB of mantissa
			mantissa <<= 1;
			shift++;
		}
		mantissa &= 0xFFFFFFFFFFFFFULL;  // remove implicit 1
		exponent = 1 - shift;            // adjusted exponent
	}

	*exp = (int)exponent - 1022;  // bias=1023, fraction in [0.5,1)
	// Rebuild double with exponent = 1022 (so fraction = mantissa in [0.5,1))
	v.u = sign | (1022ULL << 52) | mantissa;

	return v.d;
}
double pow(double x, double y) {
	double result;
	__asm__ __volatile__ (
		"fldl %1\n\t"      /* ST0 = x */
		"fldl %2\n\t"      /* ST0 = y, ST1 = x */
		"fyl2x\n\t"        /* ST0 = y*log2(x), pop -> ST0 */
		"fld %%st(0)\n\t"  /* duplicate ST0 */
		"frndint\n\t"      /* integer part in ST0 */
		"fsub %%st(1), %%st(0)\n\t"  /* ST0 = fractional part */
		"f2xm1\n\t"        /* compute 2^fractional - 1 */
		"fld1\n\t"
		"faddp %%st(1), %%st(0)\n\t" /* 2^fractional */
		"fscale\n\t"       /* multiply by 2^integer_part */
		"fstp %%st(1)\n\t" /* pop extra copy */
		"fstpl %0\n\t"
		: "=m"(result)
		: "m"(x), "m"(y)
		: "st"
	);
	return result;
}

int isfinite(double x) {
	union {
		double d;
		uint64_t u;
	} v = { x };

	uint64_t exponent = (v.u >> 52) & 0x7FFULL;
	return exponent != 0x7FFULL;
}

double log10(double x) {
	const double LOG2_10 = 3.321928094887362;  // log2(10)
	return log2(x) / LOG2_10;
}