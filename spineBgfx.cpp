#include "spineBgfx.h"

void spine::BGFXTextureLoader::load(AtlasPage& page, const String& path)
{
	bgfx::TextureInfo ti;
	auto texColor = loadTexture(path.buffer(), BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, 0, &ti, nullptr);

	page.setRendererObject(&texColor);
	page.width = ti.width;
	page.height = ti.height;
}

void spine::BGFXTextureLoader::unload(void* texture)
{
	delete (bgfx::TextureHandle*)texture;
}

spine::SpineExtension* spine::getDefaultExtension() {
	return new spine::DefaultSpineExtension();
}

spine::SkeletonDrawable::SkeletonDrawable(SkeletonData* skeleton, AnimationStateData* stateData)
{
}

spine::SkeletonDrawable::~SkeletonDrawable()
{
}

void spine::SkeletonDrawable::update(float deltaTime)
{
}

void spine::SkeletonDrawable::draw() const
{
}