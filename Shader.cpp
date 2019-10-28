#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const std::string & vertexPath, const std::string & fragmentPath)
{
	std::ifstream vertFile(vertexPath), fragFile(fragmentPath);
	std::stringstream vertStream, fragStream;
	vertStream << vertFile.rdbuf(); fragStream << fragFile.rdbuf();
	vertFile.close(); fragFile.close();
	const std::string v = vertStream.str(), f = fragStream.str();
	const char *vertCode = v.c_str(), *fragCode = f.c_str();

	unsigned int vert, frag;

	vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vertCode, nullptr);
	glCompileShader(vert);
	checkCompileErrors(vert, "VERTEX");

	frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &fragCode, nullptr);
	glCompileShader(frag);
	checkCompileErrors(frag, "FRAGMENT");

	ID = glCreateProgram();
	glAttachShader(ID, vert);
	glAttachShader(ID, frag);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	glDeleteShader(vert); glDeleteShader(frag);
}

Shader::Shader()
{
}

void Shader::load(const std::string & vertexPath, const std::string & fragmentPath)
{
	std::ifstream vertFile(vertexPath), fragFile(fragmentPath);
	std::stringstream vertStream, fragStream;
	vertStream << vertFile.rdbuf(); fragStream << fragFile.rdbuf();
	vertFile.close(); fragFile.close();
	const std::string v = vertStream.str(), f = fragStream.str();
	const char *vertCode = v.c_str(), *fragCode = f.c_str();

	unsigned int vert, frag;

	vert = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vert, 1, &vertCode, nullptr);
	glCompileShader(vert);
	checkCompileErrors(vert, "VERTEX");

	frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, &fragCode, nullptr);
	glCompileShader(frag);
	checkCompileErrors(frag, "FRAGMENT");

	ID = glCreateProgram();
	glAttachShader(ID, vert);
	glAttachShader(ID, frag);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	glDeleteShader(vert); glDeleteShader(frag);
}

const unsigned int Shader::getID() const
{
	return ID;
}

void Shader::use() const
{
	glUseProgram(ID);
}

void Shader::setBool(const std::string & name, const bool & value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string & name, const int & value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string & name, const float & value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{
	glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string &name, float x, float y) const
{
	glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{
	glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
	glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
	glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::checkCompileErrors(const unsigned int & shader, const std::string & type)
{
	int success;
	char infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}
