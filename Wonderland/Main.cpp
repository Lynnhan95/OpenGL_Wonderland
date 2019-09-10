// GLFW; GLAD must be included first
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// System
#include <iostream>

// Custom components
#include "SimParameters.h"
#include "GuiLayout.h"
#include "Camera.h"
#include "Scene.h"
#include "MapReader.h"
#include "Timer.h"
#include "AudioSource.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// Current scene
static Scene*		scene;
static Camera*		camera;
static AudioSource* audioSource;

// Initialized resource paths
static std::map<std::string, ResourcePath>			   gResource_paths;
// Initialized particle system profiles
static std::map<std::string, ParticleSystemProperties> gPartiSystem_profiles;
// Initialized light source profiles
static std::map<std::string, LightProfile>			   gLightSource_profiles;

// Cursor record
double prev_xpos = 0, prev_ypos = 0;

// Auto scale (Not implemented)
float windows_scaling = 1;


// Initialization
GLFWwindow* Start()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	// Retrieve the Windows DPI; (not implemented in GLFW 3.2?)
	//windows_scaling = glfwGetWindowContentScale() / (glfwGetFramebufferSize() / glfwGetWindowSize());

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, WINDOW_NAME.c_str(), NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return NULL;
	}

	glClearColor(CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], CLEAR_COLOR[3]);

	// Initialize Gui
	Init_Gui(window);

	// Initialize shaders
	RuntimeShader::instance().Init_Shaders();
	// Initialize resource paths
	Init_Resource_Paths(gResource_paths);
	// Initialize particle system profiles
	Init_ParticleSystem_Properties(gPartiSystem_profiles);
	// Initialize light source profiles
	Init_Light_Profiles(gLightSource_profiles);

	// Create scene
	scene = new Scene();
	LoadMap(scene, MAP_NAME.c_str(), gResource_paths, gPartiSystem_profiles, gLightSource_profiles);

	// Initialize lightmap
	scene->mainLightMap = *new LightMap();
	scene->mainLightMap.InitializeLightMap(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

	// Generate scene content combo list
	ImGui_Construct_ComboList(scene);

	// Generate Mipmap
	// When MAGnifying the image (no bigger mipmap available), use LINEAR filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// When MINifying the image, use a LINEAR blend of two mipmaps, each filtered LINEARLY too
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Generate mipmaps, by the way.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Create camera
	camera = new Camera();

	// Create skybox
	scene->UseSkymap(LoadCubemap(SKYCUBE_1_NAME));

	// Calibrate sun
	scene->GetSun()->profile.direction = glm::vec3(glm::rotate((STARTING_TIME - 60.0f) * 4 * D2R, glm::vec3(0, 0, 1)) * glm::vec4(0, -1, 0, 0));

	// Initialize audio source
	audioSource = new AudioSource();
	audioSource->Initialize(true, BGM_FILE.c_str());
	audioSource->Start();

	return window;
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera->Update(); // Camera has to update first
	scene ->DrawScene(camera);

	Draw_Gui(camera, scene);
}

// Frame update
void Update(GLFWwindow* window)
{
	// process input
	processInput(window);

	// Play sound
	audioSource->Play();

	// draw
	display();

	// update content
	scene->Update(camera);
	Timer::Update();
	Timer::TimeElapse(scene);
}

// Program entry
int main()
{
	GLFWwindow* window = Start();
	if (window == NULL)
	{
		return -1;
	}

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		Update(window);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float time_factor = Timer::deltaTime() * 100;

	// Move camera with WASD+QE
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera->Translate(camera->forward()*(cam_speed / CAM_SPEED_FACTOR)*time_factor);

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera->Translate(-camera->forward()*(cam_speed / CAM_SPEED_FACTOR)*time_factor);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera->Translate(camera->right()*(cam_speed / CAM_SPEED_FACTOR)*time_factor);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera->Translate(-camera->right()*(cam_speed / CAM_SPEED_FACTOR)*time_factor);

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera->Translate(UP*(cam_speed / CAM_SPEED_FACTOR)*time_factor);

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera->Translate(-UP*(cam_speed / CAM_SPEED_FACTOR)*time_factor);

	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	
	// Control camera orientation with mouse
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		camera->Rotate(glm::vec3(ypos - prev_ypos, xpos - prev_xpos, 0)*D2R*(cam_rot_speed / CAM_ROT_SPEED_FACTOR)*time_factor);
	}
	else
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	prev_xpos = xpos;
	prev_ypos = ypos;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	curr_wWidth = width;
	curr_wHeight = height;
	// make sure the viewport matches the new window dimensions
	glViewport(0, 0, width, height);
}

// Reload current scene from map file
void ReloadScene(const char* addrstr)
{
	if (scene != NULL)
	{
		scene->Clear();
		LoadMap(scene, addrstr, gResource_paths, gPartiSystem_profiles, gLightSource_profiles);
	}
}