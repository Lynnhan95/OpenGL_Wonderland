#pragma once
#include <vector>
#include <string>

enum HandleType
{
	FLT1 = 0,
	VEC2,
	VEC3,
	VEC4,
	COL3,
	HT_BOOL
};

enum OperationType
{
	OT_INPUT = 0,
	SLIDE
};

// Interaction handle for Gui
struct Handle
{
	HandleType type;
	OperationType opType;

	std::string name = "<variable>";

	// Pointer slots
	float* v1;
	float* v2; // Preserved position

	float min = 0.0f;
	float max = 1.0f;

	bool *b1;
};

struct IInteractable
{
	// Retrieve variable name
	virtual std::string GetIObjectName() = 0;

	// Expose variable reference to Gui
	virtual std::vector<Handle> GetIHandle() = 0;

	// Clear handles
	virtual void ClearHandles() = 0;
};