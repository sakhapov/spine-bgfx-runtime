#ifndef SPINE_BGFX_H_
#define SPINE_BGFX_H_

#include <vector>

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

namespace extension {
	struct VertexDecl
	{
		uint8_t x;
		uint8_t y;
		uint8_t z;
		int16_t u;
		int16_t v;
		spine::Color color;

		static void init()
		{
			ms_layout
				.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Uint8)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Int16, true, true)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Float, true, true)
				.end();
		}

		static bgfx::VertexLayout ms_layout;
	};

	bgfx::VertexLayout VertexDecl::ms_layout;
}

namespace spine 
{

	class SkeletonDrawable
	{
	public:
		typedef struct Texture {
			bgfx::TextureHandle textureHndl;
			bgfx::UniformHandle s_texColor;
			int width;
			int height;
		} Texture;
		
		bgfx::ProgramHandle shaderProg;
		Skeleton* skeleton;
		AnimationState* state;
		float timeScale;
		std::vector<extension::VertexDecl>* vertexArray;
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

	class BGFXTextureLoader : public TextureLoader 
	{
	public:
		virtual void load(AtlasPage& page, const String& path);
		virtual void unload(void* texture);
	};
}
#endif /* SPINE_BGFX_H_ */
