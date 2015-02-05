/* The MIT License:

Copyright (c) 2015 Ivan Gagis <igagis@gmail.com>

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

#include "Widget.hpp"

namespace morda{

class List : public virtual Widget{
protected:
	List(bool isVertical, const stob::Node* chain = nullptr);
public:
	List(const List&) = delete;
	List& operator=(const List&) = delete;
	
	class ItemsProvider : virtual public ting::Shared{
	protected:
		ItemsProvider(){}
	public:
		virtual ~ItemsProvider()NOEXCEPT{}
		
		virtual size_t count()const NOEXCEPT = 0;
		
		virtual std::shared_ptr<Widget> getWidget(size_t index) = 0;
		
		virtual void recycle(std::shared_ptr<Widget> w){}
	};
	
	void notifyDataSetChanged();
	
	void setItemsProvider(std::shared_ptr<ItemsProvider> provider){
		this->provider = std::move(provider);
		this->notifyDataSetChanged();
	}
	
private:
	std::shared_ptr<ItemsProvider> provider;
};

}
