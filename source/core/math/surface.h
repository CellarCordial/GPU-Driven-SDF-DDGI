#ifndef MATH_SURFACE_H
#define MATH_SURFACE_H

#include "vector.h"


namespace fantasy 
{
    struct QuadricSurface
	{
        double a2 = 0.0, b2 = 0.0, c2 = 0.0, d2 = 0.0;
        double ab = 0.0, ac = 0.0, ad = 0.0;
        double bc = 0.0, bd = 0.0; 
        double cd = 0.0;

        QuadricSurface() = default;
        QuadricSurface(const double3& p0, const double3& p1, const double3& p2);

        bool get_vertex(float3& position);
        float distance_to_surface(const float3& p);
        float3 calculate_normal(float3 p);
        float3 calculate_tangent(float3 p);
	};

    QuadricSurface merge(const QuadricSurface& surface0, const QuadricSurface& surface1);
}















#endif