#include"areaLight.h"

namespace Raven {
	Vector3f DiffuseAreaLight::Li(const SurfaceInteraction& p, const Vector3f& wi)const {
		return Dot(p.n, wi) > 0.0 ? emittedRadiance : Vector3f(0.0);
	}

	Vector3f DiffuseAreaLight::sample_Li(const SurfaceInteraction& inter, const Point2f& uv,
		Vector3f* wi, double* pdf,SurfaceInteraction* lightSample)const {
		//�ڹ�Դ�������һ���㣬����Ӹõ������p�ķ�������
		SurfaceInteraction lightInter = shape_ptr->sample(inter, uv);
		*wi = Normalize(lightInter.p - inter.p);//����Ϊ�ӵ�p�����Դ
		//����Shape��pdf������������õ��pdf
		*pdf = shape_ptr->pdf(lightInter);
		*lightSample = lightInter;
		return	Li(lightInter, -*wi);
	}

	Vector3f DiffuseAreaLight::power()const {
		return emittedRadiance * area * M_PI;
	}

	double DiffuseAreaLight::pdf_Li(const SurfaceInteraction& inter, const Vector3f& wi)const {
		return shape_ptr->pdf(inter, wi);
	}

}