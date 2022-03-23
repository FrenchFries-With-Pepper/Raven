#ifndef _RAVEN_LIGHT_AREA_LIGHT_H_
#define _RAVEN_LIGHT_AREA_LIGHT_H_


#include"../core/base.h"
#include"../core/light.h"

namespace Raven {

	/// <summary>
	/// �����Դ�����
	/// </summary>
	class AreaLight :public Light {
	public:
		AreaLight(const Transform* LTW, const Transform* WTL, int flag, int nSamples, const Shape* shape) :
			Light(LTW, WTL, flag, nSamples), shape_ptr(shape), area(shape->area()) {}

		////����ռ��е�һ����p���ڹ�Դ���������������������Radiance
		virtual Vector3f sampleLi(const SurfaceInteraction& inter, const Point2f& uv, LightSample* lightSample)const = 0;

		//������Դ�ϵ�һ��������䷽�򣬼�������Radiance
		virtual Vector3f Li(const SurfaceInteraction& inter, const Vector3f& wi)const = 0;

		//���ع�Դ��ռ��з�����ܵ�����
		virtual Vector3f power()const = 0;

		//������Դ�ϵ�һ��������䷽�򣬼��������pdf
		virtual double pdf_Li(const SurfaceInteraction& inter, const Vector3f& wi)const = 0;

		//virtual Vector3f sample_Li(const SurfaceInteraction& inter, const Point2f& uv,
		//	Vector3f* wi, double* pdf, SurfaceInteraction* lightSample)const = 0;
	protected:

		const Shape* shape_ptr;
		const double area;
	};


	/// <summary>
	/// DiffuseAreaLightʵ���࣬�ù�Դ�ϵ�����һ����ռ��е����ⷽ�����ͬ�ȴ�С��Radiance
	/// </summary>
	class DiffuseAreaLight :public AreaLight {
	public:
		DiffuseAreaLight(const Transform* LTW, const Transform* WTL,
			int nSamples, const Shape* shape, Vector3f I) :
			AreaLight(LTW, WTL, LightFlag::AreaLight, nSamples, shape), emittedRadiance(I) {}

		virtual Vector3f Li(const SurfaceInteraction& p, const Vector3f& wi)const;

		virtual Vector3f sampleLi(const SurfaceInteraction& inter, const Point2f& uv,
			LightSample* lightSample)const;

		virtual Vector3f power()const;

		virtual double pdf_Li(const SurfaceInteraction& inter, const Vector3f& wi)const;


	private:
		const Vector3f emittedRadiance;
	};
}


#endif 