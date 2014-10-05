#include "Window.hpp"

#include "../App.hpp"

#include "../util/util.hpp"


using namespace morda;


namespace{

const char* DWindowDesc = R"qwertyuiop(
		{
			//left border
			LinearContainer{
				vertical{true}
				reverse{true}
				layout{
					dim{min min}
					fill{false true}
				}
				FrameContainer{
					layout{
						dim{min min}
					}
					ImageLabel{
						image{morda_img_window_lt}
					}
					MouseProxy{
						name{morda_lt_proxy}
						layout{
							fill{true true}
						}
					}
				}
				FrameContainer{
					layout{
						weight{1}
						dim{min 0}
						fill{false true}
					}
					ImageLabel{
						image{morda_img_window_l}
						layout{
							fill{false true}
						}
					}
					MouseProxy{
						name{morda_l_proxy}
						layout{
							fill{true true}
						}
					}
				}
				FrameContainer{
					layout{
						dim{min min}
					}
					ImageLabel{
						image{morda_img_window_lb}
					}
					MouseProxy{
						name{morda_lb_proxy}
						layout{
							fill{true true}
						}
					}
				}
			}

			LinearContainer{
				vertical{true}
				reverse{true}
				layout{
					weight{1}
					dim{min min}
					fill{true true}
				}

				//top border
				FrameContainer{
					layout{
						dim{0 min}
						fill{true false}
					}
					ImageLabel{
						image{morda_img_window_t}
						layout{
							fill{true false}
						}
					}
					MouseProxy{
						name{morda_t_proxy}
						layout{
							fill{true true}
						}
					}
				}

				//caption
				FrameContainer{
					layout{
						dim{0 min}
						fill{true true}
					}

					ColorLabel{
						layout{
							dim{0 0}
							fill{true true}
						}
						color{0xffff0000}
					}

					LinearContainer{
						layout{
							dim{0 min}
							fill{true true}
						}

						Label{
							name{morda_title}
							layout{
								dim{0 min}
								gravity{0% 50%}
								weight{1}
							}
						}
					}

					MouseProxy{
						name{morda_caption_proxy}
						layout{
							fill{true true}
						}
					}
				}

				FrameContainer{
					name{morda_content}
					layout{
						dim{0 0}
						weight{1}
						fill{true true}
					}
//					color{0xff800080}
				}

				//bottom border
				FrameContainer{
					layout{
						dim{0 min}
						fill{true false}
					}
					ImageLabel{
						image{morda_img_window_b}
						layout{
							fill{true false}
						}
					}
					MouseProxy{
						name{morda_b_proxy}
						layout{
							fill{true true}
						}
					}
				}
			}

			//right border
			LinearContainer{
				vertical{true}
				reverse{true}
				layout{
					dim{min min}
					fill{false true}
				}

				FrameContainer{
					layout{
						dim{min min}
					}
					ImageLabel{
						image{morda_img_window_rt}
					}
					MouseProxy{
						name{morda_rt_proxy}
						layout{
							fill{true true}
						}
					}
				}

				FrameContainer{
					layout{
						weight{1}
						dim{min 0}
						fill{false true}
					}

					ImageLabel{
						image{morda_img_window_r}
						layout{
							fill{false true}
						}
					}
					MouseProxy{
						name{morda_r_proxy}
						layout{
							fill{true true}
						}
					}
				}

				FrameContainer{
					layout{
						dim{min min}
					}
					ImageLabel{
						image{morda_img_window_rb}
					}
					MouseProxy{
						name{morda_rb_proxy}
						layout{
							fill{true true}
						}
					}
				}
			}
		}
	)qwertyuiop";

}



morda::Window::Window(const stob::Node* desc) :
		Widget(desc),
		LinearContainer(stob::Parse(DWindowDesc).get()),
		emptyMinDim(this->LinearContainer::ComputeMinDim())
{
	this->SetupWidgets();
	
	this->contentArea->SetClip(this->IsClip());
	this->SetClip(false);
	
	if(const stob::Node* n = GetProperty(desc, "title")){
		this->SetTitle(n->Value());
	}
	
	if(desc){
		this->contentArea->Add(*desc);
	}
}

void morda::Window::SetupWidgets(){
	this->contentArea = this->FindChildByNameAs<FrameContainer>("morda_content");
	ASSERT(this->contentArea)
	
	std::function<decltype(MouseProxy::onMouseButton)(bool&)> getButtonFunc = [this](bool& flag){
		return decltype(MouseProxy::onMouseButton)([this, &flag](Widget& widget, bool isDown, const morda::Vec2r& pos, EMouseButton button, unsigned pointerId){
			if(button != Widget::EMouseButton::LEFT){
				return false;
			}

			if(isDown){
				flag = true;
				this->capturePoint = pos;
				return true;
			}
			flag = false;
			return false;
		});
	};
	
	{
		auto caption = this->FindChildByNameAs<MouseProxy>("morda_caption_proxy");
		ASSERT(caption)
	
		caption->onMouseButton = getButtonFunc(this->captionCaptured);

		caption->onMouseMove = [this](Widget& widget, const morda::Vec2r& pos, unsigned pointerId){
			if(this->captionCaptured){
				this->MoveBy(pos - this->capturePoint);
				return true;
			}
			return false;
		};
	}
	
	this->title = this->FindChildByNameAs<Label>("morda_title");
	ASSERT(this->title)
	
	{
		auto w = this->FindChildByNameAs<MouseProxy>("morda_lt_proxy");
		ASSERT(w)
		w->onMouseButton = getButtonFunc(this->leftTopResizeCaptured);
		w->onMouseMove = [this](Widget& widget, const morda::Vec2r& pos, unsigned pointerId){
			if(this->leftTopResizeCaptured){
				morda::Vec2r d = pos - this->capturePoint;
				ting::util::ClampTop(d.x, this->Rect().d.x - this->emptyMinDim.x);
				ting::util::ClampBottom(d.y, -(this->Rect().d.y - this->emptyMinDim.y));
				this->MoveBy(morda::Vec2r(d.x, 0));
				this->ResizeBy(morda::Vec2r(-d.x, d.y));
			}
			return false;
		};
	}
	
	{	
		auto w = this->FindChildByNameAs<MouseProxy>("morda_lb_proxy");
		ASSERT(w)
		w->onMouseButton = getButtonFunc(this->leftBottomResizeCaptured);
		w->onMouseMove = [this](Widget& widget, const morda::Vec2r& pos, unsigned pointerId){
			if(this->leftBottomResizeCaptured){
				morda::Vec2r d = pos - this->capturePoint;
				ting::util::ClampTop(d.x, this->Rect().d.x - this->emptyMinDim.x);
				ting::util::ClampTop(d.y, this->Rect().d.y - this->emptyMinDim.y);
				this->MoveBy(d);
				this->ResizeBy(morda::Vec2r(-d.x, -d.y));
			}
			return false;
		};
	}

	{
		auto w = this->FindChildByNameAs<MouseProxy>("morda_rt_proxy");
		ASSERT(w)
		w->onMouseButton = getButtonFunc(this->rightTopResizeCaptured);
		w->onMouseMove = [this](Widget& widget, const morda::Vec2r& pos, unsigned pointerId){
			if(this->rightTopResizeCaptured){
				morda::Vec2r d = pos - this->capturePoint;
				ting::util::ClampBottom(d.x, -(this->Rect().d.x - this->emptyMinDim.x));
				ting::util::ClampBottom(d.y, -(this->Rect().d.y - this->emptyMinDim.y));
				this->ResizeBy(d);
			}
			return false;
		};
	}
	
	{
		auto w = this->FindChildByNameAs<MouseProxy>("morda_rb_proxy");
		ASSERT(w)
		w->onMouseButton = getButtonFunc(this->rightBottomResizeCaptured);
		w->onMouseMove = [this](Widget& widget, const morda::Vec2r& pos, unsigned pointerId){
			if(this->rightBottomResizeCaptured){
				morda::Vec2r d = pos - this->capturePoint;
				ting::util::ClampBottom(d.x, -(this->Rect().d.x - this->emptyMinDim.x));
				ting::util::ClampTop(d.y, this->Rect().d.y - this->emptyMinDim.y);
				this->MoveBy(morda::Vec2r(0, d.y));
				this->ResizeBy(morda::Vec2r(d.x, -d.y));
			}
			return false;
		};
	}
	
	{
		auto w = this->FindChildByNameAs<MouseProxy>("morda_l_proxy");
		ASSERT(w)
		w->onMouseButton = getButtonFunc(this->leftResizeCaptured);
		w->onMouseMove = [this](Widget& widget, const morda::Vec2r& pos, unsigned pointerId){
			if(this->leftResizeCaptured){
				morda::Vec2r d = pos - this->capturePoint;
				ting::util::ClampTop(d.x, this->Rect().d.x - this->emptyMinDim.x);
				this->MoveBy(morda::Vec2r(d.x, 0));
				this->ResizeBy(morda::Vec2r(-d.x, 0));
			}
			return false;
		};
	}
	
	{
		auto w = this->FindChildByNameAs<MouseProxy>("morda_r_proxy");
		ASSERT(w)
		w->onMouseButton = getButtonFunc(this->rightResizeCaptured);
		w->onMouseMove = [this](Widget& widget, const morda::Vec2r& pos, unsigned pointerId){
			if(this->rightResizeCaptured){
				morda::Vec2r d = pos - this->capturePoint;
				ting::util::ClampBottom(d.x, -(this->Rect().d.x - this->emptyMinDim.x));
				this->ResizeBy(morda::Vec2r(d.x, 0));
			}
			return false;
		};
	}
	
	{
		auto w = this->FindChildByNameAs<MouseProxy>("morda_t_proxy");
		ASSERT(w)
		w->onMouseButton = getButtonFunc(this->topResizeCaptured);
		w->onMouseMove = [this](Widget& widget, const morda::Vec2r& pos, unsigned pointerId){
			if(this->topResizeCaptured){
				morda::Vec2r d = pos - this->capturePoint;
				ting::util::ClampBottom(d.y, -(this->Rect().d.y - this->emptyMinDim.y));
				this->ResizeBy(morda::Vec2r(0, d.y));
			}
			return false;
		};
	}
	
	{
		auto w = this->FindChildByNameAs<MouseProxy>("morda_b_proxy");
		ASSERT(w)
		w->onMouseButton = getButtonFunc(this->bottomResizeCaptured);
		w->onMouseMove = [this](Widget& widget, const morda::Vec2r& pos, unsigned pointerId){
			if(this->bottomResizeCaptured){
				morda::Vec2r d = pos - this->capturePoint;
				ting::util::ClampTop(d.y, this->Rect().d.y - this->emptyMinDim.y);
				this->MoveBy(morda::Vec2r(0, d.y));
				this->ResizeBy(morda::Vec2r(0, -d.y));
			}
			return false;
		};
	}
}

void morda::Window::SetTitle(const std::string& str){
	this->title->SetText(ting::utf8::ToUTF32(str));
}
