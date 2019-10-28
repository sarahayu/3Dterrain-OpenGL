#include "Frustum.h"
#include "../chunk/Chunk.h"

Frustum::Frustum(const glm::mat4 & projView)
{

	// left
	planes[0].x = projView[0][3] + projView[0][0];
	planes[0].y = projView[1][3] + projView[1][0];
	planes[0].z = projView[2][3] + projView[2][0];
	planes[0].w = projView[3][3] + projView[3][0];

	// right
	planes[1].x = projView[0][3] - projView[0][0];
	planes[1].y = projView[1][3] - projView[1][0];
	planes[1].z = projView[2][3] - projView[2][0];
	planes[1].w = projView[3][3] - projView[3][0];

	// bottom
	planes[2].x = projView[0][3] + projView[0][1];
	planes[2].y = projView[1][3] + projView[1][1];
	planes[2].z = projView[2][3] + projView[2][1];
	planes[2].w = projView[3][3] + projView[3][1];

	// top
	planes[3].x = projView[0][3] - projView[0][1];
	planes[3].y = projView[1][3] - projView[1][1];
	planes[3].z = projView[2][3] - projView[2][1];
	planes[3].w = projView[3][3] - projView[3][1];

	// near
	planes[4].x = projView[0][3] + projView[0][2];
	planes[4].y = projView[1][3] + projView[1][2];
	planes[4].z = projView[2][3] + projView[2][2];
	planes[4].w = projView[3][3] + projView[3][2];

	// far
	planes[5].x = projView[0][3] - projView[0][2];
	planes[5].y = projView[1][3] - projView[1][2];
	planes[5].z = projView[2][3] - projView[2][2];
	planes[5].w = projView[3][3] - projView[3][2];

	for (auto& plane : planes)
	{
		float length = glm::length(glm::vec3(plane.x, plane.y, plane.z));
		plane /= length;
	}
}

const bool Frustum::within(const Chunk & c) const
{
	for (auto& plane : planes)
	{
		float distance = glm::dot(c.getVP({ plane.x,plane.y,plane.z }), { plane.x,plane.y,plane.z }) + plane.w;

		if (distance < 0)
		{
			return false;
		}
	}

	return true;
}
