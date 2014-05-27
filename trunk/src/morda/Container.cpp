#include "Container.hpp"

#include "App.hpp"

#include "util/util.hpp"



using namespace morda;



void Container::ApplyDescription(const stob::Node& description){
	if(const stob::Node* n = description.GetProperty("layout")){
		this->SetLayout(morda::App::Inst().Inflater().CreateLayout(*n));
	}
	
	for(const stob::Node* n = description.Child(); n; n = n->Next()){
		if(n->IsProperty()){
			continue;//skip properties
		}
		
		this->Add(morda::App::Inst().Inflater().Inflate(*n));
	}
}



//override
void Container::Render(const morda::Matr4f& matrix)const{
	for(const ting::Ref<Widget>* c = &this->childrenHead; *c; c = &(*c)->Next()){
		if((*c)->IsHidden()){
			continue;
		}
		
		morda::Matr4f matr(matrix);
		matr.Translate((*c)->Rect().p);

		(*c)->RenderInternal(matr);
	}
}



//override
bool Container::OnMouseButton(bool isDown, const morda::Vec2f& pos, EMouseButton button, unsigned pointerId){
//	TRACE(<< "Container::OnMouseButton(): isDow = " << isDown << ", button = " << button << ", pos = " << pos << std::endl)
	
	//Copy children list to iterate through it later, because the original list of children
	//may change during iterating.
	//TODO:
//	T_ChildList childs;
//	childs.reserve(this->children.size());
//	for(T_ChildList::iterator i = this->children.begin(); i != this->children.end(); ++i){
//		childs.push_back(*i);
//	}
	
	//call children in reverse order
	for(const ting::Ref<Widget>* c = &this->childrenTail; *c; c = &(*c)->Prev()){
		if((*c)->IsHidden() || (*c)->IsDisabled()){
			continue;
		}
		
		if(!(*c)->Rect().Overlaps(pos)){
			continue;
		}
		
		//Sometimes mouse click event comes without prior mouse move,
		//but, since we get mouse click, then the widget is hovered.
		(*c)->setHovered(true);
		
		morda::Vec2f localPos(pos - (*c)->Rect().p);
		
		if((*c)->OnMouseButton(isDown, localPos, button, pointerId)){
			return true;
		}
	}
	return false;
}



//override
bool Container::OnMouseMove(const morda::Vec2f& pos, unsigned pointerId){
	//Copy children list to iterate through it later, because the original list of children
	//may change during iterating.
	//TODO:
//	T_ChildList childs;
//	childs.reserve(this->children.size());
//	for(T_ChildList::iterator i = this->children.begin(); i != this->children.end(); ++i){
//		childs.push_back(*i);
//	}
	
	//call children in reverse order
	for(const ting::Ref<Widget>* c = &this->childrenTail; *c; c = &(*c)->Prev()){
		if((*c)->isHidden){
			ASSERT(!(*c)->IsHovered())
			continue;
		}
		
		if(!(*c)->Rect().Overlaps(pos)){
			(*c)->setHovered(false);
			continue;
		}
		
		(*c)->setHovered(true);
		
		if((*c)->OnMouseMove(pos - (*c)->Rect().p, pointerId)){//consumed mouse move event
			//un-hover rest of the children
			for(c = &(*c)->Prev(); *c; c = &(*c)->Prev()){
				(*c)->setHovered(false);
			}
			return true;
		}		
	}
	return false;
}



void Container::OnHoverChanged(){
	//TODO: if some child removed during iterating?
	
	if(this->IsHovered()){
		return;
	}
	
	//un-hover all the children if container became un-hovered
	for(const ting::Ref<Widget>* c = &this->childrenHead; *c; c = &(*c)->Next()){
		(*c)->setHovered(false);
	}
}



void Container::OnResize(){
//	TRACE(<< "Container::OnResize(): invoked" << std::endl)
	if(this->layout){
		this->layout->ArrangeWidgets(*this);
	}else{
		for(const ting::Ref<Widget>* c = &this->Children(); *c; c = &(*c)->Next()){
			if((*c)->NeedsRelayout()){
				(*c)->Resize((*c)->Rect().d);
			}
		}
	}
}



//override
morda::Vec2f Container::ComputeMinDim()const throw(){
	if(this->layout){
		return this->layout->ComputeMinDim(*this);
	}
	return this->Widget::ComputeMinDim();
}



void Container::Add(const ting::Ref<Widget>& w){
	ASSERT_INFO(w, "Widget::Add(): widget pointer is 0")
	if(w->parent.GetRef().IsValid()){
		throw morda::Exc("Container::Add(): cannot add widget, it is already added to some container");
	}
	ASSERT(w->next.IsNotValid())
	ASSERT(w->prev.IsNotValid())
	
	if(!this->childrenHead){
		ASSERT(!this->childrenTail)
		this->childrenHead = w;
		this->childrenTail = w;
	}else{
		this->childrenTail->next = w;
		w->prev = this->childrenTail;
		this->childrenTail = w;
	}
	
	++this->numChildren;
	
	w->parent = this;

	this->SetRelayoutNeeded();
	
	ASSERT(!w->IsHovered())
}



void Container::Remove(const ting::Ref<Widget>& w){
	ASSERT(w.IsValid())
	ASSERT(w->parent.GetRef() == ting::Ref<Container>(this))

	if(w->prev){
		w->prev->next = w->next;
	}else{
		this->childrenHead = w->next;
	}
	if(w->next){
		w->next->prev = w->prev;
	}else{
		this->childrenTail = w->prev;
	}
	
	--this->numChildren;
	
	w->parent.Reset();
	w->setHovered(false);
	
	this->SetRelayoutNeeded();
}



//override
ting::Ref<Widget> Container::FindChildByName(const std::string& name)throw(){
//	TRACE_AND_LOG(<< "Container::FindChildByName(): enter" << std::endl)
	for(const ting::Ref<Widget>* c = &this->Children(); *c; c = &(*c)->Next()){
//		TRACE_AND_LOG(<< "(*c)->Name() = " << ((*c)->Name()) << std::endl)
		if((*c)->Name() == name){
			return *c;
		}
		
		if(ting::Ref<Widget> r = (*c)->FindChildByName(name)){
			return r;
		}
	}
	return 0;
}
