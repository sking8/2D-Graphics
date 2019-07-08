#include "include/GPath.h"
#include <vector>
#include "include/GPoint.h"
#include "include/GMatrix.h"
#include <iostream>
#include "Utils.h"
#include "include/GMath.h"


GPath& GPath::addRect(const GRect& r, Direction dir){
	GPoint points[4];
	points[0] = GPoint::Make(r.fLeft, r.fTop);
	points[1] = GPoint::Make(r.fRight, r.fTop);
	points[2] = GPoint::Make(r.fRight, r.fBottom);
	points[3] = GPoint::Make(r.fLeft, r.fBottom);

	if (dir == Direction::kCW_Direction) {
		this->moveTo(points[0]);
		for (int i = 1; i < 4; i++) {
			this->lineTo(points[i]);
		}
	}
	else {
		this->moveTo(points[0]);
		for (int i = 3; i > 0; i--) {
			this->lineTo(points[i]);
		}
	}

	return *this;
}

GPath& GPath::addPolygon(const GPoint pts[], int count){
	GASSERT(count > 2);

	this->moveTo(pts[0]);
	for (int i = 1; i < count; i++) {
		this->lineTo(pts[i]);
	}

	return *this;
}




GPath& GPath::addCircle(GPoint center, float radius, Direction direction) {
	GASSERT(radius>0);
	GPoint p0 = { center.fX, center.fY + radius };
	this->moveTo(p0);

	GPoint p1, p2;

	if (direction == Direction::kCW_Direction) {
		p1 = { p0.fX + radius / (1 + sqrtf(2)), p0.fY };
		p2 = { center.fX + radius / sqrtf(2), center.fY + radius / sqrtf(2) };
	}
	else {
		p1 = { p0.fX - radius / (1 + sqrtf(2)), p0.fY };
		p2 = { center.fX - radius / sqrtf(2), center.fY + radius / sqrtf(2) };
	}

	GPoint curr[2] = { p1, p2 };
	
	GMatrix mat;
	
	if (direction == Direction::kCCW_Direction) {
		mat.setRotate(M_PI/4);
	}else{
		mat.setRotate(-M_PI / 4);
	}

	mat.preTranslate(-center.fX, -center.fY);
	mat.postTranslate(center.fX, center.fY);
	
	for (int i = 0; i < 8; i++) {
		this->quadTo(curr[0], curr[1]);
		/*std::cout <<"x: "<< curr[0].fX <<" y: "<< curr[0].fY<<std::endl;
		std::cout << "x: " << curr[1].fX << " y: " << curr[1].fY<<std::endl;*/

		mat.mapPoints(curr, 2);
	}


	return *this;
}


GRect GPath::bounds() const{
	GRect bound;

	if (this->fPts.size() == 0) {
		return bound.MakeWH(0,0);
	}

	float x0 = this->fPts[0].fX;
	float y0 = this->fPts[0].fY;
	float x1 = this->fPts[0].fX;
	float y1 = this->fPts[0].fY;

	for (GPoint point : this->fPts) {
		if (point.fX < x0) {
			x0 = point.fX;
		}
		else if (point.fX > x1) {
			x1 = point.fX;
		}

		if (point.fY < y0) {
			y0 = point.fY;
		}
		else if (point.fY > y1) {
			y1 = point.fY;
		}
	}


	bound.setLTRB(x0,y0,x1,y1);
	return bound;

}

void GPath::transform(const GMatrix& m){
	//GASSERT(this->fPts.size() >= 2);

	GPoint* pts = &fPts[0];
	m.mapPoints(pts, this->fPts.size());
}


void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
	dst[0] = src[0];
	dst[4] = src[2];

	dst[1] = calc_point_with_t(src[0], src[1], t);
	dst[3] = calc_point_with_t(src[1], src[2], t);

	dst[2] = calc_point_with_t(dst[1], dst[3],t);
}

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
	dst[0] = src[0];
	dst[6] = src[3];

	dst[1] = calc_point_with_t(src[0], src[1], t);
	GPoint tmp = calc_point_with_t(src[1], src[2], t);
	dst[5] = calc_point_with_t(src[2], src[3], t);

	dst[2] = calc_point_with_t(dst[1], tmp, t);
	dst[4] = calc_point_with_t(tmp, dst[5], t);
	dst[3] = calc_point_with_t(dst[2], dst[4], t);

}
