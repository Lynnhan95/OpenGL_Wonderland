#pragma once

#include <string>
#include <glm/glm.hpp>

const float PI = 3.1415926535f;
const float D2R = PI / 180; // Degree to radius

// Startup window size
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

static const std::string WINDOW_NAME = "Wonderland (beta)";

static unsigned int curr_wWidth =  SCR_WIDTH;
static unsigned int curr_wHeight = SCR_HEIGHT;

// Fixed camera parameters
const float		CLEAR_COLOR[4] = { 0.1f, 0.1f, 0.1f, 0.0f };
const float		FOV = 45.0f; // 45.0 is a plausible value
const float		NEAR_CLIP = 0.03f;
const float		FAR_CLIP = 1000.0f;
const glm::vec3 INIT_POS(-2.0f, 2.0f, -0.5f);
const glm::vec3 INIT_FWD(0, 0, 1);
const glm::vec3 INIT_RGT(-1, 0, 0);
const glm::vec3 INIT_ROT(0, 90 * D2R, 0);
const glm::vec3 UP(0, 1, 0);
const float		CAM_SPEED_FACTOR = 125.0f;
const float		CAM_ROT_SPEED_FACTOR = 7.5f;

// Modifiable camera parameters
static int   cam_speed = 2;
static int   cam_rot_speed = 2;
static float window_aspect = (float)SCR_WIDTH / SCR_HEIGHT;

// Stroke parameters
// TODO: Make this in screen space
const float		STROKE_WIDTH = 2.0f;
const glm::vec3 STROKE_COLOR(1, 0.3f, 0);
//const glm::vec3 STROKE_COLOR_BLOCKED(0.5f, 0.15f, 0);

// Fixed lightning parameters
const unsigned int MAX_POINT_LIGHT = 10;
const unsigned int MAX_DIRECTIONAL_LIGHT = 1;
const unsigned int SHADOW_MAP_WIDTH = 2048;
const unsigned int SHADOW_MAP_HEIGHT = 2048;
const unsigned int SHADOW_DISTANCE = 10;
const float STARTING_TIME = 50.0f; // (0-120)
const float TIME_ELAPSE_SPEED = 1; // Warning, setting this value too large would cause undesired consequence
const float MINIMUM_LUMINANCE = 0.1f;

// Physics parameters
const glm::vec3 GRAVITY(0, -9.8f, 0); // Only implemented for particles

// Shader paths
static const std::string VS_BASIC			("Shader\\vert_basic.glsl");
static const std::string VS_LIT				("Shader\\vert_light.glsl");
static const std::string VS_LIT_FISH		("Shader\\vert_light_fish.glsl");
static const std::string VS_LIT_WATER		("Shader\\vert_light_water.glsl");
static const std::string VS_SHADOWMAP_FISH	("Shader\\vert_shadowmap_fish.glsl");
static const std::string VS_STROKE			("Shader\\vert_stroke.glsl");
static const std::string VS_SHADOWMAP		("Shader\\vert_shadowmap.glsl");
static const std::string VS_SKYCUBE			("Shader\\vert_skycube.glsl");
static const std::string VS_PARTICLE_BASIC	("Shader\\vert_particle.glsl");
static const std::string VS_LIT_LEAVES		("Shader\\vert_light_leaves.glsl");
static const std::string VS_SHADOWMAP_LEAVES("Shader\\vert_shadowmap_leaves.glsl");

static const std::string GS_STROKE			("Shader\\geo_stroke.glsl");

static const std::string FS_SHADOWMAP		("Shader\\frag_shadowmap.glsl");
static const std::string FS_BASIC			("Shader\\frag_basic.glsl");
static const std::string FS_LIT				("Shader\\frag_light.glsl");
static const std::string FS_STROKE			("Shader\\frag_stroke.glsl");
static const std::string FS_SKYCUBE			("Shader\\frag_skycube.glsl");
static const std::string FS_PARTICLE_BASIC	("Shader\\frag_particle.glsl");

// Map
static const std::string MAP_NAME = "Map\\Wonderland.wmap";

// Resource paths

// Default texture
static const std::string DEFAULT_TEXTURE_NAME =		"Textures\\Objects\\Pure_tile_White.jpg";
// Tree
//static const std::string TREE1_MESH_NAME =		"Models\\Tree\\Birch_tree1.fbx";
//static const std::string TREE1_ALBEDO_0_NAME =	"Textures\\Tree\\Birch_bark.tga";
//static const std::string TREE1_ALBEDO_1_NAME =	"Textures\\Tree\\Birch_leaves.tga";
static const std::string TREE1_MESH_NAME =			"Models\\formal_scene\\tree1\\tree1.obj";
static const std::string TREE2_MESH_NAME =			"Models\\formal_scene\\tree2\\tree2.obj";
static const std::string TREE3_MESH_NAME =			"Models\\formal_scene\\tree3\\tree3.obj";
// Fish
static const std::string FISH1_MESH_NAME =			"Models\\Amago\\Amago0.obj";
static const std::string FISH1_ALBEDO_0_NAME =		"Textures\\Amago\\AmagoT.bmp";
// Terrain
//static const std::string GROUND_MESH_NAME =		"Models\\Terrain\\desert.3DS";
//static const std::string GROUND_ALBEDO_0_NAME =	"Textures\\Terrain\\cartoon-grass-texture-121937-9720577_Mod.png";
//static const std::string WATER1_MESH_NAME =		"Models\\Terrain\\Plane.obj";
//static const std::string WATER1_ALBEDO_0_NAME =	"Textures\\Terrain\\CartoonWater.jpg";
//static const std::string GROUND_MESH_NAME =		"Models\\formal_scene\\grassland\\newGrassLand.obj";
static const std::string GROUND_MESH_NAME =			"Models\\formal_scene\\grassland\\grassland.obj";
static const std::string GROUND_ALBEDO_0_NAME =		"Models\\formal_scene\\grassland\\UntitledColor.jpg";
static const std::string MOUNTAIN1_MESH_NAME =		"Models\\formal_scene\\mountain1\\mountain_dark.obj";
static const std::string MOUNTAIN1_ALBEDO_0_NAME =	"Models\\formal_scene\\mountain1\\mountain.1Color.jpg";
static const std::string MOUNTAIN2_MESH_NAME =		"Models\\formal_scene\\mountain2\\mountain_light.obj";
static const std::string MOUNTAIN2_ALBEDO_0_NAME =	"Models\\formal_scene\\mountain2\\UntitledColor.jpg";
static const std::string WATER1_MESH_NAME =			"Models\\formal_scene\\lake1\\lake1.obj";
static const std::string WATER2_MESH_NAME =			"Models\\formal_scene\\lake2\\lake2.obj";
static const std::string POND1_MESH_NAME =		    "Models\\formal_scene\\pond\\pond.obj";
// Objects
//static const std::string ROCK1_MESH_NAME =		"Models\\Objects\\Rock1.fbx";
//static const std::string ROCK1_ALBEDO_0_NAME =	"Textures\\Objects\\Rock1.tga";
//static const std::string HOUSE1_MESH_NAME =		"Models\\Objects\\cartoon_house.FBX";
//static const std::string HOUSE1_ALBEDO_0_NAME =	"Textures\\Objects\\Cartoon_house.bmp";
//static const std::string HOUSE1_ALBEDO_1_NAME =	"Textures\\Objects\\WINDOW.bmp";
//static const std::string HOUSE1_ALBEDO_2_NAME =	"Textures\\Objects\\Pure_tile1.jpg";
static const std::string ROCK1_MESH_NAME =			"Models\\formal_scene\\stone1\\stone1.obj";
static const std::string ROCK2_MESH_NAME =			"Models\\formal_scene\\stone2\\stone2.obj";
static const std::string HOUSE1_MESH_NAME =			"Models\\formal_scene\\house\\house.obj";
static const std::string LADDER1_MESH_NAME =		"Models\\formal_scene\\ladder\\ladder.obj";
static const std::string ROAD1_MESH_NAME =			"Models\\formal_scene\\road\\innerRoad.obj";
static const std::string ROAD2_MESH_NAME =			"Models\\formal_scene\\road\\outerRoad.obj";
static const std::string CLOUD1_MESH_NAME =			"Models\\formal_scene\\cloud\\cloud.obj";
static const std::string BOAT1_MESH_NAME =			"Models\\formal_scene\\boat\\boat.obj";
static const std::string BALLON1_MESH_NAME =		"Models\\formal_scene\\ballon\\ballon.obj";
static const std::string FIRE1_MESH_NAME =			"Models\\formal_scene\\fire\\fire.obj";
// Plants
//static const std::string FLOWER1_MESH_NAME =		"Models\\Plants\\Flower1.fbx";
//static const std::string MUSHROOM1_MESH_NAME =	"Models\\Plants\\Mushroom1.fbx";
//static const std::string GRASS1_MESH_NAME =		"Models\\Plants\\Grass1.fbx";
//static const std::string PLANT_ALBEDO_0_NAME =	"Textures\\Plants\\Plant.tga";
static const std::string FLOWER1_MESH_NAME =		"Models\\formal_scene\\sunflower\\sunflower.obj";
// Animals
static const std::string COW_MESH_NAME =			"Models\\formal_scene\\cow\\cow.obj";
static const std::string DEAR_MESH_NAME =			"Models\\formal_scene\\dear\\dear.obj";
// Particles
static const std::string PLANE_MESH_NAME =			"Models\\Terrain\\Plane.obj";
static const std::string FOG1_ALBEDO_0_NAME =		"Textures\\Particles\\ParticleCloudWhite.png";
static const std::string SNOW1_ALBEDO_0_NAME =		"Textures\\Particles\\Snowflake.png";
static const std::string FIRE1_ALBEDO_0_NAME =		"Textures\\Particles\\Fire.png";

// Skycube
//static const std::string SKYCUBE_MESH_NAME =		"Models\\Elements\\cube_m.obj";
static const std::string SKYCUBE_1_NAME[6] = {
	//"Textures\\Sky\\ely_lakes\\lakes_ft.tga",
	//"Textures\\Sky\\ely_lakes\\lakes_bk.tga",
	//"Textures\\Sky\\ely_lakes\\lakes_up.tga",
	//"Textures\\Sky\\ely_lakes\\lakes_dn.tga",
	//"Textures\\Sky\\ely_lakes\\lakes_rt.tga",
	//"Textures\\Sky\\ely_lakes\\lakes_lf.tga"
	//"Textures\\Sky\\mp_hanging\\hangingstone_ft.tga",
	//"Textures\\Sky\\mp_hanging\\hangingstone_bk.tga",
	//"Textures\\Sky\\mp_hanging\\hangingstone_up.tga",
	//"Textures\\Sky\\mp_hanging\\hangingstone_dn.tga",
	//"Textures\\Sky\\mp_hanging\\hangingstone_rt.tga",
	//"Textures\\Sky\\mp_hanging\\hangingstone_lf.tga"
	"Textures\\Sky\\hw_alps\\alps_ft.tga",
	"Textures\\Sky\\hw_alps\\alps_bk.tga",
	"Textures\\Sky\\hw_alps\\alps_up.tga",
	"Textures\\Sky\\hw_alps\\alps_dn.tga",
	"Textures\\Sky\\hw_alps\\alps_rt.tga",
	"Textures\\Sky\\hw_alps\\alps_lf.tga"
};

// Music
// Currently only support WAV format
static const std::string BGM_FILE = "Audio\\Alto's Adventure - Zen Mode Soundtrack (OST).wav";
//static const std::string BGM_FILE = "Audio\\stereo.wav"; // For loop testing