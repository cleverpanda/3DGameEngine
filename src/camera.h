#pragma once
#include "camera.h"
#include "matrix.h"
#include "vector.h"

typedef struct {
	vec3_t pos;
	vec3_t dir;
	vec3_t vel;
	float yaw;
} camera_t;

extern camera_t camera;
