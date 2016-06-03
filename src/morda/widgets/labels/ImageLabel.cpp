#include "ImageLabel.hpp"

#include "../../App.hpp"

#include "../../util/util.hpp"


using namespace morda;


ImageLabel::ImageLabel(const stob::Node* chain) :
		Widget(chain)
{
	if(auto image = getProperty(chain, "image")){
		this->img = App::inst().resMan.load<ResImage>(image->value());
		this->resize(this->img->dim());
	}
	
	if(auto n = getProperty(chain, "keepAspectRatio")){
		this->keepAspectRatio = n->asBool();
	}else{
		this->keepAspectRatio = false;
	}
	
	if(auto n = getProperty(chain, "repeatX")){
		this->repeat_v.x = n->asBool();
	}else{
		this->repeat_v.x = false;
	}
	
	if(auto n = getProperty(chain, "repeatY")){
		this->repeat_v.y = n->asBool();
	}else{
		this->repeat_v.y = false;
	}
}

void ImageLabel::render(const morda::Matr4r& matrix) const{
	if(!this->img){
		return;
	}

	morda::PosTexShader &s = App::inst().shaders().posTexShader;
	
	if(!this->scaledImage){
		this->scaledImage = this->img->get(this->rect().d);

		if(this->repeat_v.x || this->repeat_v.y){
			ASSERT(PosTexShader::quadFanTexCoords.size() == this->texCoords.size())
			auto src = PosTexShader::quadFanTexCoords.cbegin();
			auto dst = this->texCoords.begin();
			auto scale = this->rect().d.compDiv(this->img->dim());
			if(!this->repeat_v.x){
				scale.x = 1;
			}
			if(!this->repeat_v.y){
				scale.y = 1;
			}
			for(; dst != this->texCoords.end(); ++src, ++dst){
				*dst = src->compMul(scale);
			}
		}else{
			this->texCoords = PosTexShader::quadFanTexCoords;
		}
	}
	ASSERT(this->scaledImage)

	morda::Matr4r matr(matrix);
	matr.scale(this->rect().d);

	this->scaledImage->render(matr, s, this->texCoords);
}

morda::Vec2r ImageLabel::measure(const morda::Vec2r& quotum)const{
	if(!this->img){
		return Vec2r(0);
	}
	
	Vec2r imgDim = this->img->dim(morda::App::inst().units.dpi());
	
	ASSERT(imgDim.isPositive())
	
	if(!keepAspectRatio){
		Vec2r ret = imgDim;
		
		for(unsigned i = 0; i != ret.size(); ++i){
			if(quotum[i] >= 0){
				ret[i] = quotum[i];
			}
		}

		return ret;
	}
	
	ASSERT(this->img)
	ASSERT(imgDim.y > 0)
	
	real ratio = imgDim.x / imgDim.y;
	
	if(quotum.x < 0 && quotum.y < 0){
		return imgDim;
	}else if(quotum.x < 0){
		ASSERT(quotum.y >= 0)
		
		Vec2r ret;
		ret.y = quotum.y;
		ret.x = ratio * quotum.y;
		return ret;
	}else{
		ASSERT(quotum.x >= 0)
		ASSERT(quotum.y < 0)
		
		ASSERT(ratio > 0)
		
		Vec2r ret;
		ret.x = quotum.x;
		ret.y = quotum.x / ratio;
		return ret;
	}
}



void ImageLabel::setImage(const std::shared_ptr<const ResImage>& image) {
	if(this->img && image && this->img->dim() == image->dim()){
	}else{
		this->setRelayoutNeeded();
	}
	
	this->img = image;
	this->scaledImage.reset();
}

void ImageLabel::onResize() {
	this->Widget::onResize();
	this->scaledImage.reset();
}
