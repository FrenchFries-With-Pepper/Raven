#include"path.h"
#include<omp.h>
#include"../core/light.h"

static omp_lock_t lock;
namespace Raven {
	void PathTracingRenderer::render(const Scene& scene) {
		int finishedLine = 1;

		omp_init_lock(&lock);

#pragma omp parallel for
		for (int i = 0; i < film.height; ++i) {
			//		std::cerr << "\rScanlines remaining: " << film.height - 1 - i << ' ' << std::flush;
			double process = (double)finishedLine / film.height;

			omp_set_lock(&lock);
			UpdateProgress(process);
			omp_unset_lock(&lock);

			for (int j = 0; j < film.width; ++j) {
				Spectrum pixelColor(0.0);
				for (int s = 0; s < spp; s++) {
					//camera sample
					auto cu = double(j) + GetRand();
					auto cv = double(i) + GetRand();
					auto fu = GetRand();
					auto fv = GetRand();
					auto t = GetRand();
					CameraSample sample(cu, cv, t, fu, fv);
					Ray r;

					if (camera->GenerateRay(sample, r)) {
						pixelColor += integrate(scene, r);
					}
				}
				double scaler = 1.0 / spp;
				pixelColor *= scaler;

				film.setColor(pixelColor, j, i);
				//film.in(pixelColor);
			}
			finishedLine++;


		}
		omp_destroy_lock(&lock);
		std::cout << "\nDone.\n";
		film.write();
		film.writeTxt();
	}
	//·��׷���㷨����ʱֻ������lambertain
	Spectrum PathTracingRenderer::integrate(const Scene& scene, const Ray& rayIn, int bounce)const {
		//	Spectrum backgroundColor = Spectrum(Spectrum::fromRGB(0.235294, 0.67451, 0.843137));
		Spectrum Li(0.0);
		Spectrum beta(1.0);//���ߵ�˥������
		Ray ray = rayIn;

		bool specularBounce = false;
		double etaScale = 1;
		for (; bounce < maxDepth; bounce++) {

			//��ȡ��������ߵ��ཻ��Ϣ
			std::optional<SurfaceInteraction> record = scene.intersect(ray, std::numeric_limits<double>::max());
			//����δ�볡���ཻ
			if (!record) {
				break;
			}
			else {
				//�����볡���ཻ���ཻ����Ϣ��������record��
				Point3f p = record->p;
				Vector3f wo = Normalize(-ray.dir);
				Normal3f n = record->n;

				//ֻ�д���������Ĺ��߻��й�Դ��ֱ�ӷ��ع�Դ��emittion
				if (bounce == 0 || specularBounce) {

					//����������Ĺ���ֱ�ӻ��й�Դ
					if (record->hitLight) {
						Spectrum emittion = record->light->Li(*record, wo);
						Li += beta * emittion;
						return Li;
					}
				}

				for (auto& light : scene.lights) {
					//������Դ,�����Ըý���Ϊ�յ��·���Ĺ���
					LightSample lightSample;
					Spectrum emit = light->sampleLi(*record, Point2f(GetRand(), GetRand()), &lightSample);
					Spectrum fLight = record->bsdf->f(wo, lightSample.wi);
					double length = (lightSample.p - p).length();
					//double dot1 = Max(0.0, Dot(lightSample.wi, n));
					double dot2 = Max(0.0, Dot(-lightSample.wi, lightSample.n));

					double dot1 = Max(0.0, Dot(lightSample.wi, n));
					//double dot2 = abs(Dot(-lightSample.wi, lightSample.n));
					Spectrum dirLi = emit * fLight * dot1 * dot2 / lightSample.pdf;
					//�ж������ڵ�
					//TODO::Debug scene->hit����������õ�hit������ʹ��hit����intersect

					Ray shadowRay(p, lightSample.wi);
					if (!scene.hit(shadowRay, length - epsilon))
						Li += dirLi * beta;
				}

				//Vector3f L_dir = SampleAllLights(*record, scene);
				//Li += beta * L_dir;


				//����brdf��������䷽��,����beta
				auto [f, wi, pdf, sampledType] = record->bsdf->sample_f(wo, Point2f(GetRand(), GetRand()));
				if (f == Spectrum(0.0) || pdf == 0.0)
					break;

				//����˥��
				double cosTheta = abs(Dot(wi, n));
				beta *= f * cosTheta / pdf;
				specularBounce = (sampledType & BxDFType::Specular) != 0;
				ray = record->scartterRay(wi);

				//����˹���̶Ľ���ѭ��
				if (bounce > 3) {
					double q = Max((double).05, 1 - beta.y());
					if (GetRand() < q)
						break;
					beta /= 1 - q;
				}
			}

		}
		return Li;
	}

	//GeometryData PathTracingRenderer::gBuffer(const Ray& ray, const Scene& scene)const {
	//	GeometryData data;
	//	std::optional<SurfaceInteraction> record = scene.intersect(ray, std::numeric_limits<double>::max());
	//	if (record != std::nullopt) {
	//		const Point3f& p = (*record).p;
	//		const Normal3f& n = (*record).n;
	//		data.n = (*record).n;
	//		data.p = (*record).p;
	//		data.hit = true;
	//	}
	//	return data;
	//}

}