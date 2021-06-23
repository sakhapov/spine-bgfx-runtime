#include "spineBgfx.h"

void spine::BGFXTextureLoader::load(AtlasPage& page, const String& path)
{
	bgfx::TextureHandle texColor = loadTexture("fieldstone-n.dds");
}

void spine::BGFXTextureLoader::unload(void* texture)
{

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
