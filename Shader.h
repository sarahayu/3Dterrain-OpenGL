#pragma once
#include <glad\glad.h>
#include <string>
#include <glm\glm.hpp>

class Shader
{
public:
	Shader(const std::string &vertexPath, const std::string &fragmentPath);
	Shader();

	void load(const std::string &vertexPath, const std::string &fragmentPath);

	const unsigned int getID() const;
	void use() const;
	void setBool(const std::string &name, const bool &value) const;
	void setInt(const std::string &name, const int &value) const;
	void setFloat(const std::string &name, const float &value) const;
	void setVec2(const std::string &name, const glm::vec2 &value) const;
	void setVec2(const std::string &name, float x, float y) const;
	void setVec3(const std::string &name, const glm::vec3 &value) const;
	void setVec3(const std::string &name, float x, float y, float z) const;
	void setVec4(const std::string &name, const glm::vec4 &value) const;
	void setVec4(const std::string &name, float x, float y, float z, float w);
	void setMat2(const std::string &name, const glm::mat2 &mat) const;
	void setMat3(const std::string &name, const glm::mat3 &mat) const;
	void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
	unsigned int ID;

	void checkCompileErrors(const unsigned int &shader, const std::string &type);
};