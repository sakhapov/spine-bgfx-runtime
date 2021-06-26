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

void spineboy(spine::SkeletonData* skeletonData, spine::Atlas* atlas)
{
	SP_UNUSED(atlas);

	SkeletonBounds bounds;

	// Configure mixing.
	AnimationStateData stateData(skeletonData);
	stateData.setMix("walk", "jump", 0.2f);
	stateData.setMix("jump", "run", 0.2f);

	SkeletonDrawable drawable(skeletonData, &stateData);
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
		/*while (window.pollEvent(event))
			if (event.type == sf::Event::Closed) window.close();*/

			/*float delta = deltaClock.getElapsedTime().asSeconds();
			deltaClock.restart();

			bounds.update(*skeleton, true);
			sf::Vector2i position = sf::Mouse::getPosition(window);
			if (bounds.containsPoint((float)position.x, (float)position.y)) {
				headSlot->getColor().g = 0;
				headSlot->getColor().b = 0;
			}
			else {
				headSlot->getColor().g = 1;
				headSlot->getColor().b = 1;
			}*/

		drawable.update(0);
		drawable.draw();

		//window.clear();
		//window.draw(drawable);
		//window.display();
	}
}

bool sandbox::bgfxInit()
{
	if (!glfwInit())
		return 1;
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Spine BGFX runtime prototype", nullptr, nullptr);
	if (!window)
		return 1;
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
		return 1;

	testcase(spineboy, "export/spineboy-pro.json", "export/spineboy-pro.skel", "export/spineboy-pma.atlas", 0.6f);

	const bgfx::ViewId kClearView = 0;
	bgfx::setViewClear(kClearView, BGFX_CLEAR_COLOR);
	bgfx::setViewRect(kClearView, 0, 0, bgfx::BackbufferRatio::Equal);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		// Handle window resize.
		int oldWidth = width, oldHeight = height;
		glfwGetWindowSize(window, &width, &height);
		if (width != oldWidth || height != oldHeight) {
			bgfx::reset((uint32_t)width, (uint32_t)height, BGFX_RESET_VSYNC);
			bgfx::setViewRect(kClearView, 0, 0, bgfx::BackbufferRatio::Equal);
		}
		// This dummy draw call is here to make sure that view 0 is cleared if no other draw calls are submitted to view 0.
		bgfx::touch(kClearView);
		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		//bgfx::dbgTextImage(bx::max<uint16_t>(uint16_t(width / 2 / 8), 20) - 20, bx::max<uint16_t>(uint16_t(height / 2 / 16), 6) - 6, 40, 12, s_logo, 160);
		bgfx::dbgTextPrintf(0, 0, 0x0f, "Press F1 to toggle stats.");
		bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");
		bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
		bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");
		const bgfx::Stats* stats = bgfx::getStats();
		bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters.", stats->width, stats->height, stats->textWidth, stats->textHeight);
		// Enable stats or debug text.
		bgfx::setDebug(s_showStats ? BGFX_DEBUG_STATS : BGFX_DEBUG_TEXT);
		// Advance to next frame. Process submitted rendering primitives.
		bgfx::frame();
	}
	bgfx::shutdown();
	glfwTerminate();

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

