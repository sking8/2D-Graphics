#ifndef Utils_DEFINED
#define Utils_DEFINED

#include "GRect.h"
#include "include/GPixel.h"
#include "include/GMath.h"
#include "include/GColor.h"
#include "include/GPoint.h"

static inline unsigned div255(unsigned x) {
	x += 128;
	return x + (x >> 8) >> 8;
}

static inline bool cover(const float& x, const float& y, const GRect& rect) {
	return rect.fLeft < x && x <= rect.fRight && rect.fTop < y && y <= rect.fBottom;
}

// turn 0xAABBCCDD into 0x00AA00CC00BB00DD
static uint64_t expand(uint32_t x) {
	uint64_t hi = x & 0xFF00FF00; // the A and G components
	uint64_t lo = x & 0x00FF00FF; // the R and B components
	return (hi << 24) | lo;
}

// turn 0xXX into 0x00XX00XX00XX00XX
static uint64_t replicate(uint64_t x) {
	return (x << 48) | (x << 32) | (x << 16) | x;
}
// turn 0x..AA..CC..BB..DD into 0xAABBCCDD
static  uint32_t compact(uint64_t x) {
	return ((x >> 24) & 0xFF00FF00) | (x & 0xFF00FF);
}

static uint32_t quad_mul_div255(uint32_t x, uint8_t invA) {
	uint64_t prod = expand(x) * invA;
	prod += replicate(128);
	prod += (prod >> 8) & replicate(0xFF);
	prod >>= 8;
	return compact(prod);
}


static uint64_t quad_mul(uint32_t x, uint8_t invA) {
	uint64_t prod = expand(x) * invA;
	return prod;
}

static uint32_t quad_div255(uint64_t prod) {
	prod += replicate(128);
	prod += (prod >> 8) & replicate(0xFF);
	prod >>= 8;
	return compact(prod);
}

static float calculate_dy(float dx, float slope) {
	float dy;
	dy = dx / slope;
	return dy;
}

static float calculate_dx(float dy, float slope) {
	float dx;
	dx = slope * dy;
	return dx;
}

static GPixel color_to_pixel(const GColor& color) {

	int a = GRoundToInt(GPinToUnit(color.fA) * 255);
	int r = GRoundToInt(GPinToUnit(color.fR*color.fA) * 255);
	int g = GRoundToInt(GPinToUnit(color.fG*color.fA) * 255);
	int b = GRoundToInt(GPinToUnit(color.fB*color.fA) * 255);
	GPixel source = GPixel_PackARGB(a, r, g, b);

	return source;
}

static GPoint calc_point_with_t (const GPoint& p0, const GPoint& p1, float t){
	GPoint out;

	out = p0 + (p1 - p0)*t;

	return out;
}

static GPoint calc_point_with_t(const GPoint& p0, const GPoint& p1, const GPoint& p2, float t) {
	GPoint out;
	
	GPoint a = GPoint(p0+p2-2*p1);
	GPoint b = GPoint(2 * (p1 - p0));
	
	out = (a*t + b)*t + p0;

	return out;
}

static GPoint calc_point_with_t(const GPoint& p0, const GPoint& p1, const GPoint& p2, const GPoint& p3, float t) {
	GPoint out;

	out = pow(1 - t, 3)*p0 + 3 * t*pow(1 - t, 2)*p1 + 3 * pow(t, 2)*(1 - t)*p2 + pow(t, 3)*p3;

	return out;
}

static GColor Cadd(const GColor a,const GColor b) {
	return GColor::MakeARGB(a.fA + b.fA, a.fR + b.fR, a.fG + b.fG, a.fB + b.fB);
}

static GColor Cminus(const GColor a, const GColor b) {
	return GColor::MakeARGB(a.fA - b.fA, a.fR - b.fR, a.fG - b.fG, a.fB - b.fB);
}


static GColor Cmul(float f, const GColor c) {
	return GColor::MakeARGB(f*c.fA,f*c.fR,f*c.fG,f*c.fB);
}

static GPixel Pmul(const GPixel p1, const GPixel p2) {
	int a = div255(GPixel_GetA(p1)*GPixel_GetA(p2));
	int r = div255(GPixel_GetR(p1)*GPixel_GetR(p2));
	int g = div255(GPixel_GetG(p1)*GPixel_GetG(p2));
	int b = div255(GPixel_GetB(p1)*GPixel_GetB(p2));

	return GPixel_PackARGB(a,r,g,b);

}

static float calc_dist(GPoint p0, GPoint p1) {
	return sqrtf((p0.fX - p1.fX)*(p0.fX - p1.fX) + (p0.fY - p1.fY)*(p0.fY - p1.fY));
}


#endif