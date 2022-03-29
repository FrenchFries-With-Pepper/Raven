#include"path.h"
#include"../core/light.h"
namespace Raven {
	void PathTracingRenderer::render(const Scene& scene) {
		for (int i = 0; i < film.height; ++i) {
			std::cerr << "\rScanlines remaining: " << film.height - 1 - i << ' ' << std::flush;
			for (int j = 0; j < film.width; ++j) {
				Vector3f pixelColor(0.0);
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
						//if (i == 360 && j == 168)
						//	std::cout << "?";
						pixelColor += integrate(scene, r);
					}
				}
				//if (i == 300 || j == 228 || j == 108 || i == 425) {
				//	pixelColor = Vector3f(0.0);
				//}
				double scaler = 1.0 / spp;
				pixelColor *= scaler;

				film.in(pixelColor);
			}
		}
		std::cerr << "\nDone.\n";
		film.write();
		film.writeTxt();
	}

	//·��׷���㷨����ʱֻ������lambertain
	Vector3f PathTracingRenderer::integrate(const Scene& scene, const Ray& rayIn, int depth)const {
		Vector3f backgroundColor = Vector3f(0.235294, 0.67451, 0.843137);
		Vector3f Li(0.0);
		Vector3f beta(1.0);//���ߵ�˥������
		Ray ray = rayIn;
		for (; depth < maxDepth; depth++) {

			//��ȡ��������ߵ��ཻ��Ϣ
			std::optional<SurfaceInteraction> record = scene.intersect(ray, epsilon, std::numeric_limits<double>::max());
			//����δ�볡���ཻ
			if (!record) {
				//Li += backgroundColor * beta;
				break;
			}
			else {
				//�����볡���ཻ���ཻ����Ϣ��������record��
				Point3f p = record->p;
				Vector3f wo = Normalize(-ray.dir);
				Normal3f n = record->n;
				Vector3f wi;

				//ֻ�д���������Ĺ��߻��й�Դ��ֱ�ӷ��ع�Դ��emittion
				if (depth == 0 && (*record).hitLight) {
					//����������Ĺ���ֱ�ӻ��й�Դ
					Li += record->light->Li(*record, wo);
					return Li;
				}

				for (auto& light : scene.lights) {
					//������Դ,�����Ըý���Ϊ�յ��·���Ĺ���
					LightSample lightSample;
					Vector3f emit = light->sampleLi(*record, Point2f(GetRand(), GetRand()), &lightSample);
					Vector3f fLight = record->bsdf->f(wo, lightSample.wi);
					double length = (lightSample.p - p).length();
					double dot1 = Max(0.0,Dot(lightSample.wi, n));
					double dot2 = Max(0.0,Dot(-lightSample.wi, lightSample.n));
					Vector3f dirLi = emit * fLight * dot1 / lightSample.pdf;
					//�ж������ڵ�
					//TODO::Debug scene->hit����������õ�hit������ʹ��hit����intersect
					Ray shadowRay(p, lightSample.wi);
					std::optional<SurfaceInteraction> test = scene.intersect(shadowRay, epsilon, length - 0.1);
					if (test == std::nullopt)
						Li += dirLi * beta;
				}

				//Vector3f L_dir = SampleAllLights(*record, scene);
				//Li += beta * L_dir;

				//����brdf��������䷽��,����beta
				double pdf;
				Vector3f f = record->bsdf->sample_f(wo, wi, Point2f(GetRand(), GetRand()), &pdf);
				double cosTheta = Max(0.0,Dot(wi, n));
				beta *= f * cosTheta / pdf;
				if (std::isnan(wi[0]))
					std::cout << "?";
				ray = Ray(record->p, wi);

				if (depth > 3) {
					double q = Max((double).05, 1 - beta.y);
					if (GetRand() < q)
						break;
					beta /= 1 - q;
				}
			}

		}
		return Li;
	}

	GeometryData PathTracingRenderer::gBuffer(const Ray& ray, const Scene& scene)const {
		GeometryData data;
		std::optional<SurfaceInteraction> record = scene.intersect(ray, 1e-6, std::numeric_limits<double>::max());
		if (record != std::nullopt) {
			const Point3f& p = (*record).p;
			const Normal3f& n = (*record).n;
			data.n = (*record).n;
			data.p = (*record).p;
			data.hit = true;
		}
		return data;
	}

}