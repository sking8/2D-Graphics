#include <stdlib.h>
#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GPaint.h"
#include "include/GPath.h"
#include <iostream>

static void make_regular_poly(GPoint pts[], int count, float cx, float cy, float radius) {
	float angle = 0;
	const float deltaAngle = M_PI * 2 / count;

	for (int i = 0; i < count; ++i) {
		pts[i].set(cx + cos(angle) * radius, cy + sin(angle) * radius);
		angle += deltaAngle;
	}
}


void GDrawSomething_polys(GCanvas* canvas) {
	canvas->clear({ 1, 1, 1, 1 });
	GMatrix scale;
	scale.setScale(100, 100);	

	int count = 10;
	float da = 2 * M_PI / count;
	float angle = 0;
	canvas->translate(256, 256);
	GPoint storage[3];

	for (int i = 0; i < count; ++i) {
		float a = 0.8;
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		
		canvas->save();
		canvas->translate(20*cosf(angle)*i, 20*sinf(angle)*i);
		canvas->rotate(angle);
		make_regular_poly(storage, 3, 0,0,10*i);
		canvas->drawConvexPolygon(storage, 3, GPaint({ a, r, g, b }));

		canvas->restore();
		angle += da;
	}

}