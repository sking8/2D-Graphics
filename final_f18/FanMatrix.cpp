#include "include/GColor.h"
#include "include/GMath.h"
#include "include/GPoint.h"
#include "include/GRect.h"
#include "include/GMatrix.h"
#include <math.h>
#include <iostream>



void GMatrix::setIdentity(){
	this->set6(1,0,0,0,1,0);
}

void GMatrix::setTranslate(float tx, float ty){
	this->set6(1, 0, tx, 0, 1, ty);
}

void GMatrix::setScale(float sx, float sy){
	this->set6(sx, 0, 0, 0, sy, 0);
}

void GMatrix::setRotate(float radians){
	this->set6(cos(radians), -sin(radians), 0, sin(radians), cos(radians),0);
}

void GMatrix::setConcat(const GMatrix& secundo, const GMatrix& primo){
	float a, b, c, d, e, f;
	a = secundo[0] * primo[0] + secundo[1] * primo[3];
	b = secundo[0] * primo[1] + secundo[1] * primo[4];
	c = secundo[0] * primo[2] + secundo[1] * primo[5]+secundo[2];
	d = secundo[3] * primo[0] + secundo[4] * primo[3];
	e = secundo[3] * primo[1] + secundo[4] * primo[4];
	f = secundo[3] * primo[2] + secundo[4] * primo[5]+secundo[5];

	this->set6(a, b, c, d, e, f);

}

bool GMatrix::invert(GMatrix* inverse) const{
	float det = (*this)[0] * (*this)[4] - (*this)[1] * (*this)[3];
	if (det == 0) {
		return false;
	}

	float a, b, c, d, e, f;
	a = (*this)[4] / det;
	b = -(*this)[1] / det;
	c = ((*this)[1] * (*this)[5] - (*this)[2] * (*this)[4])/det;
	d = -(*this)[3] / det;
	e = (*this)[0] / det;
	f = ((*this)[2] * (*this)[3] - (*this)[0] * (*this)[5]) / det;

	inverse->set6(a, b, c, d, e, f);
	
	

	return true;
}


void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const{
	for (int i = 0; i < count; i++) {
		const GPoint& tmp = src[i];
		float a,b;
		a = (*this)[0] * tmp.fX + (*this)[1] * tmp.fY + (*this)[2];
		b = (*this)[3] * tmp.fX + (*this)[4] * tmp.fY + (*this)[5];
		dst[i].fX = a;
		dst[i].fY = b;
	}
}
