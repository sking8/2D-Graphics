#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GBitmap.h"
#include "include/GMath.h"
#include "Utils.h"
#include "include/GColor.h"
#include "ShadeMode.h"
#include <vector>
#include <iostream>


class FanShader : public GShader{

public:

	FanShader(const GBitmap& device, const GMatrix& localMatrix, GShader::TileMode mode) {
		fDevice = device;
		scale.setScale(device.width(), device.height());
		FanShader::localMatrix =  localMatrix;
		this->mode = mode;
	}

	bool isOpaque() {
		bool result = true;

		for (int y = 0; y < fDevice.height(); ++y) {
			GPixel* row = fDevice.getAddr(0, y);
			for (int x = 0; x < fDevice.width(); ++x) {
				if (GPixel_GetA(row[x]) != 0) {
					result = false;
				}
			}
		}

		return result;
	}
	 
	bool setContext(const GMatrix& ctm) {
		// fInverse is a private matrix stored in MyShader.
		GMatrix tmp;
		tmp.setConcat(ctm,localMatrix);
		tmp.preConcat(scale);
		return tmp.invert(&fInverse);
	}

	void shadeRow(int x, int y, int count, GPixel row[]) {

		/*const float dx = fInverse[GMatrix::SX];
		const float dy = fInverse[GMatrix::KY];*/
		GPoint local;

		for (int i = 0; i < count; ++i) {
			local = fInverse.mapXY(x + i+ 0.5, y + 0.5);
			
			// clip points into the canvas
			(*shadeMode[mode])(local.fX);
			(*shadeMode[mode])(local.fY);
			
			local = scale.mapXY(local.fX, local.fY);

			// use local to lookup/compute a color
			row[i] =*fDevice.getAddr((int)local.fX, (int)local.fY);

		}
	}

private:
	GMatrix localMatrix;
	GMatrix inverseScale;
	GMatrix fInverse;
	GMatrix scale;
	GBitmap fDevice;
	GShader::TileMode mode;
};

class LinearShader : public GShader {

public:
	LinearShader(GPoint p0, GPoint p1, const GColor* colors, int count, GShader::TileMode mode) {
		this->count = count;
		
		float dx = p1.fX - p0.fX;
		float dy = p1.fY - p0.fY;
		localMatrix.set6(dx,-dy,p0.fX,dy,dx,p0.fY);
		this->mode = mode;
		
		
		for (int i = 0; i < count; i++) {
			this->colors.push_back(colors[i]);
		}

	}

	bool isOpaque() {

		bool result = true;

		for (int i=0; i<count; ++i) {
			if (colors[i].fA != 0) {
				result = false;
			}
		}
		return result;

	}

	bool setContext(const GMatrix& ctm) {
		GMatrix tmp;
		tmp.setConcat(ctm, localMatrix);

		return tmp.invert(&fInverse);
	}

	void shadeRow(int x, int y, int count, GPixel* row) {

		//for (int i = 0; i < this->count; i++) {
		//	//std::cout << colors[i].fA << " " << colors[i].fR << " " << colors[i].fG << " " << colors[i].fB << std::endl;
		//	if (colors[i].fG == 100) {
		//		std::cout << " 100 !" << std::endl;
		//		std::cout << "y " << y << std::endl;
		//	}
		//
		//}


		for (int i = 0; i < count; i++) {
			GPoint local = fInverse.mapXY( x+i + 0.5, y + 0.5);
			float mx = local.fX;

			(*shadeMode[mode])(mx);

			/*std::cout << "mx: "<<mx << std::endl;*/

			int intervals = this->count - 1;
			
			int left = GFloorToInt(mx*intervals);
			int right = GCeilToInt(mx*intervals);


			/*std::cout << "left: "<<left << std::endl;
			std::cout << "right: " << right<< std::endl;*/
		
			float u = mx * intervals - left;
			float v = 1 - u;
			GColor out_c;

			float a = v * this->colors[left].fA + u * this->colors[right].fA;
			float r = v * this->colors[left].fR + u * this->colors[right].fR;
			float g = v * this->colors[left].fG + u * this->colors[right].fG;
			float b = v * this->colors[left].fB + u * this->colors[right].fB;

			out_c = GColor::MakeARGB(a,r,g,b);

			//std::cout << "out_c A: " << out_c.fA << ", out_c R: " << out_c.fR << ", out_c G: " << out_c.fG << ", out_c B: " << out_c.fB << std::endl;
			/*if (out_c.fG == 99.5) {
				for (int i = 0; i < this->count; i++) {
					std::cout << colors[i].fA << " " << colors[i].fR << " " << colors[i].fG << " " << colors[i].fB << std::endl;
				}

				std::cout << "u " << u << " v: " << v<< std::endl;
				std::cout << "left: " << left << std::endl;
				std::cout << "right: " << right << std::endl;
				std::cout << "count: " << this->count << std::endl;
				std::cout << "left G " << colors[left].fG << "right G" << colors[right].fG << std::endl;
			}*/


			GPixel out_p = color_to_pixel(out_c);

			//std::cout << "out_p A: " << GPixel_GetA(out_p) << ", out_p R: " << GPixel_GetR(out_p) << ", out_p G: " << GPixel_GetG(out_p) << ", out_p B: " << GPixel_GetB(out_p) << std::endl;


			row[i] = out_p;
		}
	}


private:
	GMatrix localMatrix;
	GMatrix fInverse;
	int count;
	std::vector<GColor> colors;
	GShader::TileMode mode;
};

class SingleShader : public GShader {
public:
	SingleShader( GColor color) {
		this->color = color;
	}

	bool isOpaque() {
		if (this->color.fA == 0) {
			return true;
		}
		return false;
	}

	void shadeRow(int x, int y, int count, GPixel* row) {
		GPixel out = color_to_pixel(this->color);
		for (int i = 0; i < count; i++) {
			row[i] = out;
		}
	}

	bool setContext(const GMatrix& ctm) {
		return true;
	}


private:
	GColor color;

};

class TricolorShader :public GShader {
public:
	TricolorShader(GPoint points[],GColor colors[]) {
		for (int i = 0; i < 3; i++) {
			this->colors[i] = colors[i];
		}

		GVector u = points[1] - points[0];
		GVector v = points[2] - points[0];
		this->localMatrix.set6(u.fX,v.fX,points[0].fX,u.fY,v.fY,points[0].fY);

		DC1 = Cminus(colors[1], colors[0]);
		DC2 = Cminus(colors[2], colors[0]);
	}

	bool isOpaque() {
		for (int i = 0; i < 3; i++) {
			if (colors[i].fA == 0.0) {
				return true;
			}	
		}
		return false;
	}

	bool setContext(const GMatrix& ctm) {
		GMatrix tmp;
		tmp.setConcat(ctm, localMatrix);
		return tmp.invert(&fInverse);
	}

	void shadeRow(int x, int y, int count, GPixel row[]) {
		GPoint local=fInverse.mapXY(x+0.5, y+0.5);
		GColor color;
		color = Cadd(Cadd(Cmul(local.fX , DC1) , Cmul(local.fY,DC2)) , colors[0]);
		float a = fInverse[GMatrix::SX];
		float d = fInverse[GMatrix::KY];
		GColor DC = Cadd(Cmul(a, DC1), Cmul(d, DC2));

		for (int i = 0; i < count; i++) {
			row[i] = color_to_pixel(color);
			color = Cadd(color, DC);
		}
	}

private:
	GMatrix localMatrix;
	GMatrix fInverse;
	GColor colors[3];
	GColor DC1;
	GColor DC2;
};

class ProxyShader :public GShader {
public:
	ProxyShader(GShader* real,const GPoint points[3],const GPoint tex[3]) {
		this->real = real;
		GMatrix T, P;

		GVector u = tex[1] - tex[0];
		GVector v = tex[2] - tex[0];
		T.set6(u.fX, v.fX, tex[0].fX, u.fY, v.fY, tex[0].fY);

		u = points[1] - points[0];
		v = points[2] - points[0];
		P.set6(u.fX, v.fX, points[0].fX, u.fY, v.fY, points[0].fY);
		
		T.invert(&localMatrix);
		localMatrix.postConcat(P);
	}

	bool isOpaque() {
		return real->isOpaque();
	}

	bool setContext(const GMatrix& ctm) {
		GMatrix tmp;
		tmp.setConcat(ctm, localMatrix);
		return real->setContext(tmp);
	}

	void shadeRow(int x, int y, int count, GPixel row[]) {
		real->shadeRow(x, y, count, row);
	}


private:
	GShader* real;
	GMatrix localMatrix;
};

class ComposeShader : public GShader {
public:
	ComposeShader(GShader* s1, GShader* s2) {
		this->s1 = s1;
		this->s2 = s2;
	}

	bool isOpaque() {
		return (s1->isOpaque() && s2->isOpaque());
	}

	bool setContext(const GMatrix& ctm) {
		return (s1->setContext(ctm) && s2->setContext(ctm));
	}

	void shadeRow(int x, int y, int count, GPixel* row) {
		GPixel A[count];
		GPixel B[count];
		s1->shadeRow(x, y, count, A);
		s2->shadeRow(x, y, count, B);
		
		for (int i = 0; i < count; i++) {
			row[i] = Pmul(A[i], B[i]);
		}

	}

private:
	GShader* s1;
	GShader* s2;
};

class RadialShader :public GShader {
public:
	RadialShader(GPoint center, float radius,
		const GColor colors[], int count, GShader::TileMode mode) {

		localMatrix.set6(radius, 0, center.fX, 0, radius, center.fY);
		this->count = count;

		for (int i = 0; i < count; i++) {
			this->colors.push_back(colors[i]);
		}

		this->mode = mode;
	}

	bool isOpaque() {
		for (int i = 0; i < count; i++) {
			if (colors[i].fA == 0) {
				return false;
			}
		}
		return true;
	}

	bool setContext(const GMatrix& ctm) {
		GMatrix tmp;
		tmp.setConcat(ctm, localMatrix);

		return tmp.invert(&fInverse);
	}


	void shadeRow(int x, int y, int count, GPixel row[]) {
		GPoint local = fInverse.mapXY(x + 0.5, y + 0.5);
		float dx = fInverse[GMatrix::SX];
		float dy = fInverse[GMatrix::KY];
		GPoint origin = GPoint::Make(0, 0);
		int intervals = this->count - 1;
		GColor out_c;
		GPixel out_p;

		for (int i = 0; i < count; i++) {
			float d = calc_dist(local, origin);

			(*shadeMode[mode])(d);

			int left = GFloorToInt(d*intervals);
			int right = GCeilToInt(d*intervals);

			float u = d * intervals - left;
			float v = 1 - u;
			

			float a = v * this->colors[left].fA + u * this->colors[right].fA;
			float r = v * this->colors[left].fR + u * this->colors[right].fR;
			float g = v * this->colors[left].fG + u * this->colors[right].fG;
			float b = v * this->colors[left].fB + u * this->colors[right].fB;

			out_c = GColor::MakeARGB(a, r, g, b);
			out_p = color_to_pixel(out_c);

			row[i] = out_p;

			local.fX += dx;
			local.fY += dy;
		}
	}

private:
	int count;
	std::vector<GColor> colors;
	GShader::TileMode mode;
	GMatrix localMatrix;
	GMatrix fInverse;
};




std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& device, const GMatrix& localMatrix, GShader::TileMode mode) {
	if (&device==nullptr || &localMatrix==nullptr) {
		return nullptr;
	}

	return std::unique_ptr<GShader>(new FanShader(device,localMatrix,mode));
}

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor* colors, int count, GShader::TileMode mode){
	if (count < 1) {
		return nullptr;
	}

	if (count == 1) {
		return std::unique_ptr<GShader>(new SingleShader(colors[0]));
	}

	return std::unique_ptr<GShader>(new LinearShader(p0,p1,colors,count,mode));
}
