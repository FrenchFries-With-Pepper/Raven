#include"bvh.h"
#include<optional>
#include<vector>

namespace Raven {

	BVHAccel::BVHAccel(const std::vector<std::shared_ptr<Primitive>>& primitives, size_t maxSize = 1) :
		Accelerate(primitives), maxPrimInNode(maxSize) {

		//��ʼ��primitiveInfo����
		std::vector<PrimitiveInfo> primInfo(prims.size());
		for (size_t i = 0; i < primInfo.size(); i++) {
			primInfo[i] = PrimitiveInfo(prims[i]->worldBounds(), i);
		}

		std::vector<std::shared_ptr<Primitive>> ordered;

		//�ݹ鹹��BVH��
		root = recursiveBuild(primInfo, 0, primInfo.size(), ordered);

		//���ź����prim�����滻ԭ����
		prims.swap(ordered);
	}

	std::shared_ptr<BVHNode> BVHAccel::recursiveBuild(std::vector<PrimitiveInfo>& info, size_t start, size_t end,
		std::vector<std::shared_ptr<Primitive>>& ordered) {
		std::shared_ptr<BVHNode> currentNode = std::make_shared<BVHNode>();

		//��ǰnode��boundingBox
		Bound3f centroidBound;
		for (size_t i = start; i < end; i++)
			centroidBound = Union(centroidBound, info[i].box);

		//���primitive��Ŀ������ֵ������Ҷ�ӽڵ�
		int nPrimitive = end - start;
		if (nPrimitive == 1) {

			size_t firstOffset = ordered.size();

			for (int i = 0; i < nPrimitive; i++) {
				size_t primNum = info[start + i].primitiveIndex;
				ordered.push_back(prims[primNum]);
			}

			currentNode->buildLeaf(centroidBound, firstOffset, nPrimitive);
			std::cout << "Leaf node generated, primitive count = " << nPrimitive << std::endl;
			return currentNode;
		}
		else if (nPrimitive == 2) {
			size_t middle = start + 1;
			std::shared_ptr<BVHNode> leftNode = recursiveBuild(info, start, middle, ordered);
			std::shared_ptr<BVHNode> rightNode = recursiveBuild(info, middle, end, ordered);
			currentNode->buildInterior(leftNode, rightNode);
			return currentNode;
		}
		//ѡ�񻮷���
		int axis = centroidBound.maxExtent();
		bool split = false;
		size_t middle;
		for (int i = 0; i < 3; i++) {
			//����
			switch (axis) {
			case 0:
				std::sort(&info[start], &info[end - 1],
					[](const PrimitiveInfo& p0, const PrimitiveInfo& p1)->bool {
						return p0.centroid.x < p1.centroid.x;
					});
				break;
			case 1:
				std::sort(&info[start], &info[end - 1],
					[](const PrimitiveInfo& p0, const PrimitiveInfo& p1)->bool {
						return p0.centroid.y < p1.centroid.y;
					});
				break;
			case 2:
				std::sort(&info[start], &info[end - 1],
					[](const PrimitiveInfo& p0, const PrimitiveInfo& p1)->bool {
						return p0.centroid.z < p1.centroid.z;
					});
				break;
			}

			//SAH
			const int nBuckets = 16;
			SAHBucket buckets[nBuckets];

			//����ÿ��bucket��primitve����Ŀ��boundingbox�Ĵ�С
			for (size_t i = start; i < end; i++) {
				Bound3f box = info[i].box;
				int b = nBuckets * centroidBound.offset(info[i].centroid)[axis];
				if (b == nBuckets)b--;
				buckets[b].nPrimitives++;
				buckets[b].box = Union(buckets[b].box, box);
			}

			//������ÿ��bucket���ֵĿ���
			double cost[nBuckets - 1];
			for (int i = 0; i < nBuckets - 1; i++) {
				//����ӵ�i��bucket�л��֣����ߵĿ���

				Bound3f left, right;
				int leftCount = 0, rightCount = 0;

				//���ֺ����
				for (int j = 0; j <= i; j++) {
					left = Union(left, buckets[j].box);
					leftCount += buckets[j].nPrimitives;
				}

				//���ֺ��ұ�
				for (int j = i + i; j < nBuckets; j++) {
					right = Union(right, buckets[j].box);
					rightCount += buckets[j].nPrimitives;
				}
				cost[i] = 0.125f + (Max(0.0, left.surfaceArea()) * leftCount
					+ Max(0.0, right.surfaceArea()) * rightCount)
					/ centroidBound.surfaceArea();
			}

			//Ѱ����С�����Ļ���
			double minCost = std::numeric_limits<double>::max();
			int minBucket = 0;
			for (int i = 0; i < nBuckets - 1; i++) {
				if (buckets[i].nPrimitives == 0)
					continue;
				if (cost[i] < minCost) {
					minCost = cost[i];
					minBucket = i;
				}
			}

			//ͳ�ƻ��ֺ�����ж���primitive
			size_t leftPrimitives = 0;
			for (int i = 0; i <= minBucket; i++)
				leftPrimitives += buckets[i].nPrimitives;
			assert(leftPrimitives + start <= end);
			assert(leftPrimitives >= 0);

			//����Ҫ���л���(��С����δ����������)
			if (leftPrimitives != 0 && leftPrimitives + start < end) {
				middle = start + leftPrimitives;
				split = true;
				break;
			}

			//���Ÿ�������δ�ҵ����ʵĻ��֣�������һ��������
			axis = (axis + 1) % 3;

		}

		//�ҵ��˿������͵Ļ���
		if (split) {
			//ִ�л��ֲ��ݹ����ӽڵ�
			std::shared_ptr<BVHNode> leftNode = recursiveBuild(info, start, middle, ordered);
			std::shared_ptr<BVHNode> rightNode = recursiveBuild(info, middle, end, ordered);
			currentNode->buildInterior(leftNode, rightNode);
			return currentNode;
		}
		//δ�ҵ��������͵Ļ���
		else {
			//����Ҷ�ӽڵ�
			size_t firstOffset = ordered.size();

			for (int i = 0; i < nPrimitive; i++) {
				size_t primNum = info[i + start].primitiveIndex;
				ordered.push_back(prims[primNum]);
			}

			currentNode->buildLeaf(centroidBound, firstOffset, nPrimitive);
			std::cout << "Leaf node generated, primitive count = " << nPrimitive << std::endl;
			return currentNode;
		}
	}

	bool BVHAccel::hit(const Ray& r_in, double tMax)const {
		const int maxSize = 64;
		int size = 1;
		std::shared_ptr<BVHNode> nodes[maxSize];
		nodes[0] = root;
		int head = -1;
		int rear = 0;
		//��������ֱ������������node���߹�����primitive�ཻ
		while (head != rear) {
			head = (head + 1) % 64;//������һ��Node
			size--;
			double t0, t1;//������boundingbox�ཻ�ľ������

			//���Ե�ǰ�ڵ��Ƿ�������ཻ
			if (nodes[head]->box.hit(r_in, &t0, &t1)) {

				//������ýڵ�İ�Χ���ཻ
				const std::shared_ptr<BVHNode>& node = nodes[head];
				if (node->children[0] != nullptr || node->children[1] != nullptr) {
					//�ýڵ�Ϊ�м�ڵ�,���ӽڵ�������
					rear = (rear + 1) % 64;
					size++;
					assert(size < maxSize);
					nodes[rear] = node->children[0];
					rear = (rear + 1) % 64;
					size++;
					assert(size < maxSize);
					nodes[rear] = node->children[1];
				}
				else {
					//�ýڵ�ΪҶ�ӽڵ�,�����ӽڵ��е�primitve
					for (size_t i = 0; i < node->nPrims; i++) {
						size_t index = node->firstPrimOffset + i;
						if (prims[index]->hit(r_in, tMax))
							return true;
					}
				}
			}
		}

		return false;
	}

	std::optional<SurfaceInteraction> BVHAccel::intersect(const Ray& ray, double tMax)const {
		const int maxSize = 64;
		int size = 1;
		std::shared_ptr<BVHNode> nodes[maxSize];
		nodes[0] = root;
		int head = -1;
		int rear = 0;
		double closet = tMax;
		std::optional<SurfaceInteraction> record;
	
		//��������ֱ������������node
		while (head != rear) {
			head = (head + 1) % 64;
			size--;
			//���Ե�ǰ�ڵ��Ƿ�������ཻ
			double t0, t1;
			if (nodes[head]->box.hit(ray, &t0, &t1)) {
				//��ǰ�ڵ�İ�Χ��������ཻ
	
				const std::shared_ptr<BVHNode>& node = nodes[head];
				if (node->children[0] != nullptr || node->children[1] != nullptr) {
					//�ýڵ�Ϊ�м�ڵ�
					rear = (rear + 1) % 64;
					size++;
					assert(size < maxSize);
					nodes[rear] = node->children[0];
					rear = (rear + 1) % 64;
					size++;
					assert(size < maxSize);
					nodes[rear] = node->children[1];
				}
				else {
					//�ýڵ�ΪҶ�ӽڵ�
					for (size_t i = 0; i < node->nPrims; i++) {
						//�жϽڵ��е�ÿһ��primitive�Ƿ�������ཻ
						size_t index = node->firstPrimOffset + i;
						std::optional<SurfaceInteraction> hitRecord = prims[index]->intersect(ray, closet);
						if (hitRecord.has_value()) {
							//��primitive������ཻ
							closet = hitRecord->t;
							record = hitRecord;
						}
					}
				}
			}
		}
	
		return record;
	}
}