#include "Label.hpp"

#include <ting/math.hpp>

#include "../../App.hpp"



using namespace morda;



Label::Label(const stob::Node* chain) :
		Widget(chain),
		SingleLineTextWidget(chain)
{}




//override
void Label::render(const morda::Matr4r& matrix)const{
	morda::Matr4r matr(matrix);
	matr.Translate(-this->textBoundingBox().p.x, -this->Font().BoundingBox().p.y);
	
	PosTexShader& s = [this]() -> PosTexShader&{
		if(this->color() == 0xffffffff){//if white
			return morda::App::Inst().Shaders().posTexShader;
		}else{
			ColorPosTexShader& s = morda::App::Inst().Shaders().colorPosTexShader;

			s.SetColor(this->color());
			return s;
		}
	}();
	
	this->Font().RenderString(s, matr, this->text());
}
