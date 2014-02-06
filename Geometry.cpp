//
// Geometry.cpp - general purpose geometry and math
//

// Copyright (c) 2013 Arthur Danskin
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "StdAfx.h"
#include "Geometry.h"

// intersect two circles, returning number of intersections with points in RA and RB
int intersectCircleCircle(float2 *ra, float2 *rb, const float2 &p, float pr, const float2 &c, float cr)
{
    if (!intersectCircleCircle(p, pr, c, cr))
        return 0;
    float2 c2  = c - p;         // position of c relative to p
    float dist = length(c2);
    if (dist < epsilon && fabsf(pr - cr) < epsilon) {
        // coincident
        return 3;
    } else if (dist < max(pr, cr) - min(pr, cr)) {
        // fully contained
        return 0;
    } else if (fabsf(dist - (pr + cr)) < epsilon) {
        // just touching
        *ra = p + (pr/dist) * c2;
        return 1;
    }

    // with p at origin and c at (dist, 0)
    float x = (dist*dist - cr*cr + pr*pr) / (2.f * dist);
    float y = sqrt(pr*pr - x*x); // plus and minus
    
    float2 ydir   = rotate90(c2 / dist);
    float2 center = p + (x / dist) * c2;

    *ra = center + ydir * y;
    *rb = center - ydir * y;
    return 2;
}

bool intersectSegmentSegment(float2 a1, float2 a2, float2 b1, float2 b2)
{
	const float div = (b2.y - b1.y) * (a2.x - a1.x) - (b2.x - b1.x) * (a2.y - a1.y);
	if (fabsf(div) < 0.00001)
		return false; // parallel
    
	const float ua = ((b2.x - b1.x) * (a1.y - b1.y) - (b2.y - b1.y) * (a1.x - b1.x)) / div;
	const float ub = ((a2.x - a1.x) * (a1.y - b1.y) - (a2.y - a1.y) * (a1.x - b1.x)) / div;
    
	return (0 <= ua && ua <= 1) && (0 <= ub && ub <= 1);
}

bool intersectSegmentSegment(float2 *o, float2 a1, float2 a2, float2 b1, float2 b2)
{
	const float div = (b2.y - b1.y) * (a2.x - a1.x) - (b2.x - b1.x) * (a2.y - a1.y);
	if (std::fabs(div) < 0.00001)
		return false; // parallel
    
	const float ua = ((b2.x - b1.x) * (a1.y - b1.y) - (b2.y - b1.y) * (a1.x - b1.x)) / div;
	const float ub = ((a2.x - a1.x) * (a1.y - b1.y) - (a2.y - a1.y) * (a1.x - b1.x)) / div;
    
	bool intersecting = (0 <= ua && ua <= 1) && (0 <= ub && ub <= 1);
    if (intersecting && o)
        *o = lerp(a1, a2, ua);
    return intersecting;
}

/*               Return a positive value if the point pd lies inside the     */
/*               circle passing through pa, pb, and pc; a negative value if  */
/*               it lies outside; and zero if the four points are cocircular.*/
/*               The points pa, pb, and pc must be in counterclockwise       */
/*               order, or the sign of the result will be reversed.          */
float incircle(float2 pa, float2 pb, float2 pc, float2 pd)
{
    double adx, ady, bdx, bdy, cdx, cdy;
    double abdet, bcdet, cadet;
    double alift, blift, clift;
    
    adx = pa.x - pd.x;
    ady = pa.y - pd.y;
    bdx = pb.x - pd.x;
    bdy = pb.y - pd.y;
    cdx = pc.x - pd.x;
    cdy = pc.y - pd.y;
    
    abdet = adx * bdy - bdx * ady;
    bcdet = bdx * cdy - cdx * bdy;
    cadet = cdx * ady - adx * cdy;
    alift = adx * adx + ady * ady;
    blift = bdx * bdx + bdy * bdy;
    clift = cdx * cdx + cdy * cdy;
    
    return alift * bcdet + blift * cadet + clift * abdet;
}

// Graham's scan
int convexHull(vector<float2> &points)
{
    if (points.size() < 3)
        return 0;
    if (points.size() == 3)
        return 3;
        
    float minY  = std::numeric_limits<float>::max();
    int   minYI = -1;
    
    for (int i=0; i<points.size(); i++)
    {
        if (points[i].y < minY) {
            minY = points[i].y;
            minYI = i;
        }
    }
    swap(points[0], points[minYI]);
    const float2 points0 = points[0];
    std::sort(points.begin() + 1, points.end(),
              [points0](float2 a, float2 b)
              {
                  return vectorToAngle(a - points0) < vectorToAngle(b - points0);
              });
    vector<float2>::iterator nend = std::unique(points.begin(), points.end(),
                                                [](float2 a, float2 b) { return isZero(a - b); });
    
    const size_t N = std::distance(points.begin(), nend);
    long M = 1;
    for (int i=1; i<N; i++)
    {
        while (orient(points[M-1], points[M], points[i]) < 0)
        {
            if (M > 1)
                M--;
            else if (i == N-1)
                return 0;
            else
                i++;
        }
        M++;
        swap(points[M], points[i]);
    }
    if (M < 2)
        return 0;
    return M+1;
}


// modified from http://stackoverflow.com/questions/1073336/circle-line-collision-detection
// ray is at point E in direction d
// circle is at point C with radius r
bool intersectRayCircle(float2 *o, float2 E, float2 d, float2 C, float r)
{
    float2 f = E - C;
    d = normalize(d) * (r + length(f));

    float a = dot(d, d);
    float b = 2 * dot(f, d);
    float c = dot(f, f) - r * r;

    float discriminant = b*b-4*a*c;
    if (discriminant < 0)
        return false;

    // ray didn't totally miss sphere, so there is a solution to the equation.

    discriminant = sqrt( discriminant );

    // either solution may be on or off the ray so need to test both t1 is always the smaller value,
    // because BOTH discriminant and a are nonnegative.
    float t1 = (-b - discriminant)/(2*a);
    float t2 = (-b + discriminant)/(2*a);

    if( t1 >= 0 && t1 <= 1 )
    {
        // t1 is an intersection, and if it hits, it's closer than t2 would be
        if (o)
            *o = E + t1 * d;
        return true ;
    }

    // here t1 didn't intersect so we are either started inside the sphere or completely past it
    if( t2 >= 0 && t2 <= 1 )
    {
        if (o)
            *o = E + t2 * d;
        return true ;
    }

    return false;
}


float intersectBBSegmentV(float bbl, float bbb, float bbr, float bbt, float2 a, float2 b)
{
	float idx = 1.0f/(b.x - a.x);
	float tx1 = (bbl == a.x ? -INFINITY : (bbl - a.x)*idx);
	float tx2 = (bbr == a.x ?  INFINITY : (bbr - a.x)*idx);
	float txmin = min(tx1, tx2);
	float txmax = max(tx1, tx2);
	
	float idy = 1.0f/(b.y - a.y);
	float ty1 = (bbb == a.y ? -INFINITY : (bbb - a.y)*idy);
	float ty2 = (bbt == a.y ?  INFINITY : (bbt - a.y)*idy);
	float tymin = min(ty1, ty2);
	float tymax = max(ty1, ty2);
	
	if(tymin <= txmax && txmin <= tymax){
		float minv = max(txmin, tymin);
		float maxv = min(txmax, tymax);
		
		if(0.0 <= maxv && minv <= 1.0) 
            return (minv > 0.f) ? minv : min(maxv, 1.f);
	}
	
	return INFINITY;
}

bool intersectPointTriangle(float2 P, float2 A, float2 B, float2 C)
{
    // Compute vectors        
    float2 v0 = C - A;
    float2 v1 = B - A;
    float2 v2 = P - A;

    // Compute dot products
    float dot00 = dot(v0, v0);
    float dot01 = dot(v0, v1);
    float dot02 = dot(v0, v2);
    float dot11 = dot(v1, v1);
    float dot12 = dot(v1, v2);

    // Compute barycentric coordinates
    float invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
    return (u >= 0) && (v >= 0) && (u + v < 1);
}

bool intersectSectorCircle(float2 ap, float ar, float ad, float aa, float2 cp, float cr)
{
    if (!intersectCircleCircle(ap, ar, cp, cr))
        return false;
    if (intersectPointCircle(ap, cp, cr)         || // sector center inside circle
        intersectSectorPoint(ap, ar, ad, aa, cp) || // circle center inside sector
        (aa >= M_PIf))                              // sector is complete circle
        return true;
    else if (aa < epsilon)           // sector is a line
        return intersectSegmentCircle(ap, ap + ar * angleToVector(ad), cp, cr);

    const float2 a0 = ap + ar * angleToVector(ad + aa);
    if (intersectSegmentCircle(ap, a0, cp, cr)) // circle intersects left edge
        return true;
    const float2 a1 = ap + ar * angleToVector(ad - aa);
    if (intersectSegmentCircle(ap, a1, cp, cr)) // circle intersects right edge
        return true;

    float2 r[2];
    const int points = intersectCircleCircle(&r[0], &r[1], ap, ar, cp, cr);
    for (uint i=0; i<points; i++) {
        if (intersectSectorPoint1(ap, ad, aa, r[i])) // circle intersects arc
            return true;
    }
    return false;
}

int quadraticFormula(double* r0, double* r1, double a, double b, double c)
{
    if (isZero(a)) {
        *r0 = isZero(b) ? (-c) : (-c / b);
        return 1;
    }
    
    const double discr = b*b - 4*a*c;
    if (discr < 0) {
        return 0;
    } else if (discr == 0) {
        *r0 = -b / (2*a);
        return 1;
    }
    
    const double sqrt_discr = sqrt(discr);
    *r0 = (-b - sqrt_discr) / (2*a);
    *r1 = (-b + sqrt_discr) / (2*a);
    return 2;
}


float2 clamp_rect(float2 v, float2 rad)
{
    float x = intersectBBSegmentV(-rad.x, -rad.y, rad.x, rad.y, float2(0.f), v);
    return (0 <= x && x <= 1.f) ? x * v : v;
}


float2 circle_in_rect(float2 pos, float rad, float2 rcenter, float2 rrad)
{
    if (rad > min_dim(rrad))
        return rcenter;

    float2 npos = pos;
    for (uint i=0; i<2; i++) {
        if (npos[i] - rad < rcenter[i] - rrad[i])
            npos[i] = rcenter[i] - rrad[i] + rad;
        else if (npos[i] + rad > rcenter[i] + rrad[i])
            npos[i] = rcenter[i] + rrad[i] - rad;
    }

    ASSERT(containedCircleInRectangle(npos, rad-epsilon, rcenter, rrad));
    return pos;
}
