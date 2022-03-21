#include"path.h"
#include"../core/light.h"
namespace Raven {
	void PathTracingRenderer::render(const Scene& scene) {
		for (int i = 0; i < film.height; ++i) {
			std::cerr << "\rScanlines remaining: " << film.height - 1 - i << ' ' << std::flush;
			for (int j = 0; j < film.width; ++j) {
				Vector3f pixelColor(0.0);
				//auto cu = double(j);
				//auto cv = double(i);
				//CameraSample cs(cu, cv, 0.0, 0.0, 0.0);
				//Ray gr;
				//if (camera->GenerateRay(cs, gr)) {
				//	film.setGBuffer(gBuffer(gr,scene),j,i);
				//}

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
				film.in(pixelColor);
			}
		}
		std::cerr << "\nDone.\n";
		film.write();
		//		film.writeColor();
	}

	//·��׷���㷨����ʱֻ������lambertain
	Vector3f PathTracingRenderer::integrate(const Scene& scene, const Ray& rayIn)const {

		Vector3f Li(0.0);
		Vector3f beta(1.0, 1.0, 1.0);//���ߵ�˥������
		Ray ray = rayIn;
		for (int depth = 0; depth < maxDepth; depth++) {

			//��ȡ��������ߵ��ཻ��Ϣ
			SurfaceInteraction record;
			if (scene.intersect(ray, record, epsilon, std::numeric_limits<double>::max())) {
				//�����볡���ཻ���ཻ����Ϣ��������record��

				Point3f p = record.p;
				Vector3f wo = Normalize(-ray.dir);
				Normal3f n = record.n;
				Vector3f wi;

				//ֻ�д���������Ĺ��߻��й�Դ��ֱ�ӷ��ع�Դ��emittion
				if (depth == 0 && record.hitLight) {
					//����������Ĺ���ֱ�ӻ��й�Դ
					Li += record.light->Li(record, wo);
					break;
				}

				//������Դ,�����Ըý���Ϊ�յ��·���Ĺ���
				LightSample lSample;
				Vector3f emit = scene.sampleLight(record, GetRand(), Point2f(GetRand(), GetRand()), &lSample);
				Vector3f fLight = record.bsdf->f(wo, lSample.wi);
				double length = (lSample.p - p).length();
				Vector3f dirLi = emit * fLight * Dot(lSample.wi, n) * Dot(-lSample.wi, lSample.n)
					/ (length * length * lSample.pdf);
				//�ж������ڵ�
				Ray shadowRay(p, lSample.wi);
				SurfaceInteraction test;//scene.hit��������bug������intersect����һ��
				bool blocked = scene.intersect(shadowRay, test, epsilon, length - epsilon);
				if (!blocked)
					Li += dirLi * beta;


				//����brdf��������䷽��,����beta
				double pdf;
				Vector3f f = record.bsdf->sample_f(wo, wi, Point2f(GetRand(), GetRand()), &pdf);
				double cosTheta = Clamp(Dot(wi, n), 0.0, 1.0);
				beta *= f * cosTheta / pdf;
				ray = Ray(record.p, wi);

				if (depth > 3) {
					double q = Max((double).05, 1 - beta.y);
					if (GetRand() < q)
						break;
					beta /= 1 - q;
				}
			}
			//����δ�볡���ཻ
			else {
				break;
			}
		}
		return Li;
	}

	//if (depth >= maxDepth)
	//	return Vector3f(0.0);
	//SurfaceInteraction sinter;
	//if (scene.intersect(r_in, sinter, epsilon, std::numeric_limits<double>::max())) {
	//	//get shading data
	//
	//
	//	Vector3f L_dir(0.0);
	//	Vector3f L_ind(0.0);
	//	Vector3f wo = Normalize(-r_in.dir);
	//	Vector3f wi;
	//	Vector3f n = Normalize(Vector3f(sinter.n));
	//	const Point3f& p = sinter.p;
	//
	//	//��������Area Light
	//	if (sinter.hitLihgt) {
	//		return sinter.light->Li(sinter, wo);
	//	}
	//
	//	//������Դ������ֱ�ӹ���
	//	double rand = GetRand();
	//	const Light* light = scene.chooseLight(rand);
	//	Point2f randuv(GetRand(), GetRand());
	//	SurfaceInteraction lightSample;
	//	Vector3f wiLight;//�ӽ���ָ���Դ
	//	double pdfLight = 0.0;
	//
	//	for (int i = 0; i < 10; i++) {
	//		Vector3f emit = light->sample_Li(sinter, randuv, &wiLight, &pdfLight, &lightSample);
	//		//	std::cout << emit;
	//		const Point3f& pLight = lightSample.p;
	//		const Normal3f& nLight = lightSample.n;
	//		Ray shadowRay(p, wiLight);
	//		Vector3f temp = (pLight - p);
	//		double t = temp.length();
	//		//�ж�Shadow Ray�Ƿ񱻳����ڵ�
	//		SurfaceInteraction test;
	//		if (!scene.intersect(shadowRay, test, 0.000001, t - 0.001)) {
	//
	//			L_dir += emit * sinter.bsdf->f(wo, wiLight) * Dot(wiLight, n) * Dot(-wiLight, nLight) / (t * t * pdfLight);
	//		}
	//	}
	//	L_dir /= 10;
	//
	//	//����brdf,��ӹ���
	//	double pdf;
	//	Vector3f f = sinter.bsdf->sample_f(wo, wi, Point2f(GetRand(), GetRand()), &pdf);//sample out direction
	//	double cosTheta = Clamp(Dot(wi, n), 0.0, 1.0);
	//	Ray r_out(sinter.p, wi);
	//	L_ind = f * integrate(scene, r_out, depth + 1) * cosTheta / pdf;
	//
	//	return L_ind + L_dir;
	//}
	//return Vector3f(0.0);
//}

	GeometryData PathTracingRenderer::gBuffer(const Ray& ray, const Scene& scene)const {
		SurfaceInteraction record;
		GeometryData data;

		if (scene.intersect(ray, record, 0.001, std::numeric_limits<double>::max())) {
			const Point3f& p = record.p;
			const Normal3f& n = record.n;
			data.n = record.n;
			data.p = record.p;
			data.hit = true;
		}
		return data;
	}

}