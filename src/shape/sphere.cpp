#include"sphere.h"
#include"../core/distribution.h"
namespace Raven {
	bool Sphere::hit(const Ray& r_in, double tMin = 0.001F, double tMax = FLT_MAX)const {
		//Ray localRay = (*worldToLocal)(r_in);

		//double a = Dot(localRay.dir, localRay.dir);
		//double b = 2 * Dot(Vector3f(localRay.origin), localRay.dir);
		//double c = Dot(Vector3f(localRay.origin), Vector3f(localRay.origin)) - radius * radius;
		//double t0, t1;
		//if (!Quadratic(a, b, c, t0, t1))
		//	return false;
		////determinate>=0, at least has one hit point,test if tHit is out of time range
		//if (t0 > tMax || t1 < tMin)//since t0<t1 by default if t0>tmax || t1< tmin,both values are out of range
		//	return false;
		////defaultly set tHit = t0, the smaller t value. if t0 is out of time limit, set tHit = t1
		//if (t0 < tMin && t1 > tMax)
		//	return false;
		//return true;


				//transform the ray from world space to local space
		Ray localRay = (*worldToLocal)(r_in);

		//perform intersection test
		double a = Dot(localRay.dir, localRay.dir);
		double b = 2.0 * Dot(Vector3f(localRay.origin), localRay.dir);
		double c = Dot(Vector3f(localRay.origin), Vector3f(localRay.origin)) - radius * radius;
		double t0, t1, tHit;
		if (!Quadratic(a, b, c, t0, t1))
			return false;

		//determinate>=0, at least has one hit point,test if tHit is out of time range
		if (t0 > tMax || t1 < tMin)//since t0<t1 by default if t0>tmax || t1< tmin,both values are out of range
			return false;
		//defaultly set tHit = t0, the smaller t value. if t0 is out of time limit, set tHit = t1
		tHit = t0;
		if (t0 < tMin) {
			tHit = t1;
			if (t1 > tMax)
				return false;
		}

		//get pHit and corresponding phi value
		Point3f pHit = localRay.position(tHit);//get hit point 
		double phi = atan(pHit[1] / pHit[0]);
		if (phi < 0)phi += 2 * M_PI;//make sure phi is between 0 and 2PI

		//test the hit point against the clipping parameter(pMax,zMin,zMax
		if (pHit[2] > zMax || pHit[2]<zMin || phi>phiMax) {
			//false
			if (tHit == t0) {
				//pHit0 is clipped, test pHit1
				tHit = t1;
				//get pHit1 and corresponding phi
				pHit = localRay.position(tHit);
				phi = atan(Clamp(pHit[1] / pHit[0], -1.f, 1.f));
				if (phi < 0)phi += 2 * M_PI;

				//test pHit1
				if (pHit[2] > zMax || pHit[2]<zMin || phi>phiMax || t1 > tMax)
					//false,both hit point failed
					return false;
			}
			else
				//t0 is out of time range, pHit1 is clipped
				return false;
		}
	}

	Bound3f Sphere::worldBound()const {
		Bound3f localBox;
		localBox.pMin = Point3f(-radius, -radius, zMin);
		localBox.pMax = Point3f(radius, radius, zMax);
		return (*localToWorld)(localBox);
	}

	Bound3f Sphere::localBound()const {
		Bound3f localBox;
		localBox.pMin = Point3f(-radius, -radius, zMin);
		localBox.pMax = Point3f(radius, radius, zMax);
		return localBox;
	}

	std::optional<SurfaceInteraction> Sphere::intersect(const Ray& r_in, double tmin = 0.001F, double tmax = FLT_MAX)const {
		//transform the ray from world space to local space
		Ray localRay = (*worldToLocal)(r_in);

		//perform intersection test
		double a = Dot(localRay.dir, localRay.dir);
		double b = 2.0 * Dot(Vector3f(localRay.origin), localRay.dir);
		double c = Dot(Vector3f(localRay.origin), Vector3f(localRay.origin)) - radius * radius;
		double t0, t1, tHit;
		if (!Quadratic(a, b, c, t0, t1))
			return std::nullopt;
		//determinate>=0, at least has one hit point,test if tHit is out of time range
		if (t0 > tmax || t1 < tmin)//since t0<t1 by default if t0>tmax || t1< tmin,both values are out of range
			return std::nullopt;
		//defaultly set tHit = t0, the smaller t value. if t0 is out of time limit, set tHit = t1
		tHit = t0;
		if (t0 < tmin) {
			tHit = t1;
			if (t1 > tmax)
				return std::nullopt;
		}
		//���߱��������򴫲�
		if (tHit < 0.0)
			return std::nullopt;
		//get pHit and corresponding phi value
		Point3f pHit = localRay.position(tHit);//get hit point 
		double phi = atan(pHit[1] / pHit[0]);
		if (phi < 0)phi += 2 * M_PI;//make sure phi is between 0 and 2PI

		//test the hit point against the clipping parameter(pMax,zMin,zMax
		if (pHit[2] > zMax || pHit[2]<zMin || phi>phiMax) {
			//false
			if (tHit == t0) {
				//pHit0 is clipped, test pHit1
				tHit = t1;
				//get pHit1 and corresponding phi
				pHit = localRay.position(tHit);
				phi = atan(Clamp(pHit[1] / pHit[0], -1.f, 1.f));
				if (phi < 0)phi += 2 * M_PI;

				//test pHit1
				if (pHit[2] > zMax || pHit[2]<zMin || phi>phiMax || t1 > tmax)
					//false,both hit point failed
					return std::nullopt;
			}
			else
				//t0 is out of time range, pHit1 is clipped
				return std::nullopt;
		}
		//else true

		//compute geometric parameters of surface intersection
		double u = phi / phiMax;
		double theta = acos(Clamp(pHit[2] / radius, -1.0, 1.0));
		double v = theta - thetaMin / abs(thetaMax - thetaMin);
		Normal3f n = Normal3f(pHit - Point3f(0.0, 0.0, 0.0)).normalized();



		Vector3f dpdu(-pHit[1] * phiMax, -pHit[0] * phiMax, 0);
		Vector3f dpdv = (thetaMax - thetaMin) * Vector3f(pHit[2] * cos(phi), 
			pHit[2] * sin(theta), -radius * sin(theta));

		//coefficients to compute dndu dndv
		double E = abs(Dot(dpdu, dpdu));
		double F = abs(Dot(dpdu, dpdv));
		double G = abs(Dot(dpdv, dpdv));
		Vector3f dpduu = -phiMax * phiMax * Vector3f(pHit[0], pHit[1], 0);
		Vector3f dpduv = (thetaMax - thetaMin) * pHit[2] * phiMax * Vector3f(-sin(phi), cos(phi), 0);
		Vector3f dpdvv = -(thetaMax - thetaMin) * (thetaMax - thetaMin) * Vector3f(pHit[0], pHit[1], pHit[2]);
		double e = Dot(n, dpduu);
		double f = Dot(-n, dpduv);
		double g = Dot(n, dpdvv);

		Vector3f dndu = ((f * F - e * G) / (E * G - F * F)) * dpdu +
			((e * F - f * E) / (E * G - F * F)) * dpdv;
		Vector3f dndv = ((g * F - f * G) / (E * G - F * F)) * dpdu +
			((f * F - g * E) / (E * G - F * F)) * dpdv;

		//generate surfaceIntersection and transform it to world space 
		SurfaceInteraction its;
		its.p = pHit;
		its.t = tHit;
		its.uv = Point2f(u, v);
		its.n = n;
		its.dpdu = dpdu;
		its.dpdv = dpdv;
		its.dndu = dndu;
		its.dndv = dndv;
		its.wo = -localRay.dir;
		its = (*this->localToWorld)(its);


		return its;
	}

	//��Բ�Ͼ��Ȳ���һ����
	std::tuple<SurfaceInteraction, double> Sphere::sample(const Point2f& uv)const {
		//������p�뷨��n
		Point3f p = UniformSampleSphere(uv);
		Normal3f n(p);

		p *= radius;
		//��n��p�任����������
		p = (*localToWorld)(p);
		n = (*localToWorld)(n);

		SurfaceInteraction inter;
		inter.p = p;
		inter.n = n;

		//����pdf
		double pdf = 1 / area();
		return std::tuple<SurfaceInteraction, double>(inter, pdf);
	}

	//���ڵ�p����Բ�ϵ�һ���㣬���ص�pdfΪ������ǵĻ���
	std::tuple<SurfaceInteraction, double> Sphere::sample(const SurfaceInteraction& inter, const Point2f& uv)const {
		Point3f pCenter = (*localToWorld)(Point3f(0.0));//Բ�ĵ���������

		//��p����Բ�ڣ����Ȳ���
		double distanceSquared = DistanceSquared(pCenter, inter.p);
		if (distanceSquared < radius * radius) {
			auto [lightInter, pdf] = sample(uv);

			//��������pdfתΪ������ǵĻ���
			Vector3f wi = lightInter.p - inter.p;
			double dis2 = wi.lengthSquared();
			if (dis2 == 0.0)
				pdf = 0.0;
			else {
				wi = Normalize(wi);
				pdf *= DistanceSquared(lightInter.p, inter.p) / abs(Dot(lightInter.n, -wi));
			}
			if (std::isinf(pdf))
				pdf = 0.0;
			return std::tuple<SurfaceInteraction, double>(lightInter, pdf);
		}

		//����p��ɼ�Բ׶
		else {
			//�����µ�����ϵ��ʹ�õ�p��Բ�ĵ�����λ��������ϵ��z��
			Vector3f z = Normalize(pCenter - inter.p);//�ɲο���Ϊԭ�㣬z��ָ��Բ��
			auto [x, y] = genTBN(z);

			//����p����Բ��Բ׶�ϲ���һ������
			//����cosThetaMax
			double dc = Distance(inter.p, pCenter);
			double sinThetaMax = radius / dc;
			double sinThetaMax2 = sinThetaMax * sinThetaMax;
			double cosThetaMax = sqrt(Max(0.0, 1 - sinThetaMax2));
			//������P�����Ĺ��ߵķ���
			double cosTheta = (1 - uv[0]) + uv[0] * cosThetaMax;
			double sinTheta2 = 1 - cosTheta * cosTheta;
			double phi = 2 * uv[1] * M_PI;

			//������p�����������������������Բ�Ľ���pL
			//�������pL����Բ�ĵĽǶ�alpha
			double ds = dc * cosTheta - std::sqrt(Max(0.0, radius * radius - dc * sinTheta2));
			double cosAlpha = (dc * dc + radius * radius - ds * ds) /
				(2 * dc * radius);
			double sinAlpha = std::sqrt(Max(0.0, 1 - cosTheta * cosTheta));

			Vector3f nWorld = sinAlpha * cos(phi) * (-x) + sinAlpha * sin(phi) * (-y) + cosAlpha * (-z);
			Point3f  pWorld = pCenter + nWorld * radius;

			SurfaceInteraction lightSample;
			lightSample.p = pWorld;
			lightSample.n = Normal3f(nWorld);

			double pdf = 1 / (2 * M_PI * (1 - cosThetaMax));

			return std::tuple<SurfaceInteraction, double>(lightSample, pdf);
		}
	}

	double Sphere::pdf(const SurfaceInteraction& inter, const Vector3f& wi)const {
		Point3f pCenter = (*localToWorld)(Point3f(0.0));
		double dc2 = DistanceSquared(pCenter, inter.p);

		if (dc2 < radius * radius)
			//����Բ�ڣ����Ȳ�������Բ���任pdfΪ������ǵĻ���
			return Shape::pdf(inter, wi);

		//����Բ�⣬����cosThetaMax��ֱ�Ӽ���pdf
		double sinThetaMax2 = radius * radius / dc2;
		double cosThetaMax = std::sqrt(1 - sinThetaMax2);
		return UniformConePdf(cosThetaMax);
	}

	std::shared_ptr<Sphere> Sphere::build(const Transform* LTW, const Transform* WTL,
		double radius, double zMax, double zMin, double phiMax) {
		return std::make_shared<Sphere>(LTW, WTL, radius, zMax, zMin, phiMax);
	}
}