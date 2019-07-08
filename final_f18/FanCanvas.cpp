#include "GColor.h"
#include "include/GPaint.h"
#include "include/GCanvas.h"
#include "include/GMath.h"
#include "include/GPixel.h"
#include "include/GBitmap.h"
#include "include/GColor.h"
#include "include/GTypes.h"
#include "include/GPath.h"
#include "Utils.h"
#include "GEdge.h"
#include "FanBlendMode.h"
#include "include/GShader.h"
#include "include/GPoint.h"
#include <iostream>
#include <stack> 
#include <vector>
#include <list>
#include <algorithm>
#include <iterator>
#include "FanShader.h"



class FanCanvas : public GCanvas {
public:

	FanCanvas(const GBitmap& device) : fDevice(device) {
		CTM_stack.push(GMatrix());
	}

	std::unique_ptr<GShader> final_createRadialGradient(GPoint center, float radius,
		const GColor colors[], int count, GShader::TileMode mode) {

		return std::unique_ptr<GShader>(new RadialShader(center, radius,colors, count, mode));
	}

	void save() override {
		GMatrix tmp = CTM_stack.top();
		CTM_stack.push(tmp);
	}

	void restore() override {
		CTM_stack.pop();
	}

	void concat(const GMatrix& matrix) override {
		CTM_stack.top().setConcat(CTM_stack.top(), matrix);
	}



	void drawPaint(const GPaint& paint) override {

		

		if (paint.getShader()) {
			// use the shader instead of the paint�s color
			if (!paint.getShader()->setContext(CTM_stack.top())) {
				// nothing to draw if the shader fails
				return;
			}

			paint.getShader()->setContext(CTM_stack.top());
			GShader* shader = paint.getShader();
			GPixel storage[fDevice.width()];

			for (int y = 0; y < fDevice.height(); ++y) {
				GPixel* row = fDevice.getAddr(0, y);
				shader->shadeRow(0, y, fDevice.width(), storage);

				for (int x = 0; x < fDevice.width(); x++) {
					row[x] = (*BlendProc[static_cast<int>(paint.getBlendMode())])(storage[x], row[x]);
				}

			}

			return;
		}

		GPixel source = color_to_pixel(paint.getColor());

		for (int y = 0; y < fDevice.height(); ++y) {
			GPixel* row = fDevice.getAddr(0, y);
			for (int x = 0; x < fDevice.width(); ++x) {
				row[x] = (*BlendProc[static_cast<int>(paint.getBlendMode())])(source, row[x]);
			}
		}
	}

	void drawRect(const GRect& rect, const GPaint& paint) override {

		//send to draw polygon
		GPoint points[4];
		points[0].set(rect.fLeft, rect.fTop);
		points[1].set(rect.fRight, rect.fTop);
		points[2].set(rect.fRight, rect.fBottom);
		points[3].set(rect.fLeft, rect.fBottom);

		drawConvexPolygon(points, 4, paint);

	}


	void drawConvexPolygon(const GPoint points[], int count, const GPaint& paint) override {
		
		
		if (count <= 2) {
			return;
		}


		GPoint pts[count];
		//Points go through ctm
		for (int i = 0; i < count; i++) {
			CTM_stack.top().mapPoints(pts,points,count);
			
		}

		std::vector<GEdge> edges;

		// store edges
		storeEdges(pts, count, edges);

		// clip edges
		clipEdges(edges, fDevice.height(), fDevice.width());

		//sort edges
		sortEdges(edges);

		if (edges.size() == 0) {
			return;
		}

		int edge_count = 0;

		GEdge l = edges[edge_count];
		edge_count++;
		GEdge r = edges[edge_count];
		edge_count++;

		int top = GRoundToInt(l.p_top.fY);
		int bot = GRoundToInt(edges[edges.size() - 1].p_bottom.fY);
		GPixel storage[fDevice.width()];

		for (int y = top; y < bot; ++y) {
			if (GRoundToInt(l.p_bottom.fY) <= y) {
				l = edges[edge_count];
				edge_count++;
			}

			if (GRoundToInt(r.p_bottom.fY) <= y) {
				r = edges[edge_count];
				edge_count++;
			}

			int x1, x2;
			x1 = GRoundToInt(l.curr_x);
			x2 = GRoundToInt(r.curr_x);

			//if (paint.getShader()) {
			//	// use the shader instead of the paint�s color
			//	if (!paint.getShader()->setContext(CTM_stack.top())) {
			//		// nothing to draw if the shader fails
			//		return;
			//	}

			//	paint.getShader()->setContext(CTM_stack.top());
			//	GShader* shader = paint.getShader();

			//	GPixel storage[x2-x1];

			//	shader->shadeRow(x1, y, x2 - x1, storage);

			//	GPixel* row = fDevice.getAddr(0, y);

			//	for (int x = x1; x < x2; ++x) {
			//		row[x] = (*BlendProc[static_cast<int>(paint.getBlendMode())])(storage[x-x1],row[x]);
			//	}

			//} else {
			
			blit(y, x1, x2, paint, storage);
			
			/*}*/
			
			l.updateCurrentX();
			r.updateCurrentX();

			
		}

	}

	void drawPath(const GPath& path, const GPaint& paint){
		std::vector<GEdge> edges;
		storeEdges(path, edges, CTM_stack.top());

		//std::cout << "sorted edges: " << std::endl;
		//for (int i = 0; i < edges.size(); i++) {
		//	//std::cout << "i: " << i << std::endl;
		//	std::cout << edges[i].p_top.fX << "," << edges[i].p_top.fY << " " << edges[i].p_bottom.fX << "," << edges[i].p_bottom.fY << " curr_x : " << edges[i].curr_x <<" winding: "<<edges[i].winding<< std::endl;
		//}

		clipEdges(edges,fDevice.height(),fDevice.width());

		//std::cout << "clipped edges: " << std::endl;
		//for (int i = 0; i < edges.size(); i++) {
		//	//std::cout << "i: " << i << std::endl;
		//	std::cout << edges[i].p_top.fX << "," << edges[i].p_top.fY << " " << edges[i].p_bottom.fX << "," << edges[i].p_bottom.fY << " curr_x : " << edges[i].curr_x << " winding: " << edges[i].winding << std::endl;
		//}

		sortEdges(edges);

		//std::cout << "sorted edges: " << std::endl;
		//for (int i = 0; i < edges.size(); i++) {
		//	//std::cout << "i: " << i << std::endl;
		//	std::cout << edges[i].p_top.fX << "," << edges[i].p_top.fY << " " << edges[i].p_bottom.fX << "," << edges[i].p_bottom.fY << " curr_x : " << edges[i].curr_x << " winding: " << edges[i].winding << std::endl;
		//}


		//scan-converter
		const GMatrix topCTM = CTM_stack.top();
		GPath aPath = path;
		aPath.transform(topCTM);

		GRect bound = aPath.bounds();

		int top = GRoundToInt(bound.fTop);
		int bottom = GRoundToInt(bound.fBottom);
		//std::vector<GEdge>::iterator next, edge;
		int index, next=0;
		GPixel storage[fDevice.width()];
		GEdge* p = &edges.front();

		for (int y = top; y < bottom; ) {
			int size = edges.size();
			int w= 0; //winding accumulator
			int x0;
			int x1;
			index = 0;
	
			while (index<edges.size() && edges[index].y0 <= y && edges[index].y1 > y) {
				
				if (w == 0) {
					x0 = GRoundToInt(edges[index].curr_x);
				}

				w += edges[index].winding;

				if (w == 0) {
					
					x1 = GRoundToInt(edges[index].curr_x);
					blit(y, x0, x1, paint, storage);

				}

				next = index+1;
				
				
				if (edges[index].y1 == y+1 ) {
					edges.erase(edges.begin()+index);
					next--;
				}
				else {
					edges[index].updateCurrentX();
					resort_backward(index,edges);
				}

				index = next;
				
			}

			y++;

			while (edges[index].y0 == y && index<edges.size()) {
				next = index+1;

				resort_backward(index,edges);

				index = next;

			}

		}

	}

	void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, 
		const int indices[], const GPaint& paint) {
		int n = 0;
		GPoint points[3];
		GColor color[3];
		GPoint tex[3];
		GPaint p = paint;

		for (int i = 0; i < count; ++i) {
			points[0] = verts[indices[n]];
			points[1] = verts[indices[n + 1]];
			points[2] = verts[indices[n + 2]];

			if (colors != NULL) {
				color[0] = colors[indices[n]];
				color[1] = colors[indices[n + 1]];
				color[2] = colors[indices[n + 2]];
			}

			if (texs != NULL) {
				tex[0] = texs[indices[n]];
				tex[1] = texs[indices[n + 1]];
				tex[2] = texs[indices[n + 2]];
			}


			if (colors != NULL && texs != NULL) {
				TricolorShader s1(points, color);
				ProxyShader s2(paint.getShader(), points, tex);
				ComposeShader shader(&s1,&s2);
				p.setShader(&shader);
				drawConvexPolygon(points, 3, p);
			}else if (colors != NULL) {
				TricolorShader shader(points, color);
				p.setShader(&shader);
				drawConvexPolygon(points, 3, p);
			}else if (texs != NULL) {

				ProxyShader shader(paint.getShader(),points,tex);
				p.setShader(&shader);
				drawConvexPolygon(points, 3, p);
			}

			n += 3;
		}

			   		 
	}

	void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level,
		const GPaint& paint) {
		GASSERT(level >= 0);


		if (level == 0) {
			int indices[] = {0,1,3,2,1,3};
			drawMesh(verts, colors, texs, 2, indices, paint);
			return;
		}

		int count = (level + 2)*(level + 2);
		int num = (level+1)*(level+1) * 6;
		int indices[num];
		GPoint v[count];
		GPoint t[count];
		GColor color[count];
		

		float U, V;

		count = 0;
		for (int i = 0; i < (level + 2); i++) {
			U = ((float)i) / ((float)level + 1);
			for (int j = 0; j < (level + 2); j++) {
				V = ((float)j) / ((float)level + 1);
				float a = (1 - U)*(1 - V);
				float b = (1 - V)*U;
				float c = (1 - U)*V;
				float d = U * V;

				v[count] = a*verts[0] + b*verts[1] + d*verts[2] + c*verts[3];

				if (colors != NULL) {
					color[count] = Cadd(Cadd(Cadd(Cmul(a, colors[0]), Cmul(b, colors[1])), Cmul(d, colors[2])), Cmul(c, colors[3]));
				}

				if (texs != NULL) {
					t[count] = a * texs[0] + b * texs[1] + d * texs[2] + c * texs[3];
				}

				count++;
			}
		}
		GASSERT(count == (level+2)*(level+2));


		int val,k = 0;

		for (int i = 0; i < (level + 1); i++) {
			for (int j = 0; j < (level + 1); j++) {
				val = i * (level + 2) + j;
				indices[k] = val;
				indices[k + 1] = val + 1;
				indices[k + 2] = val + level + 2;
				indices[k + 3] = val + level + 2 + 1;
				indices[k + 4] = indices[k + 1];
				indices[k + 5] = indices[k + 2];
				k += 6;
			}
		}

		GASSERT(k==num);


		count = num / 3;

		if (colors == NULL) {
			drawMesh(v, nullptr, t, count, indices, paint);
		}
		else if (texs == NULL) {
			drawMesh(v, color, nullptr, count, indices, paint);
		}
		else {
			drawMesh(v, color, t, count, indices, paint);
		}
		
	}

	
private:
	const GBitmap fDevice;

	void blit(int y, int x1, int x2, const GPaint& paint, GPixel* storage) {

		int mode = static_cast<int>(paint.getBlendMode());
		
		//std::cout << "y: " << y << " x1: "<<x1<<" x2: "<<x2<< std::endl;

		if (paint.getShader()) {

			if (!paint.getShader()->setContext(CTM_stack.top())) {
				return;
			}

			paint.getShader()->setContext(CTM_stack.top());

			GShader* shader = paint.getShader();


			shader->shadeRow(x1, y, x2 - x1, storage);

			GPixel* row = fDevice.getAddr(0, y);

			for (int x = x1; x < x2; ++x) {
				row[x] = (*BlendProc[mode])(storage[x-x1], row[x]);

			}

		}
		else {
			GPixel source = color_to_pixel(paint.getColor());
			GPixel* row = fDevice.getAddr(0, y);
			for (int x = x1; x < x2; ++x) {
				row[x] = (*BlendProc[mode])(source, row[x]);
			}
		}	
	}

	std::stack<GMatrix> CTM_stack;

};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
	if (!device.pixels()) {
		return nullptr;
	}
	return std::unique_ptr<GCanvas>(new FanCanvas(device));
}