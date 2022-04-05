#include"plastic.h"
#include"../shading_model/lambertain.h"
#include"../shading_model/microfacet.h"

namespace Raven {

	void Plastic::computeScarttingFunctions(SurfaceInteraction& hitRecord)const {

		std::shared_ptr<BSDF> bsdf = std::make_shared <BSDF>(hitRecord, hitRecord.eta);

		//evaluate texture
		Vector3f kdValue = kd->evaluate(hitRecord);
		Vector3f ksValue = ks->evaluate(hitRecord);
		double roughValue = roughness->evaluate(hitRecord);

		//���shading models

		//diffuse
		if (kdValue != Vector3f(0.0, 0.0, 0.0)) {
			std::shared_ptr<BxDF> lam = std::make_shared<LambertainReflection>(kdValue);
		//	bsdf->addBxDF(lam);
		}

		//glossy
		if (ksValue != Vector3f(0.0, 0.0, 0.0)) {
			std::shared_ptr<Fresnel> fresnel = std::make_shared<FresnelDielectric>(1.85f, 1.f);
			double alpha = GGX::RoughnessToAlpha(roughValue);
			std::shared_ptr<MicrofacetDistribution> distribute =
				std::make_shared<GGX>(roughValue, roughValue,true);

			std::shared_ptr<BxDF> spec = 
				std::make_shared<MicrofacetReflection>(fresnel, distribute, ksValue);

			bsdf->addBxDF(spec);
		}
		hitRecord.bsdf = bsdf;
	}

	std::shared_ptr<Plastic> Plastic::build(
		const std::shared_ptr<Texture<Vector3f>>& kd,
		const std::shared_ptr<Texture<Vector3f>>& ks, 
		const std::shared_ptr<Texture<double>>& roughness,
		const std::shared_ptr<Texture<double>>& bump) {
		return std::make_shared<Plastic>(kd, ks, roughness, bump);
	}
}