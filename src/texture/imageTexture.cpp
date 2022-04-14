//#include"imageTexture.h"
//#include"../utils/loader.h"
//
//namespace Raven {
//	template<class T>
//	std::map <TextureInfo, std::unique_ptr<Mipmap<T>>> ImageTexture<T>::textureMap;
//
//
//	//��ȡһ��������ȡ��Mipmap
//	template<class T>
//	Mipmap<T>* ImageTexture<T>::GetTexture(
//		const std::string& path,
//		bool doTrilinear,
//		ImageWrap wrap,
//		double scale,
//		bool gamma) {
//
//		//���ϵͳ�Ѽ��ظ�ͼ�񣬷��ظ�ͼ���Mipmap
//		TextureInfo info(path, doTrilinear, wrap, scale, gamma);
//		if (textureMap.find(info) != textureMap.end())
//			return textureMap[info].get();
//
//		Mipmap<T>* mipmap;
//		Point2f resolution;
//		std::unique_ptr<Spectrum[]> texels = ReadImageSpectrum(path, &resolution);//��ȡͼ����Ϣ
//		if (texels) {
//			//�ҵ���ͼ�����·���ռ䲢�Զ����ͼ�����gamma�任
//			std::unique_ptr<T[]> convertedTexels = std::make_unique<T[]>(resolution.x * resolution.y);
//			for (int i = 0; i < resolution.x * resolution.y; i++) {
//				convertedTexels[i] = scaleAndGamma(texels[i], scale, gamma);
//			}
//
//			mipmap = new Mipmap<T>(resolution, convertedTexels.get(), doTrilinear, wrap);
//		}
//		else {
//			//δ�ҵ�ͼ������һ��ֻ��һ�����ص�Mipmap
//			T v = 1.0;
//			mipmap = new Mipmap(pointi2(1, 1), &v, doTrilinear, wrap);
//		}
//
//		textureMap[info] = std::make_unique<Mipmap<T>>(mipmap);//insert mipmap into textureMap
//
//		return mipmap;
//	}
//
//	template<class T>
//	T ImageTexture<T>::evaluate(const SurfaceInteraction& its)const {
//		auto [st, dstdx, dstdy] = mapping->map(iter);
//		T result = map->lookup(st, dstdx,dstdy);
//		return result;
//	}
//
//	template<class T>
//	Spectrum ImageTexture<T>::scaleAndGamma(const Spectrum& original, double scale, bool gamma) {
//		Spectrum sReturn;
//		for (int i = 0; i < RGBSpectrum::sampleNumber; i++) {
//			//if original texel value is gamma corrected, reconstruct a linear function
//			sReturn[i] = scale * (gamma ? InverseGammaCorrect(original[i]) : original[i]);
//		}
//		return sReturn;
//	}
//
//	template<class T>
//	Spectrum ImageTexture<T>::InvScaleAndGamma(const Spectrum& original, bool gamma) {
//		Spectrum sReturn;
//		for (int i = 0; i < RGBSpectrum::sampleNumber; i++) {
//			sReturn[i] = (gamma ? GammaCorrect(original[i]) : original[i]);
//		}
//		return sReturn;
//	}
//}