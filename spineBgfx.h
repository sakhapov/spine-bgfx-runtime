#ifndef SPINE_BGFX_H_
#define SPINE_BGFX_H_

#include <spine/spine.h>
#include <spine/Extension.h>
#include <spine/Debug.h>

#include "bx/bx.h"
#include "bx/readerwriter.h"
#include "bgfx/bgfx.h"
#include "bimg/bimg.h"
#include "bgfx_utils.h"
#include "entry.h"
#include "bgfx/platform.h"

namespace spine 
{
	class SkeletonDrawable
	{
	public:
		Skeleton* skeleton;
		AnimationState* state;
		float timeScale;
		//sf::VertexArray* vertexArray;
		VertexEffect* vertexEffect;

		SkeletonDrawable(SkeletonData* skeletonData, AnimationStateData* stateData = 0);

		~SkeletonDrawable();

		void update(float deltaTime);

		virtual void draw() const;

		void setUsePremultipliedAlpha(bool usePMA) { usePremultipliedAlpha = usePMA; };

		bool getUsePremultipliedAlpha() { return usePremultipliedAlpha; };
	private:
		mutable bool ownsAnimationStateData;
		mutable Vector<float> worldVertices;
		mutable Vector<float> tempUvs;
		mutable Vector<Color> tempColors;
		mutable Vector<unsigned short> quadIndices;
		mutable SkeletonClipping clipper;
		mutable bool usePremultipliedAlpha;
	};

	class BGFXTextureLoader : public TextureLoader {
	public:
		virtual void load(AtlasPage& page, const String& path);
		virtual void unload(void* texture);
	};
}
#endif /* SPINE_BGFX_H_ */
