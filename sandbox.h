#ifndef SANDBOX_H_
#define SANDBOX_H_

#include "spineBgfx.h"
#include <glfw/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

using namespace std;
using namespace spine;

class sandbox
{
public:
	bool bgfxInit();
	void testcase(void func(spine::SkeletonData* skeletonData, spine::Atlas* atlas), const char* jsonName, const char* binaryName, const char* atlasName, float scale);

	shared_ptr<spine::SkeletonData> readSkeletonJsonData(const spine::String& filename, spine::Atlas* atlas, float scale);
	shared_ptr<spine::SkeletonData> readSkeletonBinaryData(const char* filename, spine::Atlas* atlas, float scale);
};
#endif SANDBOX_H_