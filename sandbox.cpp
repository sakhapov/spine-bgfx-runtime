#include "sandbox.h"

template<typename T, typename... Args>
unique_ptr<T> make_unique_test(Args&&... args) {
	return unique_ptr<T>(new T(forward<Args>(args)...));
}

static bool s_showStats = false;

static void glfw_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F1 && action == GLFW_RELEASE)
		s_showStats = !s_showStats;
}

GLFWwindow* window;
const bgfx::ViewId kClearView = 0;
int width = 1024;
int height = 768;

void spineboy(spine::SkeletonData* skeletonData, spine::Atlas* atlas)
{
	SP_UNUSED(atlas);

	SkeletonBounds bounds;

	// Configure mixing.
	AnimationStateData stateData(skeletonData);
	stateData.setMix("walk", "jump", 0.2f);
	stateData.setMix("jump", "run", 0.2f);

	SkeletonDrawable drawable(skeletonData, &stateData);

	drawable.shaderProg = loadProgram("vs_bump", "fs_bump");
	drawable.timeScale = 1;
	drawable.setUsePremultipliedAlpha(true);

	Skeleton* skeleton = drawable.skeleton;
	skeleton->setToSetupPose();

	skeleton->setPosition(320, 590);
	skeleton->updateWorldTransform();

	Slot* headSlot = skeleton->findSlot("head");

	//drawable.state->setListener(callback);
	drawable.state->addAnimation(0, "walk", true, 0);
	drawable.state->addAnimation(0, "jump", false, 3);
	drawable.state->addAnimation(0, "run", true, 0);

	//sf::RenderWindow window(sf::VideoMode(640, 640), "Spine SFML - spineboy");
	//window.setFramerateLimit(60);
	//sf::Event event;
	//sf::Clock deltaClock;

	while (true) {
		while (!glfwWindowShouldClose(window)) 
		{
			glfwPollEvents();
			// Handle window resize.
			int oldWidth = width, oldHeight = height;
			glfwGetWindowSize(window, &width, &height);
			if (width != oldWidth || height != oldHeight) {
				bgfx::reset((uint32_t)width, (uint32_t)height, BGFX_RESET_VSYNC);
				bgfx::setViewRect(kClearView, 0, 0, bgfx::BackbufferRatio::Equal);
			}

			bounds.update(*skeleton, true);

			bgfx::touch(kClearView);
			// Use debug font to print information about this example.
			bgfx::dbgTextClear();

			drawable.update(0);
			drawable.draw();
			bgfx::frame();
		}

		bgfx::shutdown();
		glfwTerminate();
	}
}

bool sandbox::bgfxInit()
{
	if (!glfwInit())
		throw;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(width, height, "Spine BGFX runtime prototype", nullptr, nullptr);
	if (!window)
		throw;
	glfwSetKeyCallback(window, glfw_keyCallback);

	bgfx::renderFrame();

	bgfx::Init init;

	init.type = bgfx::RendererType::OpenGL;
	init.platformData.nwh = glfwGetWin32Window(window);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	init.resolution.width = (uint32_t)width;
	init.resolution.height = (uint32_t)height;
	init.resolution.reset = BGFX_RESET_VSYNC;
	if (!bgfx::init(init))
		throw;

	bgfx::setViewClear(kClearView, BGFX_CLEAR_COLOR);
	bgfx::setViewRect(kClearView, 0, 0, bgfx::BackbufferRatio::Equal);

	testcase(spineboy, "export/spineboy-pro.json", "export/spineboy-pro.skel", "export/spineboy-pma.atlas", 0.6f);

	return true;
}

void sandbox::testcase(void func(SkeletonData* skeletonData, Atlas* atlas),
	const char* jsonName, const char* binaryName, const char* atlasName, float scale)
{
	BGFXTextureLoader textureLoader;
	auto atlas = make_unique_test<Atlas>(atlasName, &textureLoader);

	auto skeletonData = readSkeletonJsonData(jsonName, atlas.get(), scale);
	func(skeletonData.get(), atlas.get());

	skeletonData = readSkeletonBinaryData(binaryName, atlas.get(), scale);
	func(skeletonData.get(), atlas.get());
}

shared_ptr<SkeletonData> sandbox::readSkeletonJsonData(const String& filename, Atlas* atlas, float scale) {
	SkeletonJson json(atlas);
	json.setScale(scale);
	auto skeletonData = json.readSkeletonDataFile(filename);
	if (!skeletonData) {
		printf("%s\n", json.getError().buffer());
		exit(0);
	}
	return shared_ptr<SkeletonData>(skeletonData);
}

shared_ptr<SkeletonData> sandbox::readSkeletonBinaryData(const char* filename, Atlas* atlas, float scale)
{
	SkeletonBinary binary(atlas);
	binary.setScale(scale);
	auto skeletonData = binary.readSkeletonDataFile(filename);
	if (!skeletonData) {
		printf("%s\n", binary.getError().buffer());
		exit(0);
	}
	return shared_ptr<SkeletonData>(skeletonData);
}

