#pragma once

#include <glm/glm.hpp>

namespace engine::graphics {
    struct ShaderData {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
	};

	struct Vertex {
		float position[3];
		float color[3];
	};
} // namespace engine::graphics