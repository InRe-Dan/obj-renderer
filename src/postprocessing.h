#pragma once

#include "utility/vectors.h"

#include <vector>
#include <cstdint>
#include <stdio.h>

#include <glm/glm.hpp>

struct Kernel
{
  public:
	Kernel(std::vector<std::vector<float>> kernelTemplate);
	const std::vector<std::vector<float>> vect;
	const int size;
	const int nFactor;
	const int padding;
};

static const Kernel boxBlurKernel = Kernel(std::vector<std::vector<float>>{
	std::vector<float>{1.0f, 1.0f, 1.0f},
	std::vector<float>{1.0f, 1.0f, 1.0f},
	std::vector<float>{1.0f, 1.0f, 1.0f}});

static const Kernel gaussian3x3 = Kernel(std::vector<std::vector<float>>{
	std::vector<float>{1.0f, 2.0f, 1.0f},
	std::vector<float>{2.0f, 4.0f, 2.0f},
	std::vector<float>{1.0f, 2.0f, 1.0f}});

static const Kernel sharpenKernel = Kernel(std::vector<std::vector<float>>{
	std::vector<float>{0.0f, -1.0f, 0.0f},
	std::vector<float>{-1.0f, 5.0f, -1.0f},
	std::vector<float>{0.0f, -1.0f, 0.0f}});

static const Kernel edgeDetectionKernelH = Kernel(std::vector<std::vector<float>>{
	std::vector<float>{1.0f, 2.0f, 1.0f},
	std::vector<float>{0.0f, 0.0f, 0.0f},
	std::vector<float>{-1.0f, -2.0f, -1.0f}});
static const Kernel edgeDetectionKernelV = Kernel(std::vector<std::vector<float>>{
	std::vector<float>{-1.0f, 0.0f, 1.0f},
	std::vector<float>{-2.0f, 0.0f, 2.0f},
	std::vector<float>{-1.0f, 0.0f, 1.0f}});

static const Kernel edgeDetectionFull = Kernel(std::vector<std::vector<float>>{
	std::vector<float>{-1.0f, -1.0f, -1.0f},
	std::vector<float>{-1.0f, 8.0f, -1.0f},
	std::vector<float>{-1.0f, -1.0f, -1.0f}});

// Does not apply to the edges of the target.
std::vector<std::vector<uint32_t>>
applyKernel(std::vector<std::vector<uint32_t>>& target, const Kernel& k);

void blackAndWhite(std::vector<std::vector<uint32_t>>& target);

// Assumes image to be monochromatic. Applies thresholding to all pixels.
void threshold(
	std::vector<std::vector<uint32_t>>& target,
	glm::vec3 thresholds);

// Assumes images are monochromatic. Find the hypotenuse of two images,
// normalize them to 0-255 and write them into the target.
void hypot(
	std::vector<std::vector<uint32_t>>& target,
	std::vector<std::vector<uint32_t>>& one,
	std::vector<std::vector<uint32_t>>& two);

void composite(
	std::vector<std::vector<uint32_t>>& target,
	std::vector<std::vector<uint32_t>>& image);

// Upscales any frame buffer into an equal or larger one.
void arbitraryUpscale(
	std::vector<std::vector<uint32_t>> source,
	std::vector<std::vector<uint32_t>>& target);

// This is a work in progress - nearest neighbour seems sufficient.
void bilinearUpscale(
	std::vector<std::vector<uint32_t>> source,
	std::vector<std::vector<uint32_t>>& target);