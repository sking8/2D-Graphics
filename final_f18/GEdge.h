#include "GPoint.h"
#include <vector>
#include <algorithm>
#include "include/GPoint.h"
#include "Utils.h"
#include <iostream>
#include "include/GMath.h"
#include "include/GPath.h"


struct GEdge {

	GPoint p_top;
	GPoint p_bottom;
	float slope;
	float curr_x;
	int y0; //top 
	int y1; //bottom
	int winding;

	bool init(GPoint p0, GPoint p1) {
		y0 = GRoundToInt(p0.fY); // assume p0 is the top one
		y1 = GRoundToInt(p1.fY); // assume p1 is the bottom one

		if (y0 ==y1) {
			return false;
		}

		slope = (p0.fX - p1.fX) / (p0.fY - p1.fY);

		if (y0 > y1) {
			//swap
			int y_tmp = y0;
			y0 = y1;
			y1 = y_tmp;

			p_top = p1;
			p_bottom = p0;
			winding = 1;
		}
		else {
			p_top = p0;
			p_bottom = p1;
			winding = -1;
		}

		this->init_currx();

		return true;
	}

	void init_currx() {
		float dy = y0 - p_top.fY+0.5;
		float dx = calculate_dx(dy, slope);
		curr_x = p_top.fX + dx;
	}

	void updateCurrentX() {
		curr_x += slope;
	}
};

bool sort_by_yx(GEdge& i, GEdge& j) {

	if (i.y0 < j.y0) {
		return true;
	}
	else if (i.y0 > j.y0) {
		return false;
	}
	else {

		//std::cout << "y equals to each other" << std::endl;

		if (GRoundToInt(i.curr_x) < GRoundToInt(j.curr_x)) {
			return true;
		}
		else if (GRoundToInt(i.curr_x) > GRoundToInt(j.curr_x)) {
			return false;
		}
		else if (i.slope < j.slope) {
			return true;
		}
		else {
			return false;
		}
	}


}


static void storeEdges(const GPoint points[], int count, std::vector<GEdge>& edges) {
	for (int i = 0; i < count - 1; i++) {
		GEdge edge;
		if (edge.init(points[i], points[i + 1])) {
			edges.push_back(edge);
		}
	}

	GEdge edge;
	if (edge.init(points[count - 1], points[0])) {
		edges.push_back(edge);
	}
}

static void storeEdges(const GPath& path, std::vector<GEdge>& edges, GMatrix& mat) {

	GPath::Edger edger(path);
	GPath::Verb v;
	GPoint points[4];
	v = edger.next(points);

	GEdge edge;

	/*std::cout <<"0 "<< points[0].fX << ", " << points[0].fY << std::endl;
	std::cout <<"1 "<< points[1].fX << ", " << points[1].fY << std::endl;*/

	while (v != GPath::Verb::kDone) {
		/*std::cout << "verb " << v << std::endl;
		std::cout << "points" << std::endl;
		for (int i = 0; i < 4; i++) {
			std::cout << points[i].fX<<","<<points[i].fY << std::endl;
		}*/
		
		mat.mapPoints(points, 4);

		if (v == GPath::Verb::kQuad) {
			
			GPoint tmp = (points[0]  + points[2]- 2 * points[1]) *0.25;
			float err = sqrtf(powf(tmp.fX,2) + powf(tmp.fY,2));
			int n = GCeilToInt(sqrtf(err * 4));
			GPoint start = points[0];
			GPoint end;

			for (int i = 1; i <= n; i++) {
				float t = (float)i / (float)n;
				/*std::cout << "t " << t << std::endl;*/

				end = calc_point_with_t(points[0],points[1],points[2],t);
				
				/*std::cout <<"start "<< start.fX << ", " << start.fY << std::endl;
				std::cout <<"end "<< end.fX << ", " << end.fY << std::endl;*/

				if (edge.init(start, end)) {
					edges.push_back(edge);		
				}

				start = end;
			}

			v = edger.next(points);
		}
		else if (v == GPath::Verb::kCubic) {
			GPoint tmp1 = points[0] + points[2] - 2 * points[1];
			GPoint tmp2 = points[1] + points[3] - 2 * points[2];
			float err1 = sqrtf(powf(tmp1.fX, 2) + powf(tmp1.fY, 2));
			float err2 = sqrtf(powf(tmp2.fX, 2) + powf(tmp2.fY, 2));

			float err = (err1<=err2)?err1:err2;
			int n = GCeilToInt(sqrtf(err * 3));
			GPoint start = points[0];
			GPoint end;

			for (int i = 1; i <= n; i++) {
				float t = (float)i / (float)n;

				end = calc_point_with_t(points[0], points[1], points[2],points[3],t);

				if (edge.init(start, end)) {
					edges.push_back(edge);
				}

				start = end;
			}

			v = edger.next(points);
		}
		else if (v == GPath::Verb::kLine) {
			
			if (edge.init(points[0], points[1])) {
				edges.push_back(edge);
			}
		
			v=edger.next(points);
		}
	}
	
}


static void sortEdges(std::vector<GEdge>& edges) {
	for (int i = 0; i < edges.size(); i++) {
		if (edges[i].y0 == edges[i].y1) {
			edges.erase(edges.begin() + i);
			i--;
		}
	}
	std::sort(edges.begin(), edges.end(), sort_by_yx);
}

static void resort_backward(int index, std::vector<GEdge>& edges) {
	
	// move edge backwards in the list until the list
	// is correctly sorted in curr_x
	// e.g. while (edge->curr_x < prev_edge->curr_x)
	//          move_edge_to_the_left in the list

	
	int pre = index - 1;

	/*if (edge->curr_x == 469.5) {
		std::cout << "edge pre" << pre->p_top.fX << "," << pre->p_top.fY << " " << pre->p_bottom.fX << "," << pre->p_bottom.fY << " curr_x : " << pre->curr_x << std::endl;
	}*/

	while (index>0 && index<edges.size() && edges[index].curr_x < edges[pre].curr_x) {
		//std::cout << "reach here 0" << std::endl;
		GEdge tmp= edges[pre];

		edges[pre] = edges[index];

		edges[index] = tmp;
		index=pre;
		pre=index-1;
	}

	//if (edges.end()->p_top.fX == (edges.end() - 1)->p_top.fX) {
	//	std::cout << "inside resort :" << std::endl;
	//	std::cout << "end" << edges.end()->p_top.fX << "," << edges.end()->p_top.fY << " " << edges.end()->p_bottom.fX << "," << edges.end()->p_bottom.fY << " curr_x : " << edges.end()->curr_x << std::endl;

	//	std::cout << "one before end" << (edges.end() - 1)->p_top.fX << "," << (edges.end() - 1)->p_top.fY << " " << (edges.end() - 1)->p_bottom.fX << "," << (edges.end() - 1)->p_bottom.fY << " curr_x : " << (edges.end() - 1)->curr_x << std::endl;
	//	std::cout << "current edge" << edge->p_top.fX << "," << edge->p_top.fY << " " << edge->p_bottom.fX << "," << edge->p_bottom.fY << std::endl;

	//	for (int i = 0; i < edges.size(); i++) {
	//		std::cout << "i: " << i << std::endl;
	//		std::cout << edges[i].p_top.fX << "," << edges[i].p_top.fY << " " << edges[i].p_bottom.fX << "," << edges[i].p_bottom.fY << std::endl;
	//	}

	//}

	/*std::cout << "after current edge: " << std::endl;
	std::cout << edge->p_top.fX << "," << edge->p_top.fY << " " << edge->p_bottom.fX << "," << edge->p_bottom.fY << std::endl;


	std::cout << "after resort: " << std::endl;
	for (int i = 0; i < edges.size(); i++) {
		std::cout << "i: " << i << std::endl;
		std::cout << edges[i].p_top.fX << "," << edges[i].p_top.fY << " " << edges[i].p_bottom.fX << "," << edges[i].p_bottom.fY << std::endl;
	}*/

}


static void clipEdges(std::vector<GEdge>& edges, const int height, const int width) {
	std::vector<GEdge> new_edges;

	for (int i = 0; i < edges.size(); i++) {

		//when both y is above canvas
		if ((edges[i].p_bottom.fY < 0) || (edges[i].p_top.fY > height)) {
			edges.erase(edges.begin() + i);
			i--;
			continue;
		}

		if (edges[i].p_top.fY <= 0) {
			float dx = calculate_dx(-edges[i].p_top.fY, edges[i].slope);
			edges[i].p_top.fX += dx;
			edges[i].p_top.fY = 0;
			edges[i].y0 = 0;
			edges[i].init_currx();

		}

		if (edges[i].p_bottom.fY >= height) {
			float dy = height - edges[i].p_bottom.fY;
			float dx = calculate_dx(dy, edges[i].slope);
			edges[i].p_bottom.fY = height;
			edges[i].y1 = height;
			edges[i].p_bottom.fX += dx;
		}

		if (edges[i].p_top.fX <= 0 && edges[i].p_bottom.fX <= 0) {
			edges[i].p_top.fX = 0;
			edges[i].p_bottom.fX = 0;
			edges[i].slope = 0;
			edges[i].curr_x = 0;
		}
		else if (edges[i].p_top.fX >= width && edges[i].p_bottom.fX >= width) {
			edges[i].p_top.fX = width;
			edges[i].p_bottom.fX = width;
			edges[i].slope = 0;
			edges[i].curr_x = width;
		}
		else {
			if (edges[i].slope < 0) {
				//p_top is the right one

				if (edges[i].p_bottom.fX < 0) {
					GPoint p0 = GPoint::Make(0.0, edges[i].p_bottom.fY);

					float dy = calculate_dy(-edges[i].p_bottom.fX, edges[i].slope);

					edges[i].p_bottom.fY += dy;
					edges[i].y1 = GRoundToInt(edges[i].p_bottom.fY);
					edges[i].p_bottom.fX = 0;

					GPoint p1 = GPoint::Make(0.0, edges[i].p_bottom.fY);

					GEdge left_over;

					if (left_over.init(p0, p1)) {
						left_over.winding = edges[i].winding;
						new_edges.push_back(left_over);
					}
				}

				if (edges[i].p_top.fX > width) {

					GPoint p0 = GPoint::Make(width, edges[i].p_top.fY);

					float dy = calculate_dy(width - edges[i].p_top.fX, edges[i].slope);

					edges[i].p_top.fY += dy;
					edges[i].y0 = GRoundToInt(edges[i].p_top.fY);
					edges[i].p_top.fX = width;
					edges[i].init_currx();

					GPoint p1 = GPoint::Make(width, edges[i].p_top.fY);

					GEdge left_over;

					if (left_over.init(p0, p1)) {
						left_over.winding = edges[i].winding;
						new_edges.push_back(left_over);
					}
				}
			}
			else if (edges[i].slope > 0) {
				//p_top is the left one
				if (edges[i].p_top.fX < 0) {
					GPoint p0 = GPoint::Make(0.0, edges[i].p_top.fY);
					float dy = calculate_dy(-edges[i].p_top.fX, edges[i].slope);
					edges[i].p_top.fY += dy;
					edges[i].y0 = GRoundToInt(edges[i].p_top.fY);
					edges[i].p_top.fX = 0;
					edges[i].init_currx();

					GPoint p1 = GPoint::Make(0.0, edges[i].p_top.fY);

					GEdge left_over;

					if (left_over.init(p1,p0)) {
						left_over.winding = edges[i].winding;
						new_edges.push_back(left_over);
					}

				}

				if (edges[i].p_bottom.fX > width) {
					GPoint p0 = GPoint::Make(width, edges[i].p_bottom.fY);

					float dy = calculate_dy(width - edges[i].p_bottom.fX, edges[i].slope);

					edges[i].p_bottom.fY += dy;
					edges[i].y1 = GRoundToInt(edges[i].p_bottom.fY);
					edges[i].p_bottom.fX = width;

					GPoint p1 = GPoint::Make(width, edges[i].p_bottom.fY);

					GEdge left_over;

					if (left_over.init(p1,p0)) {
						left_over.winding = edges[i].winding;
						new_edges.push_back(left_over);
					}

				}
			}

		}
	}

	edges.insert(edges.end(), new_edges.begin(), new_edges.end());

}