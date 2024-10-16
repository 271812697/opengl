

#include<glad/glad.h>

#include <Debug/Logger.h>

#include "Rendering/Resources/Texture.h"
#include "Rendering/Resources/Shader.h"

Rendering::Resources::Shader::Shader(const std::string p_path, uint32_t p_id) : path(p_path), id(p_id)
{
	QueryUniforms();
}

Rendering::Resources::Shader::~Shader()
{
	glDeleteProgram(id);
}

void Rendering::Resources::Shader::Bind() const
{
	glUseProgram(id);
}

void Rendering::Resources::Shader::Unbind() const
{
	glUseProgram(0);
}

void Rendering::Resources::Shader::SetUniformInt(const std::string& p_name, int p_value)
{
	glUniform1i(GetUniformLocation(p_name), p_value);
}

void Rendering::Resources::Shader::SetUniformUint(const std::string& p_name, unsigned int p_value)
{
    glUniform1ui(GetUniformLocation(p_name), p_value);
}

void Rendering::Resources::Shader::SetUniformFloat(const std::string& p_name, float p_value)
{
	glUniform1f(GetUniformLocation(p_name), p_value);
}

void Rendering::Resources::Shader::SetUniformVec2(const std::string & p_name, const Maths::FVector2 & p_vec2)
{
	glUniform2f(GetUniformLocation(p_name), p_vec2.x, p_vec2.y);
}

void Rendering::Resources::Shader::SetUniformVec3(const std::string& p_name, const Maths::FVector3& p_vec3)
{
	glUniform3f(GetUniformLocation(p_name), p_vec3.x, p_vec3.y, p_vec3.z);
}

void Rendering::Resources::Shader::SetUniformVec4(const std::string& p_name, const Maths::FVector4& p_vec4)
{
	glUniform4f(GetUniformLocation(p_name), p_vec4.x, p_vec4.y, p_vec4.z, p_vec4.w);
}

void Rendering::Resources::Shader::SetUniformMat4(const std::string& p_name, const Maths::FMatrix4& p_mat4)
{
	glUniformMatrix4fv(GetUniformLocation(p_name), 1, GL_TRUE, &p_mat4.data[0]);
}

int Rendering::Resources::Shader::GetUniformInt(const std::string& p_name)
{
	int value;
	glGetUniformiv(id, GetUniformLocation(p_name), &value);
   
	return value;
}

uint32_t Rendering::Resources::Shader::GetUniformUInt(const std::string& p_name)
{
    unsigned int value;
    glGetUniformuiv(id, GetUniformLocation(p_name),&value);
    return value;
}

float Rendering::Resources::Shader::GetUniformFloat(const std::string& p_name)
{
	float value;
	glGetUniformfv(id, GetUniformLocation(p_name), &value);
	return value;
}

Maths::FVector2 Rendering::Resources::Shader::GetUniformVec2(const std::string& p_name)
{
	GLfloat values[2];
	glGetUniformfv(id, GetUniformLocation(p_name), values);
	return reinterpret_cast<Maths::FVector2&>(values);
}

Maths::FVector3 Rendering::Resources::Shader::GetUniformVec3(const std::string& p_name)
{
	GLfloat values[3];
	glGetUniformfv(id, GetUniformLocation(p_name), values);
	return reinterpret_cast<Maths::FVector3&>(values);
}

Maths::FVector4 Rendering::Resources::Shader::GetUniformVec4(const std::string& p_name)
{
	GLfloat values[4];
	glGetUniformfv(id, GetUniformLocation(p_name), values);
	return reinterpret_cast<Maths::FVector4&>(values);
}

Maths::FMatrix4 Rendering::Resources::Shader::GetUniformMat4(const std::string& p_name)
{
	GLfloat values[16];
	glGetUniformfv(id, GetUniformLocation(p_name), values);
	return reinterpret_cast<Maths::FMatrix4&>(values);
}

bool Rendering::Resources::Shader::IsEngineMember(const std::string & p_uniformName)
{
    return p_uniformName.rfind("ubo_", 0) == 0
        || p_uniformName == "irradiance_map"
        || p_uniformName == "prefilter_map"
        || p_uniformName == "BRDF_LUT";

}

uint32_t Rendering::Resources::Shader::GetUniformLocation(const std::string& name)
{
	if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end())
		return m_uniformLocationCache.at(name);

	const int location = glGetUniformLocation(id, name.c_str());

	if (location == -1)
		OVLOG_WARNING("Uniform: '" + name + "' doesn't exist");

	m_uniformLocationCache[name] = location;

	return location;
}

void Rendering::Resources::Shader::QueryUniforms()
{
	GLint numActiveUniforms = 0;
	uniforms.clear();
	glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &numActiveUniforms);
	std::vector<GLchar> nameData(256);

	for (int unif = 0; unif < numActiveUniforms; ++unif)
	{
		GLint arraySize = 0;
		GLenum type = 0;
		GLsizei actualLength = 0;
		glGetActiveUniform(id, unif, static_cast<GLsizei>(nameData.size()), &actualLength, &arraySize, &type, &nameData[0]);
		std::string name(static_cast<char*>(nameData.data()), actualLength);

		if (!IsEngineMember(name))
		{
			std::any defaultValue;

			switch (static_cast<UniformType>(type))
			{
			case Rendering::Resources::UniformType::UNIFORM_BOOL:		defaultValue = std::make_any<bool>(GetUniformInt(name));					break;
			case Rendering::Resources::UniformType::UNIFORM_INT:		defaultValue = std::make_any<int>(GetUniformInt(name));	                    break;
            case Rendering::Resources::UniformType::UNIFORM_UINT:		defaultValue = std::make_any<unsigned int>(GetUniformUInt(name));           break;
			case Rendering::Resources::UniformType::UNIFORM_FLOAT:		defaultValue = std::make_any<float>(GetUniformFloat(name));					break;
			case Rendering::Resources::UniformType::UNIFORM_FLOAT_VEC2:	defaultValue = std::make_any<Maths::FVector2>(GetUniformVec2(name));		break;
			case Rendering::Resources::UniformType::UNIFORM_FLOAT_VEC3:	defaultValue = std::make_any<Maths::FVector3>(GetUniformVec3(name));		break;
			case Rendering::Resources::UniformType::UNIFORM_FLOAT_VEC4:	defaultValue = std::make_any<Maths::FVector4>(GetUniformVec4(name));		break;
			case Rendering::Resources::UniformType::UNIFORM_SAMPLER_2D:	defaultValue = std::make_any<Rendering::Resources::Texture*>(nullptr);	    break;
            case Rendering::Resources::UniformType::UNIFORM_FLOAT_MAT4:	defaultValue = std::make_any<Maths::FMatrix4>(GetUniformMat4(name));	    break;
            }

			if (defaultValue.has_value())
			{
				uniforms.push_back
				({
					static_cast<UniformType>(type),
					name,
					GetUniformLocation(nameData.data()),
					defaultValue
				});
			}
		}
	}
}

const Rendering::Resources::UniformInfo* Rendering::Resources::Shader::GetUniformInfo(const std::string& p_name) const
{
	auto found = std::find_if(uniforms.begin(), uniforms.end(), [&p_name](const UniformInfo& p_element)
	{
		return p_name == p_element.name;
	});

	if (found != uniforms.end())
		return &*found;
	else
		return nullptr;
}
