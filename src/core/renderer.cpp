#include"renderer.h"
#include"light.h"

namespace Raven {

	Vector3f EvaluateLight(const SurfaceInteraction& record, const Scene& scene, const Light& light) {

		Vector3f L(0.0);

		//�ӹ�Դ�ķֲ��ϲ���
		LightSample lSample;
		Vector3f Le = light.sampleLi(record, Point2f(GetRand(), GetRand()), &lSample);
		if (Le != Vector3f(0.0) && lSample.pdf > 0) {

			//����brdf��scarttingPdf
			Vector3f f = record.bsdf->f(record.wo, lSample.wi) * std::abs(Dot(lSample.wi, record.n));
			double scarttingPdf = record.bsdf->pdf(record.wo, lSample.wi);
			if (f != Vector3f(0.0) && scarttingPdf > 0.0) {

				//�ж�shadow ray�Ƿ��ڵ�
				Ray shadowRay(record.p, lSample.wi);
				SurfaceInteraction test;
				double distance = (record.p - lSample.p).length();
				if (scene.intersect(shadowRay, test, 1e-6, distance - 1e-6))
					Le = Vector3f(0.0);	//���shadow ray���ڵ�������Radiance=0

				//����˴β����Ĺ���
				if (Le != Vector3f(0.0)) {
					double weight = PowerHeuristic(1, lSample.pdf, 1, scarttingPdf);
					L += Le * f * weight / lSample.pdf;
					//L += Le * f / lSample.pdf;
				}
			}
		}

		//��brdf�ķֲ��ϲ���

		//����brdf
		Vector3f wi;
		double scarttingPdf;
		Vector3f f = record.bsdf->sample_f(record.wo, wi, Point2f(GetRand(), GetRand()), &scarttingPdf);
		double cosTheta = abs(Dot(wi, record.n));
		f *= cosTheta;

		double lightPdf = 1.0;
		if (f != Vector3f(0.0) && scarttingPdf > 0) {

			//����lightPdf����MISȨ��
			lightPdf = light.pdf_Li(record, wi);
			if (lightPdf == 0)
				return L;
			double weight = PowerHeuristic(1, scarttingPdf, 1, lightPdf);

			//����shadow ray �жϹ�Դ�Ƿ��ڵ�
			Ray shadowRay(record.p, wi);
			SurfaceInteraction lightIsect;
			bool foundIntersection = scene.intersect(shadowRay, lightIsect, 1e-6, std::numeric_limits<double>::max());

			//������shadow rayδ���ڵ��������Դ�Ĺ���
			if (foundIntersection && lightIsect.hitLight && lightIsect.light == &light) {
				Vector3f Le = light.Li(lightIsect, -wi);
				if (Le != Vector3f(0.0))
					L += weight * f * Le / scarttingPdf;
			}
		}
		return L;
	}

	//��ÿ����Դ�ϲ���һ���㣬����Radiance
	Vector3f SampleAllLights(const SurfaceInteraction& record, const Scene& scene) {
		Vector3f Le(0.0);
		for (auto light : scene.lights) {
			Le += EvaluateLight(record, scene, *light.get());
		}
		return Le;
	}

	Vector3f SampleOneLight(const SurfaceInteraction& record, const Scene& scene, int nSample) {
		Vector3f Le(0.0);
		const std::vector<std::shared_ptr<Light>>& lights = scene.lights;
		Vector3f totalPower(0.0);
		for (auto light : lights) {
			totalPower += light->power();
		}
		Vector3f power;
		double pr = GetRand();
		double p = 1.0;
		Light* chosen;
		for (size_t i = 0; i < lights.size(); i++) {
			power += lights[i]->power();
			p = power.y / totalPower.y;
			if (p >= pr || i == lights.size() - 1) {
				chosen = lights[i].get();
				break;
			}
		}

		for (int i = 0; i < nSample; i++)
			Le += EvaluateLight(record, scene, *chosen) / p;
		return Le / nSample;
	}
}