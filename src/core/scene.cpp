#include"scene.h"
#include"../material/matte.h"
#include"../shape/sphere.h"
#include"../shape/mesh.h"
#include"../light/areaLight.h"
#include"../utils/loader.h"
#include"../texture/solidTexture.h"
#include"../material/plastic.h"
#include"../core/Spectrum.h"
#include"../texture/mapping.h"
#include"../accelerate/bvh.h"

namespace Raven {

	Scene::Scene(
		const std::vector<std::shared_ptr<Transform>>& trans,
		const std::vector<std::shared_ptr<Light>>& lights,
		const std::vector<std::shared_ptr<TriangleMesh>>& meshes,
		const std::vector<std::shared_ptr<Primitive>>& prims,
		AccelType type) :
		meshes(meshes), lights(lights), transforms(trans) {

		switch (type) {
		case AccelType::List:
			objs = std::make_shared<PrimitiveList>(prims);
			break;
		case AccelType::KdTree:
			objs = std::make_shared<KdTreeAccel>(prims,10, 80, 1, 0.5, 5);
			break;
		case AccelType::BVH:
			objs = std::make_shared<BVHAccel>(prims, 1);
			break;
		}

	}

	Scene::Scene(const Scene& s) :transforms(s.transforms),
		lights(s.lights), meshes(s.meshes), objs(s.objs) {}

	const Light* Scene::chooseLight(double rand) const {
		Spectrum totalPower(0.0);
		const Light* light;
		for (int i = 0; i < lights.size(); i++) {
			totalPower += lights[i]->power();
		}
		Spectrum power(0.0);
		double p = 1.0;
		for (int i = 0; i < lights.size(); i++) {
			power += lights[i]->power();
			p = power.y() / totalPower.y();
			if (p >= rand || i == lights.size() - 1) {
				return lights[i].get();
			}
		}
	}

	Scene Scene::buildCornellBox() {
		std::vector<std::shared_ptr<TriangleMesh>> meshes;
		std::vector<std::shared_ptr<Transform>> usedTransform;
		std::vector<std::shared_ptr<Light>> lights;
		std::vector<std::shared_ptr<Primitive>> prim_ptrs;

		//transform
		std::shared_ptr<Transform> identity = std::make_shared<Transform>(Identity());
		usedTransform.push_back(identity);

		//Texture
		std::shared_ptr<TextureMapping2D> mapping = UVMapping2D::build();

		std::shared_ptr<Texture<Spectrum>> greenTexture =
			ConstTexture<Spectrum>::build(RGBSpectrum::fromRGB(0.843137, 0.843137, 0.091f));
		std::shared_ptr<Texture<Spectrum>> blackTexture =
			ConstTexture<Spectrum>::build(RGBSpectrum::fromRGB(0.235294, 0.67451, 0.843137));
		std::shared_ptr<Texture<Spectrum>> cheTex = CheckeredTexture<Spectrum>::build(greenTexture, blackTexture, mapping);
		std::shared_ptr<Texture<double>> sigma = ConstTexture<double>::build(0.0);
		//Material
		std::shared_ptr<MatteMaterial> mate1 = MatteMaterial::buildConst(0.0, RGBSpectrum::fromRGB(0.1, 0.97, 0.4));
		std::shared_ptr<MatteMaterial> mate2 = MatteMaterial::buildConst(0.0, RGBSpectrum::fromRGB(0.5, 0.5, 0.5));
		std::shared_ptr<MatteMaterial> redLam = MatteMaterial::buildConst(0.0, RGBSpectrum::fromRGB(0.63f, 0.065f, 0.05f));
		std::shared_ptr<MatteMaterial> greenLam = MatteMaterial::buildConst(0.0, RGBSpectrum::fromRGB(0.14f, 0.45f, 0.091f));
		std::shared_ptr<MatteMaterial> whiteLam = MatteMaterial::buildConst(0.0, RGBSpectrum::fromRGB(0.725f, 0.71f, 0.68f));
		std::shared_ptr<MatteMaterial> lightLam = MatteMaterial::buildConst(0.0, RGBSpectrum::fromRGB(0.65f));
		std::shared_ptr<MatteMaterial> checkered = MatteMaterial::build(sigma, cheTex);

		//Shape
		//std::shared_ptr<Sphere> s = std::make_shared<Sphere>(sphereLocToPrim.get(), spherePrimToLoc.get(), 80.0, 80.0, -80.0, 2 * M_PI);
		//std::shared_ptr<Sphere> ground = std::make_shared<Sphere>(sphereLocToPrim.get(), spherePrimToLoc.get(), 16000., 16000., -16000., 2 * M_PI);

		Loader loader;
		std::optional<TriangleInfo> leftInfo =
			loader.loadObj("D:/MyWorks/Raven/models/cornellbox/left.obj");
		std::optional<TriangleInfo> rightInfo =
			loader.loadObj("D:/MyWorks/Raven/models/cornellbox/right.obj");
		std::optional<TriangleInfo> floorInfo =
			loader.loadObj("D:/MyWorks/Raven/models/cornellbox/floor.obj");
		std::optional<TriangleInfo> sBoxInfo =
			loader.loadObj("D:/MyWorks/Raven/models/cornellbox/shortbox.obj");
		std::optional<TriangleInfo> tBoxInfo =
			loader.loadObj("D:/MyWorks/Raven/models/cornellbox/tallbox.obj");
		std::optional<TriangleInfo> lightInfo =
			loader.loadObj("D:/MyWorks/Raven/models/cornellbox/light.obj");

		std::shared_ptr<TriangleMesh> leftMesh =
			TriangleMesh::build(identity.get(), identity.get(), *leftInfo);

		std::shared_ptr<TriangleMesh> rightMesh =
			TriangleMesh::build(identity.get(), identity.get(), *rightInfo);

		std::shared_ptr<TriangleMesh> floorMesh =
			TriangleMesh::build(identity.get(), identity.get(), *floorInfo);

		std::shared_ptr<TriangleMesh> sBoxMesh =
			TriangleMesh::build(identity.get(), identity.get(), *sBoxInfo);

		std::shared_ptr<TriangleMesh> tBoxMesh =
			TriangleMesh::build(identity.get(), identity.get(), *tBoxInfo);

		std::shared_ptr<TriangleMesh> lightMesh =
			TriangleMesh::build(identity.get(), identity.get(), *lightInfo);

		meshes.push_back(leftMesh);
		meshes.push_back(rightMesh);
		meshes.push_back(floorMesh);
		meshes.push_back(sBoxMesh);
		meshes.push_back(tBoxMesh);
		meshes.push_back(lightMesh);


		//Point3f sp0(-50.0, 500.0, 200.0);
		//Point3f sp1(50.0, 500.0, 200.0);
		//Point3f sp2(50.0, 500.0, 100.0);
		//Point3f sp3(-50.0, 500.0, 100.0);

		std::vector<std::shared_ptr<Triangle>> sTri = lightMesh->getTriangles();
		//meshes.push_back(sqr);

		//Light
		Spectrum lightEmit = RGBSpectrum::fromRGB(8.0 * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f)
			+ 15.6 * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) + 18.4 *
			Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f));
		std::shared_ptr<DiffuseAreaLight> aLight1 =
			std::make_shared<DiffuseAreaLight>(identity.get(), identity.get(), 5, sTri[0].get(), lightEmit);
		std::shared_ptr<DiffuseAreaLight> aLight2 =
			std::make_shared<DiffuseAreaLight>(identity.get(), identity.get(), 5, sTri[1].get(), lightEmit);
		std::shared_ptr<Primitive> lightp1 = std::make_shared<Primitive>(sTri[0], lightLam, aLight1);
		std::shared_ptr<Primitive> lightp2 = std::make_shared<Primitive>(sTri[1], lightLam, aLight2);
		lights.push_back(aLight1);
		lights.push_back(aLight2);

		//std::shared_ptr<TriangleMesh> square = std::make_shared<TriangleMesh>(CreatePlane(identity.get(), identity.get(),
		//	p0, p1, p2, p3, Normal3f(0.0, 1.0, 0.0)));
		//std::vector<std::shared_ptr<Primitive>> squareTri = square->generatePrimitive(mate2);
		//meshes.push_back(square);

		//Primitive
		std::vector<std::shared_ptr<Primitive>> leftPrim = leftMesh->generatePrimitive(redLam);
		std::vector<std::shared_ptr<Primitive>>  rightPrim = rightMesh->generatePrimitive(greenLam);
		std::vector<std::shared_ptr<Primitive>>  floorPrim = floorMesh->generatePrimitive(whiteLam);
		std::vector<std::shared_ptr<Primitive>>  sBoxPrim = sBoxMesh->generatePrimitive(whiteLam);
		std::vector<std::shared_ptr<Primitive>> tBoxPrim = tBoxMesh->generatePrimitive(whiteLam);

		prim_ptrs.push_back(lightp1);
		prim_ptrs.push_back(lightp2);
		prim_ptrs.insert(prim_ptrs.end(), leftPrim.begin(), leftPrim.end());
		prim_ptrs.insert(prim_ptrs.end(), rightPrim.begin(), rightPrim.end());
		prim_ptrs.insert(prim_ptrs.end(), floorPrim.begin(), floorPrim.end());
		prim_ptrs.insert(prim_ptrs.end(), sBoxPrim.begin(), sBoxPrim.end());
		prim_ptrs.insert(prim_ptrs.end(), tBoxPrim.begin(), tBoxPrim.end());

		//prim_ptrs.push_back(squareTri[0]);
		//prim_ptrs.push_back(squareTri[1]);
		//prim_ptrs.push_back(sMiddle);

		//prim_ptrs.push_back(sq);
		//squareTri.push_back(sMiddle);
		return Scene(usedTransform, lights, meshes, prim_ptrs, AccelType::List);

	}

	//Scene Scene::buildTestScene() {
	//	std::vector<std::shared_ptr<TriangleMesh>> meshes;
	//	std::vector<std::shared_ptr<Transform>> usedTransform;
	//	std::vector<std::shared_ptr<Light>> lights;
	//	std::vector<std::shared_ptr<Primitive>> prim_ptrs;

	//	//transform
	//	Eigen::Matrix4f spherePrimitive;
	//	spherePrimitive <<
	//		1.f, 0.f, 0.f, 0.f,
	//		0.f, 0.f, 1.f, 0.f,
	//		0.f, -1.f, 0.f, 0.f,
	//		0.f, 0.f, 0.f, 1.f;

	//	std::shared_ptr<Transform> sphereLocal = std::make_shared<Transform>(spherePrimitive);
	//	std::shared_ptr<Transform> invSphereLocal = std::make_shared<Transform>(spherePrimitive.inverse());
	//	std::shared_ptr<Transform> identity = std::make_shared<Transform>(Identity());
	//	std::shared_ptr<Transform> sphereWorld = std::make_shared<Transform>(Translate(Vector3f(150, 110, 300)));
	//	std::shared_ptr<Transform> invSphereWorld = std::make_shared<Transform>(sphereWorld->inverse());

	//	usedTransform.push_back(sphereLocal);
	//	usedTransform.push_back(invSphereLocal);
	//	usedTransform.push_back(identity);
	//	usedTransform.push_back(sphereWorld);
	//	usedTransform.push_back(invSphereWorld);

	//	std::shared_ptr<Shape> sphere = Sphere::build(sphereWorld.get(), invSphereWorld.get(), 100);

	//	Loader loader;
	//	std::optional<TriangleInfo> leftInfo =
	//		loader.loadObj("D:/MyWorks/Raven/models/cornellbox/left.obj");
	//	std::optional<TriangleInfo> rightInfo =
	//		loader.loadObj("D:/MyWorks/Raven/models/cornellbox/right.obj");
	//	std::optional<TriangleInfo> floorInfo =
	//		loader.loadObj("D:/MyWorks/Raven/models/cornellbox/floor.obj");
	//	//		std::optional<TriangleInfo> sBoxInfo =
	//	//			loader.loadObj("D:/MyWorks/Raven/models/cornellbox/shortbox.obj");
	//	std::optional<TriangleInfo> tBoxInfo =
	//		loader.loadObj("D:/MyWorks/Raven/models/cornellbox/tallbox.obj");
	//	std::optional<TriangleInfo> lightInfo =
	//		loader.loadObj("D:/MyWorks/Raven/models/cornellbox/light.obj");

	//	std::shared_ptr<TriangleMesh> leftMesh =
	//		TriangleMesh::build(identity.get(), identity.get(), *leftInfo);

	//	std::shared_ptr<TriangleMesh> rightMesh =
	//		TriangleMesh::build(identity.get(), identity.get(), *rightInfo);

	//	std::shared_ptr<TriangleMesh> floorMesh =
	//		TriangleMesh::build(identity.get(), identity.get(), *floorInfo);

	//	//std::shared_ptr<TriangleMesh> sBoxMesh =
	//	//	TriangleMesh::build(identity.get(), identity.get(), *sBoxInfo);

	//	std::shared_ptr<TriangleMesh> tBoxMesh =
	//		TriangleMesh::build(identity.get(), identity.get(), *tBoxInfo);

	//	std::shared_ptr<TriangleMesh> lightMesh =
	//		TriangleMesh::build(identity.get(), identity.get(), *lightInfo);

	//	meshes.push_back(leftMesh);
	//	meshes.push_back(rightMesh);
	//	meshes.push_back(floorMesh);
	//	//meshes.push_back(sBoxMesh);
	//	meshes.push_back(tBoxMesh);
	//	meshes.push_back(lightMesh);

	//	//Texture
	//	std::shared_ptr<Texture<Vector3f>> greenTexture = ConstTexture<Vector3f>::build(Vector3f(0.843137, 0.843137, 0.091f));
	//	std::shared_ptr<Texture<Vector3f>> blackTexture = ConstTexture<Vector3f>::build(Vector3f(0.235294, 0.67451, 0.843137));
	//	std::shared_ptr<Texture<Vector3f>> cheTex = CheckeredTexture<Vector3f>::build(greenTexture, blackTexture);
	//	std::shared_ptr<Texture<double>> sigma = ConstTexture<double>::build(0.0);

	//	std::shared_ptr<Texture<Vector3f>> kdTex = ConstTexture<Vector3f>::build(Vector3f(0.2f, 0.3f, 0.25f));
	//	std::shared_ptr<Texture<Vector3f>> ksTex = ConstTexture<Vector3f>::build(Vector3f(0.7f, 0.7f, 0.7f));
	//	std::shared_ptr<Texture<double>> roughTex = ConstTexture<double>::build(0.45);

	//	//Material
	//	std::shared_ptr<MatteMaterial> mate1 = MatteMaterial::buildConst(0.0, Vector3f(0.1, 0.97, 0.4));
	//	std::shared_ptr<MatteMaterial> mate2 = MatteMaterial::buildConst(0.0, Vector3f(0.5, 0.5, 0.5));
	//	std::shared_ptr<MatteMaterial> redLam = MatteMaterial::buildConst(0.0, Vector3f(0.63f, 0.065f, 0.05f));
	//	std::shared_ptr<MatteMaterial> greenLam = MatteMaterial::buildConst(0.0, Vector3f(0.14f, 0.45f, 0.091f));
	//	std::shared_ptr<MatteMaterial> whiteLam = MatteMaterial::buildConst(0.0, Vector3f(0.725f, 0.71f, 0.68f));
	//	std::shared_ptr<MatteMaterial> lightLam = MatteMaterial::buildConst(0.0, Vector3f(0.65f));
	//	std::shared_ptr<MatteMaterial> checkered = MatteMaterial::build(sigma, cheTex);

	//	std::shared_ptr<Plastic> plastic = Plastic::build(kdTex, ksTex, roughTex);

	//	//Light
	//	std::vector<std::shared_ptr<Triangle>> sTri = lightMesh->getTriangles();
	//	Vector3f lightEmit = (8.0 * Vector3f(0.747f + 0.058f, 0.747f + 0.258f, 0.747f)
	//		+ 15.6 * Vector3f(0.740f + 0.287f, 0.740f + 0.160f, 0.740f) + 18.4 *
	//		Vector3f(0.737f + 0.642f, 0.737f + 0.159f, 0.737f));
	//	std::shared_ptr<DiffuseAreaLight> aLight1 =
	//		std::make_shared<DiffuseAreaLight>(identity.get(), identity.get(), 5, sTri[0].get(), lightEmit);
	//	std::shared_ptr<DiffuseAreaLight> aLight2 =
	//		std::make_shared<DiffuseAreaLight>(identity.get(), identity.get(), 5, sTri[1].get(), lightEmit);
	//	std::shared_ptr<Primitive> lightp1 = std::make_shared<Primitive>(sTri[0], lightLam, aLight1);
	//	std::shared_ptr<Primitive> lightp2 = std::make_shared<Primitive>(sTri[1], lightLam, aLight2);
	//	lights.push_back(aLight1);
	//	lights.push_back(aLight2);

	//	//Primitives
	//	std::shared_ptr<Primitive> spherePrim = Primitive::build(sphere, plastic, nullptr);
	//	//std::shared_ptr<Primitive>sworldPrim = TransformedPrimitive::build(sphereWorld.get(), invSphereWorld.get(), sLocalPrim);
	//	std::vector<std::shared_ptr<Primitive>> leftPrim = leftMesh->generatePrimitive(redLam);
	//	std::vector<std::shared_ptr<Primitive>>  rightPrim = rightMesh->generatePrimitive(greenLam);
	//	std::vector<std::shared_ptr<Primitive>>  floorPrim = floorMesh->generatePrimitive(whiteLam);
	//	//std::vector<std::shared_ptr<Primitive>>  sBoxPrim = sBoxMesh->generatePrimitive(whiteLam);
	//	std::vector<std::shared_ptr<Primitive>> tBoxPrim = tBoxMesh->generatePrimitive(whiteLam);

	//	prim_ptrs.push_back(lightp1);
	//	prim_ptrs.push_back(lightp2);
	//	prim_ptrs.push_back(spherePrim);
	//	prim_ptrs.insert(prim_ptrs.end(), leftPrim.begin(), leftPrim.end());
	//	prim_ptrs.insert(prim_ptrs.end(), rightPrim.begin(), rightPrim.end());
	//	prim_ptrs.insert(prim_ptrs.end(), floorPrim.begin(), floorPrim.end());
	//	//prim_ptrs.insert(prim_ptrs.end(), sBoxPrim.begin(), sBoxPrim.end());
	//	prim_ptrs.insert(prim_ptrs.end(), tBoxPrim.begin(), tBoxPrim.end());

	//	return Scene(usedTransform, lights, meshes, prim_ptrs);

	//}

	Scene Scene::buildTestSphere() {
		std::vector<std::shared_ptr<TriangleMesh>> meshes;
		std::vector<std::shared_ptr<Transform>> usedTransform;
		std::vector<std::shared_ptr<Light>> lights;
		std::vector<std::shared_ptr<Primitive>> prim_ptrs;

		std::shared_ptr<Transform> identity = std::make_shared<Transform>(Identity());
		usedTransform.push_back(identity);

		Point3f p0(-10000.0, -2.0, 10000.0);
		Point3f p1(10000.0, -2.0, 10000.0);
		Point3f p2(10000.0, -2.0, -10000.0);
		Point3f p3(-10000.0, -2.0, -10000.0);

		std::shared_ptr<TriangleMesh> plane = std::make_shared<TriangleMesh>(CreatePlane(identity.get(), identity.get(),
			p0, p1, p2, p3, Normal3f(0.0, 1.0, 0.0)));

		Point3f pl0(443.0, 848.7, 127.0);
		Point3f pl1(443.0, 848.7, 232.0);
		Point3f pl2(313.0, 848.7, 232.0);
		Point3f pl3(313.0, 848.7, 127.0);

		std::shared_ptr<TriangleMesh> light = std::make_shared<TriangleMesh>(CreatePlane(identity.get(), identity.get(),
			pl0, pl1, pl2, pl3, Normal3f(0.0, -1.0, 0.0)));



		meshes.push_back(plane);
		meshes.push_back(light);

		std::shared_ptr<Transform> sphereWorld = std::make_shared<Transform>(Translate(Vector3f(278, 200, 400)));
		std::shared_ptr<Transform> invSphereWorld = std::make_shared<Transform>(sphereWorld->inverse());

		usedTransform.push_back(sphereWorld);
		usedTransform.push_back(invSphereWorld);

		std::shared_ptr<Shape> sphere = Sphere::build(sphereWorld.get(), invSphereWorld.get(), 200);

		std::shared_ptr<MatteMaterial> kd = MatteMaterial::buildConst(0.0, RGBSpectrum::fromRGB(0.5));

		std::shared_ptr<TextureMapping2D> mapping = UVMapping2D::build(3, 3);
		std::shared_ptr<Texture<Spectrum>>whiteTexture =
			ConstTexture<Spectrum>::build(RGBSpectrum::fromRGB(0.80, 0.80, 0.080));
		std::shared_ptr<Texture<Spectrum>> blackTexture =
			ConstTexture<Spectrum>::build(RGBSpectrum::fromRGB(0.235294, 0.67451, 0.843137));
		std::shared_ptr<Texture<Spectrum>> cheTex = CheckeredTexture<Spectrum>::build(whiteTexture, blackTexture, mapping);
		std::shared_ptr<Texture<double>> sigma = ConstTexture<double>::build(0.0);


		std::shared_ptr<MatteMaterial> whiteLam = MatteMaterial::buildConst(0.0, RGBSpectrum::fromRGB(0.725f, 0.725f, 0.725f));
		std::shared_ptr<MatteMaterial> lightLam = MatteMaterial::buildConst(0.0, RGBSpectrum::fromRGB(0.65f));

		std::shared_ptr<MatteMaterial> checkered = MatteMaterial::build(sigma, cheTex);

		std::shared_ptr<Texture<Spectrum>> kdTex = ConstTexture<Spectrum>::build(RGBSpectrum::fromRGB(0.725f, 0.725f, 0.725f));
		std::shared_ptr<Texture<Spectrum>> ksTex = ConstTexture<Spectrum>::build(RGBSpectrum::fromRGB(0.7f, 0.7f, 0.7f));
		std::shared_ptr<Texture<double>> roughTex = ConstTexture<double>::build(0.2);

		std::shared_ptr<Material> plastic = Plastic::build(kdTex, ksTex, roughTex);

		//Light
		std::vector<std::shared_ptr<Triangle>> lightTris = light->getTriangles();

		Spectrum lightEmit = RGBSpectrum::fromRGB(47.0, 47.0, 47.0);
		std::shared_ptr<DiffuseAreaLight> aLight1 =
			std::make_shared<DiffuseAreaLight>(identity.get(), identity.get(), 5, lightTris[0].get(), lightEmit);
		std::shared_ptr<DiffuseAreaLight> aLight2 =
			std::make_shared<DiffuseAreaLight>(identity.get(), identity.get(), 5, lightTris[1].get(), lightEmit);

		lights.push_back(aLight1);
		lights.push_back(aLight2);

		std::shared_ptr<Primitive> lightp1 = std::make_shared<Primitive>(lightTris[0], lightLam, aLight1);
		std::shared_ptr<Primitive> lightp2 = std::make_shared<Primitive>(lightTris[1], lightLam, aLight2);

		std::shared_ptr<Primitive> p = Primitive::build(sphere, checkered, nullptr);
		std::vector<std::shared_ptr<Primitive>> ground = plane->generatePrimitive(whiteLam);

		prim_ptrs.push_back(lightp1);
		prim_ptrs.push_back(lightp2);
		prim_ptrs.push_back(p);
		prim_ptrs.insert(prim_ptrs.end(), ground.begin(), ground.end());
		return Scene(usedTransform, lights, meshes, prim_ptrs, AccelType::BVH);
	}

	Spectrum Scene::sampleLight(const SurfaceInteraction& record, double s,
		const Point2f& uv, LightSample* sample)const {
		Spectrum totalPower(0.0);
		const Light* light;
		for (int i = 0; i < lights.size(); i++) {
			totalPower += lights[i]->power();
		}
		Spectrum power(0.0);
		double p = 1.0;
		for (int i = 0; i < lights.size(); i++) {
			power += lights[i]->power();
			p = power.y() / totalPower.y();
			if (p >= s || i == lights.size() - 1) {
				light = lights[i].get();
				break;
			}
		}
		if (light)
			return light->sampleLi(record, uv, sample) / p;
		else
			return Spectrum(0.0);
	}

}