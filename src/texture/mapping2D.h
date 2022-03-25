#ifndef _RAVEN_TEXTURE_MAPPING_2D_H_
#define _RAVEN_TEXTURE_MAPPING_2D_H_

#include"../core/texture.h"
#include"../core/base.h"


namespace Raven {

	/// <summary>
	/// UVӳ�䣬ͨ����UV������м򵥵ķ���任�õ���������ST
	/// </summary>
	class UVMapping2D :public TextureMapping2D {
	public:
		UVMapping2D(double scaleU = 1.0, double scaleV = 1.0, double uOffset = 0.0, double vOffset = 0.0) {}

		virtual std::tuple<Point2f, Vector2f, Vector2f> map(const SurfaceInteraction& si)const;
	private:
		double su, sv;//UV���������
		double du, dv;//UV���굽ƫ��
	};

	/// <summary>
	/// SphericalMapping,ͨ��������Χ�򣬽���ӳ�䵽������������������ST
	/// </summary>
	class SphericalMapping2D :public TextureMapping2D {

	private:
		const std::shared_ptr<Transform> worldToTexture;
	};
}
#endif