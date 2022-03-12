#include"mesh.h"
#include"../core/distribution.h"
namespace Raven {
	void TriangleMesh::buildTriangles() {
		std::vector<Primitive> prims;
		//surfaceArea = 0;
		for (int i = 0; i < nTriangles; i++) {
			//TODO:Store triangle ptr
			std::shared_ptr<Triangle> triangle = std::make_shared<Triangle>(localToWorld, worldToLocal, this, i);
			triMemory.push_back(triangle);
			//surfaceArea += triangle->area();
			Primitive p(triangle.get(), nullptr);
			//triangles.push_back(triangle);
			//PtrCollecter::getShape(triangle);
			prims.push_back(p);
		}
		triangles = new KdTreeAccel(prims, -1, 80, 1, 0.5, prims.size());
	}

	bool TriangleMesh::hit(const Ray& r_in, double tMin, double tMax)const {
		return triangles->hit(r_in, tMin, tMax);
	}

	bool TriangleMesh::intersect(const Ray& r_in, SurfaceInteraction& its, double tMin, double tMax)const {
		return triangles->intersect(r_in, its, tMin, tMax);
	}

	Bound3f TriangleMesh::localBound()const {
		return (*worldToLocal)(worldBound());
	}

	Bound3f TriangleMesh::worldBound()const {
		Bound3f box;
		for (size_t i = 0; i < vertices.size(); i++)
			box = Union(box, vertices[i]);
		return box;
	}

	double TriangleMesh::area()const {
		return surfaceArea;
	}

	SurfaceInteraction TriangleMesh::sample(const Point2f& uv)const {
		double allArea = 0;
		double p = GetRand() * surfaceArea;
		for (int i = 0; i < nTriangles; i++) {
			allArea += triangles->prims[i].getShape()->area();
			if (allArea >= p) {
				return triangles->prims[i].getShape()->sample(uv);
			}
		}
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

	bool Triangle::hit(const Ray& r_in, double tmin, double tMax)const {
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
		double t = invDet * Dot(s2, e2); if (t<0 || t>tMax) return false;
		double b1 = invDet * Dot(s1, s); if (b1 < 0 || b1>1) return false;
		double b2 = invDet * Dot(s2, r_in.dir); if (b2 < 0 || (b2 + b1)>1)return false;
		return true;
	}

	bool Triangle::intersect(const Ray& r_in, SurfaceInteraction& its, double tmin, double tmax)const {
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
		auto determinate = Dot(s1, e1); //
		if (determinate <= 0)return false;
		double invDet = 1.0 / Dot(s1, e1);
		double t = invDet * Dot(s2, e2);
		double b1 = invDet * Dot(s1, s);
		double b2 = invDet * Dot(s2, r_in.dir);
		double b0 = 1 - b1 - b2; if (b0 < 0)return false;
		//���߱��������򴫲�
		if (t <= 0)
			return false;
		//�������궼����0ʱ���������������ڣ��������������ཻ	
		if (b0 <= 0.0 || b1 <= 0.0 || b2 <= 0.0)
			return false;
		//TODO::����ཻʱ��
		if (t >= tmax)
			return false;

		//�������������ֵ������㼸�����ꡢ���������뷨��
		//Point2f uv[3];
		//getUVs(uv);
		const Point2f& uv0 = mesh->uvs[index(0)];
		const Point2f& uv1 = mesh->uvs[index(1)];
		const Point2f& uv2 = mesh->uvs[index(2)];


		const Normal3f& n0 = mesh->normals[index(0)];
		const Normal3f& n1 = mesh->normals[index(1)];
		const Normal3f& n2 = mesh->normals[index(2)];
		Point3f pHit = b0 * p0 + b1 * p1 + b2 * p2;
		Point2f uvHit = b0 * uv0 + b1 * uv1 + b2 * uv2;
		auto nHit = b0 * n0 + b1 * n1 + b2 * n2;
		////TODO:: ȷ�������γ���˳ʱ�룿��ʱ�룿����ȷ��������ķ�����ԭ����Ϊͬһ����
		//Vector3f nHit = Cross(p0 - p1, p2 - p1);
		////����dpdu��dpdv�Ա�����������������������Ϊ������ֵ
		Vector3f dpdu, dpdv;
		double du02 = uv0[0] - uv2[0], du12 = uv1[0] - uv2[0];
		double dv02 = uv0[1] - uv2[1], dv12 = uv1[1] - uv2[1];
		double det = du02 * dv12 - dv02 * du12;
		//�������ʽ����0���������һ�黥�ഹֱ��dpdu��dpdv
		if (det == 0) {
			genTBN((Vector3f)nHit, &dpdu, &dpdv);
		}
		else {
			auto invDet = 1 / det;
			Vector3f dp02 = p0 - p1;
			Vector3f dp12 = p1 - p2;
			dpdu = invDet * (dv12 * dp02 - dv02 * dp12);
			dpdv = invDet * (du02 * dp12 - du12 * dp02);
		}

		its.dpdu = dpdu;
		its.dpdv = dpdv;
		its.n = nHit;
		its.p = pHit;
		its.uv = uvHit;
		return true;
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
		Point3f p0 = mesh->vertices[index(0)];
		Point3f p1 = mesh->vertices[index(1)];
		Point3f p2 = mesh->vertices[index(2)];
		box.pMin = Point3f(Min(p0.x, p1.x, p2.x), Min(p0.y, p1.y, p2.y), Min(p0.z, p1.z, p2.z));
		box.pMax = Point3f(Max(p0.x, p1.x, p2.x), Max(p0.y, p1.y, p2.y), Max(p0.z, p1.z, p2.z));
		return box;
	}

	double Triangle::area()const {
		return 0.f;
	}

	SurfaceInteraction Triangle::sample(const Point2f& uv)const {
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
		return sisec;
	}

}