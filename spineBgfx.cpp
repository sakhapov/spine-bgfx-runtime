#include "spineBgfx.h"

#define SPINE_MESH_VERTEX_COUNT_MAX 1000

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

spine::SkeletonDrawable::SkeletonDrawable(SkeletonData* skeletonData, AnimationStateData* stateData)
{
	Bone::setYDown(true);
	worldVertices.ensureCapacity(SPINE_MESH_VERTEX_COUNT_MAX);
	skeleton = new(__FILE__, __LINE__) Skeleton(skeletonData);

	tempUvs.ensureCapacity(16);
	tempColors.ensureCapacity(16);

	ownsAnimationStateData = stateData == 0;
	if (ownsAnimationStateData) stateData = new(__FILE__, __LINE__) AnimationStateData(skeletonData);

	state = new(__FILE__, __LINE__) AnimationState(stateData);

	quadIndices.add(0);
	quadIndices.add(1);
	quadIndices.add(2);
	quadIndices.add(2);
	quadIndices.add(3);
	quadIndices.add(0);
}

spine::SkeletonDrawable::~SkeletonDrawable()
{
}

void spine::SkeletonDrawable::update(float deltaTime)
{
	skeleton->update(deltaTime);
	state->update(deltaTime * timeScale);
	state->apply(*skeleton);
	skeleton->updateWorldTransform();
}

void spine::SkeletonDrawable::draw() const
{
}
