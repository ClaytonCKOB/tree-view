
// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>
bool hasSphereBulletCollision(glm::vec4 sphere_center, float sphere_radius, glm::vec4 bullet);

bool hasSphereSphereCollision(glm::vec4 sphere_center, float sphere_radius, glm::vec4 sphere_2, float sphere_2radius);
bool hasPointPlaneCollision(glm::vec4 point, glm::vec4 plane);