#include"path.h"
#include"../core/light.h"
namespace Raven {
	void PathTracingRenderer::render(const Scene& scene) {
		for (int i = 0; i < film.height; ++i) {
			std::cerr << "\rScanlines remaining: " << film.height - 1 - i << ' ' << std::flush;
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
						//if (i == 310 && j == 140)
						//	std::cout << "?";
						pixelColor += integrate(scene, r);
					}
				}
				//if (i == 300 || j == 350 || j == 135 || i == 350) {
				//	pixelColor = Vector3f(1.0,1.0,0.0);
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

	//路径追踪算法，暂时只考虑了lambertain
	Spectrum PathTracingRenderer::integrate(const Scene& scene, const Ray& rayIn, int depth)const {
		//	Spectrum backgroundColor = Spectrum(Spectrum::fromRGB(0.235294, 0.67451, 0.843137));
		Spectrum Li(0.0);
		Spectrum beta(1.0);//光线的衰减参数
		Ray ray = rayIn;
		for (; depth < maxDepth; depth++) {

			//获取场景与光线的相交信息
			std::optional<SurfaceInteraction> record = scene.intersect(ray, std::numeric_limits<double>::max());
			//光线未与场景相交
			if (!record) {
				//Li += backgroundColor * beta;
				break;
			}
			else {
				//光线与场景相交，相交的信息都储存在record中
				Point3f p = record->p;
				Vector3f wo = Normalize(-ray.dir);
				Normal3f n = record->n;
				Vector3f wi;

				//只有从相机出发的光线击中光源才直接返回光源的emittion
				if (depth == 0 && (*record).hitLight) {
					//从相机出发的光线直接击中光源
					Li += record->light->Li(*record, wo);
					return Li;
				}

				for (auto& light : scene.lights) {
					//采样光源,计算以该交点为终点的路径的贡献
					LightSample lightSample;
					Spectrum emit = light->sampleLi(*record, Point2f(GetRand(), GetRand()), &lightSample);
					Spectrum fLight = record->bsdf->f(wo, lightSample.wi);
					double length = (lightSample.p - p).length();
					double dot1 = Max(0.0, Dot(lightSample.wi, n));
					double dot2 = Max(0.0, Dot(-lightSample.wi, lightSample.n));
					Spectrum dirLi = emit * fLight * dot1 / lightSample.pdf;
					//判断有无遮挡
					//TODO::Debug scene->hit函数及其调用的hit函数，使用hit代替intersect
					Ray shadowRay(p, lightSample.wi);
					std::optional<SurfaceInteraction> test = scene.intersect(shadowRay, length - 0.001);
					if (test == std::nullopt)
						Li += dirLi * beta;
				}

				//Vector3f L_dir = SampleAllLights(*record, scene);
				//Li += beta * L_dir;

				//采样brdf，计算出射方向,更新beta
				double pdf;
				Spectrum f = record->bsdf->sample_f(wo, wi, Point2f(GetRand(), GetRand()), &pdf);
				if (f == Spectrum(0.0) || pdf == 0.0)
					break;
				double cosTheta = Max(0.0, Dot(wi, n));
				beta *= f * cosTheta / pdf;

				ray = record->scartterRay(wi);

				if (depth > 3) {
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