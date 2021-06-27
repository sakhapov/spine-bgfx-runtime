#include "spineBgfx.h"

#define SPINE_MESH_VERTEX_COUNT_MAX 1000

void spine::BGFXTextureLoader::load(AtlasPage& page, const String& path)
{
	bgfx::TextureInfo ti;
	spine::SkeletonDrawable::Texture txd;
	txd.textureHndl = loadTexture(path.buffer(), BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, 0, &ti, nullptr);
	txd.s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler); //hardcode just for representation of one sampler
	txd.height = ti.height;
	txd.width = ti.width;

	page.setRendererObject(&txd);
	page.width = txd.width;
	page.height = txd.height;
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
	vertexArray = new std::vector<extension::VertexDecl>();
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
	delete vertexArray;
	if (ownsAnimationStateData) delete state->getData();
	delete state;
	delete skeleton;
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
	vertexArray->clear();

	// Early out if skeleton is invisible
	if (skeleton->getColor().a == 0) return;
	if (vertexEffect != NULL) vertexEffect->begin(*skeleton);

	extension::VertexDecl vertex;
	spine::SkeletonDrawable::Texture* texture = nullptr;
	for (unsigned i = 0; i < skeleton->getSlots().size(); ++i)
	{
		Slot& slot = *skeleton->getDrawOrder()[i];
		Attachment* attachment = slot.getAttachment();
		if (!attachment) continue;

		// Early out if the slot color is 0 or the bone is not active
		if (slot.getColor().a == 0 || !slot.getBone().isActive()) {
			clipper.clipEnd(slot);
			continue;
		}

		Vector<float>* vertices = &worldVertices;
		int verticesCount = 0;
		Vector<float>* uvs = NULL;
		Vector<unsigned short>* indices = NULL;
		int indicesCount = 0;
		Color* attachmentColor;

		if (attachment->getRTTI().isExactly(RegionAttachment::rtti)) {
			RegionAttachment* regionAttachment = (RegionAttachment*)attachment;
			attachmentColor = &regionAttachment->getColor();

			// Early out if the slot color is 0
			if (attachmentColor->a == 0) {
				clipper.clipEnd(slot);
				continue;
			}

			worldVertices.setSize(8, 0);
			regionAttachment->computeWorldVertices(slot.getBone(), worldVertices, 0, 2);
			verticesCount = 4;
			uvs = &regionAttachment->getUVs();
			indices = &quadIndices;
			indicesCount = 6;
			texture = (spine::SkeletonDrawable::Texture*)((AtlasRegion*)regionAttachment->getRendererObject())->page->getRendererObject();

		}
		else if (attachment->getRTTI().isExactly(MeshAttachment::rtti)) {
			MeshAttachment* mesh = (MeshAttachment*)attachment;
			attachmentColor = &mesh->getColor();

			// Early out if the slot color is 0
			if (attachmentColor->a == 0) {
				clipper.clipEnd(slot);
				continue;
			}

			worldVertices.setSize(mesh->getWorldVerticesLength(), 0);
			texture = (spine::SkeletonDrawable::Texture*)((AtlasRegion*)mesh->getRendererObject())->page->getRendererObject();
			mesh->computeWorldVertices(slot, 0, mesh->getWorldVerticesLength(), worldVertices, 0, 2);
			verticesCount = mesh->getWorldVerticesLength() >> 1;
			uvs = &mesh->getUVs();
			indices = &mesh->getTriangles();
			indicesCount = mesh->getTriangles().size();

		}
		else if (attachment->getRTTI().isExactly(ClippingAttachment::rtti)) {
			ClippingAttachment* clip = (ClippingAttachment*)slot.getAttachment();
			clipper.clipStart(slot, clip);
			continue;
		}
		else continue;

		extension::VertexDecl::init(); //setup vertex

		bgfx::setTexture(0, texture->s_texColor, texture->textureHndl);

		float r = static_cast<float>(skeleton->getColor().r * slot.getColor().r * attachmentColor->r * 255);
		float g = static_cast<float>(skeleton->getColor().g * slot.getColor().g * attachmentColor->g * 255);
		float b = static_cast<float>(skeleton->getColor().b * slot.getColor().b * attachmentColor->b * 255);
		float a = static_cast<float>(skeleton->getColor().a * slot.getColor().a * attachmentColor->a * 255);
		vertex.color.r = r;
		vertex.color.g = g;
		vertex.color.b = b;
		vertex.color.a = a;

		Color light;
		light.r = r / 255.0f;
		light.g = g / 255.0f;
		light.b = b / 255.0f;
		light.a = a / 255.0f;

		if (clipper.isClipping()) {
			clipper.clipTriangles(worldVertices, *indices, *uvs, 2);
			vertices = &clipper.getClippedVertices();
			verticesCount = clipper.getClippedVertices().size() >> 1;
			uvs = &clipper.getClippedUVs();
			indices = &clipper.getClippedTriangles();
			indicesCount = clipper.getClippedTriangles().size();
		}

		if (vertexEffect != 0) {
			tempUvs.clear();
			tempColors.clear();
			for (int ii = 0; ii < verticesCount; ii++) {
				Color vertexColor = light;
				Color dark;
				dark.r = dark.g = dark.b = dark.a = 0;
				int index = ii << 1;
				float x = (*vertices)[index];
				float y = (*vertices)[index + 1];
				float u = (*uvs)[index];
				float v = (*uvs)[index + 1];
				vertexEffect->transform(x, y, u, v, vertexColor, dark);
				(*vertices)[index] = x;
				(*vertices)[index + 1] = y;
				tempUvs.add(u);
				tempUvs.add(v);
				tempColors.add(vertexColor);
			}

			for (int ii = 0; ii < indicesCount; ++ii) {
				int index = (*indices)[ii] << 1;
				vertex.x = (*vertices)[index];
				vertex.y = (*vertices)[index + 1];
				vertex.u = (*uvs)[index] * texture->width;
				vertex.v = (*uvs)[index + 1] * texture->height;
				Color vertexColor = tempColors[index >> 1];
				vertex.color.r = static_cast<uint8_t>(vertexColor.r * 255);
				vertex.color.g = static_cast<uint8_t>(vertexColor.g * 255);
				vertex.color.b = static_cast<uint16_t>(vertexColor.b * 255);
				vertex.color.a = static_cast<uint16_t>(vertexColor.a * 255);
				vertexArray->push_back(vertex);
			}
		}
		else {
			for (int ii = 0; ii < indicesCount; ++ii) {
				int index = (*indices)[ii] << 1;
				vertex.x = (*vertices)[index];
				vertex.y = (*vertices)[index + 1];
				vertex.u = (*uvs)[index] * texture->width;
				vertex.v = (*uvs)[index + 1] * texture->height;
				vertexArray->push_back(vertex);
			}
		}
		clipper.clipEnd(slot);

		auto vbh = bgfx::createVertexBuffer(bgfx::makeRef(vertices, sizeof(vertices)), extension::VertexDecl::ms_layout);
		auto ibh = bgfx::createIndexBuffer(bgfx::makeRef(indices, sizeof(indices)));

		bgfx::setVertexBuffer(0, vbh);
		bgfx::setIndexBuffer(ibh);

		bgfx::submit(0, shaderProg);
	}

	if (vertexEffect != 0) vertexEffect->end();
}
