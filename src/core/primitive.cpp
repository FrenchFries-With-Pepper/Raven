#include"primitive.h"

namespace Raven {
	bool Primitive::hit(const Ray& r_in, double tMin, double tMax)const {
		//perform ray-geometry intersection test
		return shape_ptr->hit(r_in, tMin, tMax);
	}

	bool Primitive::intersect(const Ray& ray, SurfaceInteraction& its, double tMin, double tMax)const {
		//first perform ray-geometry intersection test,computing surfaceIntersection if hit, then compute bsdf
		if (!shape_ptr->intersect(ray, its, tMin, tMax)) {
			return false;
		}
		if (mate_ptr)
			mate_ptr->computeScarttingFunctions(its);
		return true;
	}

	Bound3f Primitive::worldBounds()const {
		return shape_ptr->worldBound();
	}

	bool TransformedPrimitive::hit(const Ray& r_in, double tMin, double tMax)const {
		if (!primToWorld || !worldToPrim || !prim)
			return false;
		//transform the incident ray to primitive space then perform ray intersection test
		Ray transformedRay = Transform::Inverse(*primToWorld)(r_in);
		return prim->hit(r_in, tMin, tMax);
	}

	bool TransformedPrimitive::intersect(const Ray& r_in, SurfaceInteraction& its, double tMin, double tMax)const {
		if (!primToWorld || !worldToPrim || !prim)
			return false;
		Ray transformedRay = (*worldToPrim)(r_in);
		if (prim->intersect(transformedRay, its, tMin, tMax)) {//hit
			its = (*primToWorld)(its);
			return true;
		}
		return false;
	}

	Bound3f TransformedPrimitive::worldBounds()const {
		if (prim && primToWorld)
			return (*primToWorld)(prim->worldBounds());
		else
			return Bound3f();
	}

	//iterate over all primitives, find the closest intersection
	bool PrimitiveList::intersect(const Ray& r_in, SurfaceInteraction& its, double tMin, double tMax)const {
		bool flag = false;
		SurfaceInteraction temp;
		double closest = tMax;
		for (int i = 0; i < pris.size(); i++) {
			if (pris[i]->intersect(r_in, temp, tMin, closest)) {
				closest = temp.t;
				flag = true;
			}
		}
		if (flag)
			its = temp;
		return flag;
	}

	bool PrimitiveList::hit(const Ray& r_in, double tMin, double tMax)const {
		for (int i = 0; i < pris.size(); i++)
			if (pris[i]->hit(r_in, tMin, tMax))
				return true;
	}

	Bound3f PrimitiveList::worldBounds()const {
		bool flag = false;
		Bound3f box;
		for (int i = 0; i < pris.size(); i++) {
			Bound3f b = pris[i]->worldBounds();
			box = Union(box, pris[i]->worldBounds());
		}
		return box;
	}
}