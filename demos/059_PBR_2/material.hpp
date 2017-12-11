#pragma once

#include <memory>
#include "shader.hpp"
#include "materialparams.hpp"

struct Material
{
	explicit Material();

	virtual void bind() = 0;
	virtual void unbind() = 0;	

	void bindParams(const MaterialParams& params);

	std::shared_ptr<Program> program;

	template<typename MapType> void bindParams(const MapType& paramsMap);
};