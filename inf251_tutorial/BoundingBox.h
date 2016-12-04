#pragma once
#include <glm\glm.hpp>
using namespace glm;

class BoundingBox {
private:
	// Points in the x-z plane
	// i.e each has (x,z) coordinates!!!
	vec2 a, b, c, d;

	float yMin, yMax;
	
	bool isInitialized = false;

	bool enableYBounding = true;
	bool enableXZPlaneBounding = false;

	inline bool isBetween(float v, float p1, float p2) {
		return v > p1 != v > p2;
	}

	inline bool isInXZPlane(vec2 m) {
		vec2 am = m - a,
			 ab = b - a,
		     ad = d - a;

		float am_dot_ab = dot(am, ab),
			  ab_dot_ab = dot(ab, ab),
			  am_dot_ad = dot(am, ad),
			  ad_dot_ad = dot(ad, ad);

		return (0 < am_dot_ab < ab_dot_ab) &&
			   (0 < am_dot_ad < ad_dot_ad);
	}
public:
	bool isWithinBoundingBox(vec3 point) {
		return !isInitialized || 
			   (
				   (!enableXZPlaneBounding || isInXZPlane(vec2(point.x, point.z))) &&
				   (!enableYBounding || isBetween(point.y, yMin, yMax))
			   );
	}

	void setBoundingXZPlane(vec2 av, vec2 bv, vec2 cv, vec2 dv) {
		a = av;
		b = bv;
		c = cv;
		d = dv;

		enableXZPlaneBounding = true;
	}

	BoundingBox() {}

	void setYBounding(float minY, float maxY) {
		isInitialized = true;
		yMin = minY;
		yMax = maxY;
	}
};