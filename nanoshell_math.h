#pragma once

#define INFINITY (1.0f / 0.0f)

double copysign(double val, double sgn);
double trunc(double f);
double floor(double f);
double ceil(double f);
double round(double f);
double sqrt(double x);
double sin(double x);
double cos(double x);
double tan(double f);
double atan2(double y, double x);
double log2(double x);
double frexp(double x, int *exp);
double pow(double x, double y);
double log10(double x);
int signbit(double d);
int isfinite(double x);

#define isnan(x) ((x) != (x))
#define isunordered(x, y) (isnan(x) || isnan(y))
