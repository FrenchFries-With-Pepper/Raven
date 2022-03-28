#include"primitive.h"

namespace Raven {
	bool Primitive::hit(const Ray& r_in, double tMin, double tMax)const {
		//perform ray-geometry intersection test
		if (!shape_ptr->hit(r_in, tMin, tMax)) {
			return false;
		}
		;
		return true;
	}

	std::optional<SurfaceInteraction> Primitive::intersect(const Ray& ray, double tMin, double tMax)const {
		//�жϹ����Ƿ��뼸�����ཻ
		std::optional<SurfaceInteraction> hitRecord = shape_ptr->intersect(ray, tMin, tMax);

		//����δ�뼸�����ཻ
		if (hitRecord == std::nullopt) {
			return std::nullopt;
		}

		//�ཻ���һ��й�Դ
		if (light_ptr.get()) {
			hitRecord->hitLight = true;
			hitRecord->light = this->getAreaLight();
		}
		else {
			hitRecord->hitLight = false;
		}
		//�������
		if (mate_ptr.get())
			mate_ptr->computeScarttingFunctions(*hitRecord);
		return *hitRecord;
	}

	Bound3f Primitive::worldBounds()const {
		return shape_ptr->worldBound();
	}

	bool TransformedPrimitive::hit(const Ray& r_in, double tMin, double tMax)const {
		if (!primToWorld || !worldToPrim || !prim)
			return false;
		//transform the incident ray to primitive space then perform ray intersection test
		Ray transformedRay = Inverse(*primToWorld)(r_in);
		return prim->hit(r_in, tMin, tMax);
	}

	std::optional<SurfaceInteraction> TransformedPrimitive::intersect(const Ray& r_in, double tMin, double tMax)const {
		if (!primToWorld || !worldToPrim || !prim)
			return std::nullopt;

		//�����߱任��Prim����ϵ�²���
		Ray transformedRay = (*worldToPrim)(r_in);
		std::optional<SurfaceInteraction> record = prim->intersect(transformedRay, tMin, tMax);

		//δ�ཻ
		if (record == std::nullopt) {
			return std::nullopt;
		}
		else {
			*record = (*primToWorld)(*record);
			return *record;
		}
	}

	Bound3f TransformedPrimitive::worldBounds()const {
		if (prim && primToWorld)
			return (*primToWorld)(prim->worldBounds());
		else
			return Bound3f();
	}
}