/* The MIT License:

Copyright (c) 2012-2014 Ivan Gagis <igagis@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

// Home page: http://morda.googlecode.com

/**
 * @author Ivan Gagis <igagis@gmail.com>
 */

#pragma once


#include <ting/debug.hpp>

#include "../util/Vector4.hpp"

#include "Shader.hpp"



namespace morda{



/**
 * @brief Abstract shader with uniform coloring feature.
 * The uniform for color should be 'vec4' and named 'uniformColor'.
 */
class ColorShader : virtual public Shader{

	//no copying
	ColorShader(const ColorShader&);
	ColorShader& operator=(const ColorShader&);

	GLint colorUniform;

protected:
	ColorShader(){
		this->colorUniform = this->GetUniform("uniformColor");
	}

public:
	void SetColor(std::uint32_t color){
		this->SetColor(
				float(color & 0xff) / float(0xff),
				float((color >> 8) & 0xff) / float(0xff),
				float((color >> 16) & 0xff) / float(0xff),
				float((color >> 24) & 0xff) / float(0xff)
			);
	}
	
	void SetColor(morda::Vec3f color){
		glUniform4f(this->colorUniform, color.x, color.y, color.z, 1.0f);
		ASSERT(glGetError() == GL_NO_ERROR)
    }

	void SetColor(morda::Vec3f color, float alpha){
		glUniform4f(this->colorUniform, color.x, color.y, color.z, alpha);
		ASSERT(glGetError() == GL_NO_ERROR)
    }
	
	void SetColor(float r, float g, float b, float a){
		glUniform4f(this->colorUniform, r, g, b, a);
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	void SetColor(const morda::Vec4f& color){
		static_assert(sizeof(color) == sizeof(GLfloat) * 4, "size mismatch");
		glUniform4fv(this->colorUniform, 1, reinterpret_cast<const GLfloat*>(&color));
		ASSERT(glGetError() == GL_NO_ERROR)
    }
};//~class SingleColoringShader



}//~namespace