#include"mipmap.h"
#include<omp.h>

namespace Raven {
	template<class T>
	Mipmap<T>::Mipmap(
		const Image<T>& imageData,
		bool trilinear,
		ImageWrap wrap) :
		doTrilinear(trilinear), resolution(imageData.uSize(), imageData.vSize()), wrapMode(wrap) {
		Image<T> resampledImage(resolution.x, resolution.y);
		Point2i resampledRes = resolution;

		//���ͼƬ�ķֱ��ʲ���2��ָ��������Ŵ�2��ָ����
		if (!IsPowerOf2(resampledRes.x) || !IsPowerOf2(resampledRes.y)) {
			resampledRes = Point2i(RoundUpPower2(resampledRes.x), RoundUpPower2(resampledRes.y));
			resampledImage.resize(resampledRes.x, resampledRes.y);

			//��x��������ͼ��
			Image<T> xResampled(resampledRes.x, resolution.y);
			std::vector<ResampleWeight> xWeights = resampleWeights(resolution.x, resampledRes.x);
#pragma omp parallel for
			for (int t = 0; t < resolution.y; t++) {
				for (int s = 0; s < resampledRes.x; s++) {

					for (int i = 0; i < 4; i++) {

						double w = xWeights[s].weight[i];

						int32_t xOffset = xWeights[s].firstTexel + i;
						if (wrapMode == ImRepeat)
							xOffset = (xOffset + resolution.x) % resolution.x;
						if (wrapMode == ImClamp)
							xOffset = Clamp(xOffset, 0, resolution.x - 1);


						if (xOffset >= 0 && xOffset < resolution.x)
							xResampled(s, t) += imageData(xOffset, t) * w;
					}

				}
			}

			//��y��������ͼ��
			std::vector<ResampleWeight> yWeights = resampleWeights(resolution.y, resampledRes.y);
#pragma omp parallel for
			for (int s = 0; s < resampledRes.x; s++) {
				for (int t = 0; t < resampledRes.y; t++) {

					for (int i = 0; i < 4; i++) {

						double w = yWeights[t].weight[i];

						int32_t yOffset = yWeights[t].firstTexel + i;
						if (wrapMode == ImageWrap::ImRepeat)
							yOffset = (yOffset + resolution.y) % resolution.y;
						if (wrapMode == ImageWrap::ImClamp)
							yOffset = Clamp(yOffset, 0, resolution.y - 1);

						if (yOffset >= 0 && yOffset < resolution.y)
							resampledImage(s, t) += xResampled(s, yOffset) * w;
					}
				}
			}
		}
		else {
			resampledImage = imageData;
		}

		//��ʼ��ͼ�������
		maxLevel = 1 + static_cast<int>(Log2(Max(resampledRes.x, resampledRes.y)));
		pyramid.resize(maxLevel);

		//�������ײ�Ϊ������ԭͼ��
		pyramid[0].reset(new Image<T>(resampledRes.x, resampledRes.y, &resampledImage[0]));


		int sRes = resampledRes.x;
		int tRes = resampledRes.y;

		//���ɵ�i��ͼ��
		for (int i = 1; i < maxLevel; i++) {
			//��i��,�ֱ���Ϊ��һ���һ��
			sRes = Max(1, sRes / 2);
			tRes = Max(1, tRes / 2);
			pyramid[i].reset(new Image<T>(sRes, tRes));

			//��box filter������һ������صõ��ò����ص�ֵ
#pragma omp parallel for
			for (int t = 0; t < tRes; t++) {
				for (int s = 0; s < sRes; s++) {
					(*pyramid[i])(s, t) = 0.25 * (texel(i - 1, s * 2, t * 2) + texel(i - 1, s * 2 + 1, t * 2)
						+ texel(i - 1, s * 2, t * 2 + 1) + texel(i - 1, s * 2 + 1, t * 2 + 1));
				}
			}
		}
	}


	//compute weights of original texels to resampled texels
	template<class T>
	std::vector<ResampleWeight> Mipmap<T>::resampleWeights(int32_t oldRes, int32_t newRes) {
		std::vector<ResampleWeight> weights = std::vector<ResampleWeight>(newRes);
		double filterWidth = 2.;
		for (int i = 0; i < newRes; i++) {
			//compute continuous texture coordinates of resampled texel relative to original sample coordinates
			double coord = (i + 0.5) * oldRes / newRes;

			//compute index of first texel contributes to new texel
			weights[i].firstTexel = std::floor(coord - filterWidth + 0.5);

			//compute weights of 4 adjcent texels
			for (int j = 0; j < 4; j++) {
				double pos = weights[i].firstTexel + j + 0.5;//convert discret coordinate to continuous coords 
				weights[i].weight[j] = lanczosSinc(abs(coord - pos));//compute weight using sinc function
			}

			//normalize weights so that they sum to 1
			double invSum = 1 / (weights[i].weight[0] + weights[i].weight[1]
				+ weights[i].weight[2] + weights[i].weight[3]);
			for (int j = 0; j < 4; j++)
				weights[i].weight[j] *= invSum;
		}
		return weights;
	}

	template<class T>
	T Mipmap<T>::lookup(const Point2f& st, double width)const {
		//����filter���ȼ���һ����������ʾ�Ĳ���
		double level = pyramid.size() - 1 + Log2(Max(width, 1e-8));

		if (level < 0)
			return triangle(0, st);//������ԭͼ��st���������µ�ֵ

		else if (level > maxLevel - 1)
			return texel(maxLevel - 1, 0, 0);//�����Mipmapֻ��һ���̶�ֵ

		else {
			//triangle filter both image level beside and linear interpolate two filtered value
			//���ڽ����������������˲����st���������µ�ֵ����ֵ
			int levelFlr = (int)std::floor(level);
			double delta = level - levelFlr;
			T vLastLevel = triangle(levelFlr, st);
			T vNextLevel = triangle(levelFlr + 1, st);
			return Lerp(delta, vLastLevel, vNextLevel);
		}
	}

	template<class T>
	T Mipmap<T>::lookup(const Point2f& st, const Vector2f& dstdx, const Vector2f& dstdy)const {

		//double texelX = pyramid[0]->uSize() * st[0];
		//double texelY = pyramid[1]->vSize() * st[1];
		//return (*pyramid[0])(texelX, texelY);
		double filterWidth = Max(Max(abs(dstdx.x), abs(dstdx.y)),
			Max(abs(dstdy.x), abs(dstdy.y)));
		return lookup(st, filterWidth);
	}

	// ˫���Բ�ֵ
	template<class T>
	T Mipmap<T>::triangle(int level, const Point2f& st)const {
		level = Clamp(level, 0, maxLevel - 1);
		//compute continous coordinate - 0.5 of sample point in order to compute distance
		float s = st[0] * pyramid[level]->uSize() - 0.5f;
		float t = st[1] * pyramid[level]->vSize() - 0.5f;

		int is = (int)std::floor(s);
		int it = (int)std::floor(t);
		//perform bilinear interpolate between four adjacent texels
		float ds = s - is;
		float dt = t - it;
		T sLerped0 = Lerp(ds, texel(level, is, it), texel(level, is + 1, it));
		T sLerped1 = Lerp(ds, texel(level, is, it + 1), texel(level, is + 1, it + 1));
		return Lerp(dt, sLerped0, sLerped1);
	}

	template<class T>
	const T& Mipmap<T>::texel(int level, int s, int t)const {
		//get image data of input level
		const Image<T>& l = *pyramid[level];
		//bound st coordinates according to image wrap mode
		switch (wrapMode) {
		case ImRepeat:
			s = (s + l.uSize()) % l.uSize();
			t = (s + l.vSize()) % l.vSize();
			break;
		case ImClamp:
			s = Clamp(s, 0, l.uSize() - 1);
			t = Clamp(t, 0, l.vSize() - 1);
			break;
		case ImBlack:
			if (s<0 || s>l.uSize() - 1)return T(0.);
			if (t<0 || s>l.vSize() - 1)return T(0.);
			break;
		}
		return l(s, t);

	}

	template<class T>
	double Mipmap<T>::lanczosSinc(double v, double tau)const {
		v = v / tau;
		return sin(M_PI * v) / (M_PI * v);
	}


	template class Mipmap<RGBSpectrum>;
	template class Mipmap<double>;
}