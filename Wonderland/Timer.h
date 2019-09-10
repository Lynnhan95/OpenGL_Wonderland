#pragma once
#include <glad/glad.h>
#include "GLFW/glfw3.h"
#include "Scene.h"

class Timer
{
private:
	static double m_last_frame_time;
	static double m_delta_time;
	static float  prevTime;    // Record of currentTime

public:
	static float  currentTime; // Simulated time, represents 0-24h (val 0-120s)

	static double deltaTime(); // Get time between this frame and last frame
	static void   Update();
	static void   TimeElapse(Scene* scene); // Simluate dynamic scene lighting
};