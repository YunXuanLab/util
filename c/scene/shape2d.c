//
// Created by hujianzhe on 18-4-13.
//

#include "shape2d.h"
#include <errno.h>
#include <float.h>
#if defined(_WIN32) || defined(_WIN64)
	#ifndef	_USE_MATH_DEFINES
		#define	_USE_MATH_DEFINES
	#endif
#endif
#include <math.h>
#include <stdlib.h>
#if defined(_WIN32) || defined(_WIN64)
	#include <malloc.h>
	#ifndef alloca
		#define alloca		_alloca
	#endif
#else
	#if	__linux__
		#include <alloca.h>
	#endif
#endif

#ifdef	__cplusplus
extern "C" {
#endif

int shape2d_circle_is_overlap(const struct shape2d_circle_t* c1, const struct shape2d_circle_t* c2) {
	double rd = c1->radius + c2->radius;
	double xd = c1->pivot.x - c2->pivot.x;
	double yd = c1->pivot.y - c2->pivot.y;
	return xd * xd + yd * yd < rd * rd;
}

static double __projection(const struct shape2d_obb_t* p, const struct vector2_t pv[], const struct vector2_t* v) {
	return p->w * fabs(vector2_dot(v, &pv[0])) + p->h * fabs(vector2_dot(v, &pv[1]));
}
int shape2d_obb_is_overlap(const struct shape2d_obb_t* o1, const struct shape2d_obb_t* o2) {
	int i;
	struct vector2_t v = { o1->x - o2->x, o1->y - o2->y };
	struct vector2_t pv[4];
	// o1
	pv[0].x = cos(o1->radian);
	pv[0].y = sin(o1->radian);
	pv[1].x = -pv[0].y;
	pv[1].y = pv[0].x;
	// o2
	pv[2].x = cos(o2->radian);
	pv[2].y = sin(o2->radian);
	pv[3].x = -pv[2].y;
	pv[3].y = pv[2].x;
	for (i = 0; i < sizeof(pv) / sizeof(pv[0]); ++i) {
		double r1 = __projection(o1, &pv[0], &pv[i]);
		double r2 = __projection(o2, &pv[2], &pv[i]);
		double d = fabs(vector2_dot(&v, &pv[i]));
		if (r1 + r2 <= d)
			return 0;
	}
	return 1;
}

void shape2d_convex_rotate(struct shape2d_convex_t* c, double radian) {
	unsigned int i;
	for (i = 0; i < c->vertice_num; ++i) {
		double x = (c->vertices[i].x - c->pivot.x) * cos(radian) - (c->vertices[i].y - c->pivot.y) * sin(radian) + c->pivot.x;
		double y = (c->vertices[i].x - c->pivot.x) * sin(radian) + (c->vertices[i].y - c->pivot.y) * cos(radian) + c->pivot.y;
		c->vertices[i].x = x;
		c->vertices[i].y = y;
	}
	c->radian = radian;
}
int shape2d_convex_is_contain_point(const struct shape2d_convex_t* c, struct vector2_t* point) {
	unsigned int i;
	double radian_total = 0.0;
	for (i = 1; i < c->vertice_num; ++i) {
		struct vector2_t v1 = { c->vertices[i - 1].x - point->x, c->vertices[i - 1].y - point->y };
		struct vector2_t v2 = { c->vertices[i].x - point->x, c->vertices[i].y - point->y };
		double radian = vector2_radian(&v1, &v2);
		if (EDOM == errno || fabs(radian - M_PI) < DBL_EPSILON) {
			return 0;
		}
		radian_total += radian;
	}
	return fabs(radian_total - 2*M_PI) < DBL_EPSILON;
}
int shape2d_convex_is_overlap(const struct shape2d_convex_t* c1, const struct shape2d_convex_t* c2) {
	int use_malloc = 0;
	unsigned int i, j = 0;
	unsigned int axes_num = c1->vertice_num + c2->vertice_num;
	struct vector2_t* axes;
	if (axes_num > 1400) {
		use_malloc = 1;
	}
	axes = use_malloc ? (struct vector2_t*)malloc(sizeof(struct vector2_t) * axes_num) : 
						(struct vector2_t*)alloca(sizeof(struct vector2_t) * axes_num);
	if (!axes) {
		return 0;
	}
	
	for (i = 0; i < c1->vertice_num; ++i, ++j) {
		struct vector2_t* pv = c1->vertices + (i + 1) % c1->vertice_num;
		struct vector2_t v = { c1->vertices[i].x - pv->x, c1->vertices[i].y - pv->y };
		vector2_normalized(&v, &v);
		axes[j].x = -v.y;
		axes[j].y = v.x;
	}
	for (i = 0; i < c2->vertice_num; ++i, ++j) {
		struct vector2_t* pv = c2->vertices + (i + 1) % c2->vertice_num;
		struct vector2_t v = { c2->vertices[i].x - pv->x, c2->vertices[i].y - pv->y };
		vector2_normalized(&v, &v);
		axes[j].x = -v.y;
		axes[j].y = v.x;
	}
	for (i = 0; i < axes_num; ++i) {
		double r;
		double min1 = DBL_MAX, max1 = -DBL_MAX, ret1;
		double min2 = DBL_MAX, max2 = -DBL_MAX, ret2;
		for (j = 0; j < c1->vertice_num; ++j) {
			ret1 = vector2_dot(c1->vertices + j, axes + i);
			min1 = min1 > ret1 ? ret1 : min1;
			max1 = max1 < ret1 ? ret1 : max1;
		}
		for (j = 0; j < c2->vertice_num; ++j) {
			ret2 = vector2_dot(c2->vertices + j, axes + i);
			min2 = min2 > ret2 ? ret2 : min2;
			max2 = max2 < ret2 ? ret2 : max2;
		}
		r = (max1 > max2 ? max1 : max2) - (min1 < min2 ? min1 : min2);
		if ((max1 - min1) + (max2 - min2) <= r) {
			if (use_malloc) {
				free(axes);
			}
			return 0;
		}
	}
	if (use_malloc) {
		free(axes);
	}
	return 1;
}

#ifdef	__cplusplus
}
#endif