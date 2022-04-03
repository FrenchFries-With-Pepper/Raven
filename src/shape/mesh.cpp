#include"mesh.h"
#include"../core/distribution.h"
#include"../core/primitive.h"
namespace Raven {
	std::vector<std::shared_ptr<Triangle>> TriangleMesh::getTriangles() {
		std::vector<std::shared_ptr<Triangle>> triangles;
		for (size_t i = 0; i < nTriangles; i++) {
			std::shared_ptr<Triangle> t = std::make_shared<Triangle>(OTW, WTO, this, i);
			triangles.push_back(t);
		}
		return triangles;
	}

	std::vector<std::shared_ptr<Primitive>> TriangleMesh::generatePrimitive(const std::shared_ptr<Material>& mate,
		const std::shared_ptr<Light>& light) {
		std::vector<std::shared_ptr<Primitive>> primitives;
		for (size_t i = 0; i < nTriangles; i++) {
			std::shared_ptr<Triangle> t = std::make_shared<Triangle>(OTW, WTO, this, i);
			std::shared_ptr<Primitive> p = std::make_shared<Primitive>(t, mate, light);
			primitives.push_back(p);
		}
		return primitives;
	}

	void Triangle::getUVs(Point2f uv[3])const {
		if (mesh->hasUV) {
			//if triangle
			uv[0] = mesh->uvs[index(0)];
			uv[1] = mesh->uvs[index(1)];
			uv[2] = mesh->uvs[index(2)];
		}
		else {
			uv[0] = Point2f(0, 0);
			uv[1] = Point2f(0, 1);
			uv[2] = Point2f(1, 1);
		}
	}

	bool Triangle::hit(const Ray& r_in, double tMax)const {
		//ȡ��������ȡ�������ε���������p0,p1,p2
		const Point3f& p0 = mesh->vertices[index(0)];
		const Point3f& p1 = mesh->vertices[index(1)];
		const Point3f& p2 = mesh->vertices[index(2)];
		//���ÿ���ķ������������͵�������������ߵĴ���ʱ��t
		Vector3f e1 = p1 - p0;
		Vector3f e2 = p2 - p0;
		Vector3f s = r_in.origin - p0;
		Vector3f s1 = Cross(r_in.dir, e2);
		Vector3f s2 = Cross(s, e1);
		auto det = Dot(s1, e1); //����ʽ���벻Ϊ�������������ֵ�������0
		if (det <= 0)return false;
		double invDet = 1.0 / det;
		double t = invDet * Dot(s2, e2); if (t < 0 || t >= tMax) return false;
		double b1 = invDet * Dot(s1, s); if (b1 < 0 || b1>1) return false;
		double b2 = invDet * Dot(s2, r_in.dir); if (b2 < 0 || (b2 + b1)>1)return false;
		double b0 = 1 - b1 - b2;
		const Normal3f& n0 = mesh->normals[index(0)];
		const Normal3f& n1 = mesh->normals[index(1)];
		const Normal3f& n2 = mesh->normals[index(2)];
		Normal3f nHit = n0 * b0 + n1 * b1 + n2 * b2;
		if (Dot(nHit, r_in.dir) > 0)
			return false;
		return true;
	}

	std::optional<SurfaceInteraction> Triangle::intersect(const Ray& r_in, double tMax)const {
		//ȡ��������ȡ�������ε���������p0,p1,p2
		const Point3f& p0 = mesh->vertices[index(0)];
		const Point3f& p1 = mesh->vertices[index(1)];
		const Point3f& p2 = mesh->vertices[index(2)];
		//���ÿ���ķ������������͵�������������ߵĴ���ʱ��t
		Vector3f e1 = p1 - p0;
		Vector3f e2 = p2 - p0;
		Vector3f s = r_in.origin - p0;
		Vector3f s1 = Cross(r_in.dir, e2);
		Vector3f s2 = Cross(s, e1);
		auto determinate = Dot(s1, e1);

		//����ʽΪ�㣬�޽�
		if (determinate <= 0)
			return std::nullopt;

		double invDet = 1.0 / determinate;
		double t = invDet * Dot(s2, e2);
		double b1 = invDet * Dot(s1, s);
		double b2 = invDet * Dot(s2, r_in.dir);
		double b0 = 1 - b1 - b2; 

		//���㲻����������
		if (b0 < 0)
			return std::nullopt;

		//���߱��������򴫲�
		if (t <= 0)
			return std::nullopt;

		//�������궼����0ʱ���������������ڣ��������������ཻ	
		if (b0 <= 0.0 || b1 <= 0.0 || b2 <= 0.0)
			return std::nullopt;

		//TODO::����ཻʱ��
		if (t >= tMax)
			return std::nullopt;

		//�������������ֵ������㼸�����ꡢ���������뷨��
		Point2f uv[3];
		getUVs(uv);

		const Point2f& uv0 = uv[0];
		const Point2f& uv1 = uv[1];
		const Point2f& uv2 = uv[2];

		const Normal3f& n0 = mesh->normals[index(0)];
		const Normal3f& n1 = mesh->normals[index(1)];
		const Normal3f& n2 = mesh->normals[index(2)];
		Point3f pHit = b0 * p0 + b1 * p1 + b2 * p2;
		Point3f pb1 = b0 * p0;
		Point3f pb2 = b1 * p1;
		Point3f pb3 = b2 * p2;
		Point2f uvHit = b0 * uv0 + b1 * uv1 + b2 * uv2;
		auto nHit = b0 * n0 + b1 * n1 + b2 * n2;

		//ֻ�������������
		if (Dot(nHit, r_in.dir) > 0)
			return std::nullopt;

		////����dpdu��dpdv�Ա�����������������������Ϊ������ֵ
		Vector3f dpdu, dpdv;
		double du02 = uv0[0] - uv2[0], du12 = uv1[0] - uv2[0];
		double dv02 = uv0[1] - uv2[1], dv12 = uv1[1] - uv2[1];
		double det = du02 * dv12 - dv02 * du12;

		//�������ʽ����0���������һ�黥�ഹֱ��dpdu��dpdv
		if (det == 0) {
			auto [dpu, dpv] = genTBN((Vector3f)nHit);
			dpdu = dpu;
			dpdv = dpv;
		}
		else {
			auto invDet = 1 / det;
			Vector3f dp02 = p0 - p1;
			Vector3f dp12 = p1 - p2;
			dpdu = invDet * (dv12 * dp02 - dv02 * dp12);
			dpdv = invDet * (du02 * dp12 - du12 * dp02);
		}

		SurfaceInteraction its;
		its.dpdu = dpdu;
		its.dpdv = dpdv;
		its.n = nHit;
		its.p = pHit;
		its.uv = uvHit;
		its.t = t;
		its.wo = -r_in.dir;
		return its;
	}

	Bound3f Triangle::localBound()const {
		Bound3f box;
		Point3f p0 = mesh->vertices[index(0)];
		Point3f p1 = mesh->vertices[index(1)];
		Point3f p2 = mesh->vertices[index(2)];
		p0 = (*worldToLocal)(p0);
		p1 = (*worldToLocal)(p1);
		p2 = (*worldToLocal)(p2);
		box.pMin = Point3f(Min(p0.x, p1.x, p2.x), Min(p0.y, p1.y, p2.y), Min(p0.z, p1.z, p2.z));
		box.pMax = Point3f(Max(p0.x, p1.x, p2.x), Max(p0.y, p1.y, p2.y), Max(p0.z, p1.z, p2.z));
		return box;
	}

	Bound3f Triangle::worldBound()const {
		Bound3f box;
		const Point3f& p0 = mesh->vertices[index(0)];
		const Point3f& p1 = mesh->vertices[index(1)];
		const Point3f& p2 = mesh->vertices[index(2)];
		box.pMin = Point3f(Min(p0.x, p1.x, p2.x), Min(p0.y, p1.y, p2.y), Min(p0.z, p1.z, p2.z));
		box.pMax = Point3f(Max(p0.x, p1.x, p2.x), Max(p0.y, p1.y, p2.y), Max(p0.z, p1.z, p2.z));
		return box;
	}

	double Triangle::area()const {
		const Point3f& p0 = mesh->vertices[index(0)];
		const Point3f& p1 = mesh->vertices[index(1)];
		const Point3f& p2 = mesh->vertices[index(2)];
		auto e1 = p0 - p2;
		auto e2 = p1 - p2;
		return 0.5 * Cross(e1, e2).length();
	}

	std::tuple<SurfaceInteraction, double> Triangle::sample(const Point2f& uv)const {
		//���uvֵ��Ӧ����������
		Point2f b = UniformSampleTriangle(uv);

		const Point3f& p0 = mesh->vertices[index(0)];
		const Point3f& p1 = mesh->vertices[index(1)];
		const Point3f& p2 = mesh->vertices[index(2)];

		const Normal3f& n0 = mesh->normals[index(0)];
		const Normal3f& n1 = mesh->normals[index(1)];
		const Normal3f& n2 = mesh->normals[index(2)];

		//interpolate vertices by barycentric coordinate
		Point3f sample = p0 * b[0] + p1 * b[1] + p2 * (1 - b[0] - b[1]);
		Normal3f normal = n0 * b[0] + n1 * b[1] + n2 * (1 - b[0] - b[1]);

		SurfaceInteraction sisec;
		sisec.n = normal;
		sisec.p = sample;
		double pdf = 1 / area();
		return std::tuple<SurfaceInteraction, double>(sisec, pdf);
	}

	TriangleMesh CreatePlane(const Transform* LTW, const Transform* WTL, const Point3f& v0,
		const Point3f& v1, const Point3f& v2, const Point3f& v3, const Normal3f& normal) {
		std::vector<Point3f> vertices = { v0,v1,v2,v3 };
		std::vector<int> indices = { 0,1,3,1,2,3 };
		std::vector<Point2f> uvs = { Point2f(0,1),Point2f(1,1),Point2f(1,0),Point2f(0,0) };
		std::vector<Normal3f> normals = { normal,normal,normal, normal };
		std::vector<Vector3f> tangants;
		TriangleMesh mesh(LTW, WTL, 2, vertices, indices, normals, tangants, uvs, AccelType::List);
		return mesh;
	}


	std::shared_ptr<TriangleMesh> TriangleMesh::build(const Transform* LTW, const Transform* WTL,
		const TriangleInfo& info, AccelType buildType) {
		return std::make_shared<TriangleMesh>(LTW, WTL, info.numbers, info.vertices,
			info.indices, info.normals, info.tangants, info.uvs, buildType);
	}

}