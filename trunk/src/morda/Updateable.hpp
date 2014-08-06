/* The MIT License:

Copyright (c) 2012 Ivan Gagis <igagis@gmail.com>

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


#include <ting/Shared.hpp>

#include "Exc.hpp"



namespace morda{



class Updateable : virtual public ting::Shared{
	friend class App;
	
private:
	class Updater{
		friend class morda::Updateable;
		
		typedef std::pair<ting::u32, ting::WeakRef<morda::Updateable> > T_Pair;
		
		class UpdateQueue : public std::list<T_Pair>{
		public:
			UpdateQueue::iterator Insert(const T_Pair& p);

			inline ting::Ref<morda::Updateable> PopFront(){
				ting::Ref<morda::Updateable> ret = this->front().second;
				this->pop_front();
				return ret;
			}
		};
		
		UpdateQueue q1, q2;
		
		UpdateQueue *activeQueue, *inactiveQueue;
		
		ting::Inited<ting::u32, 0> lastUpdatedTimestamp;
		
		typedef std::list<ting::Ref<morda::Updateable> > T_ToAddList;
		T_ToAddList toAdd;
		
		void AddPending();
		
		void UpdateUpdateable(const ting::Ref<morda::Updateable>& u);
	public:
		Updater() :
				activeQueue(&q1),
				inactiveQueue(&q2)
		{}
		
		void RemoveFromToAdd(Updateable* u);
		
		//returns dt to wait before next update
		ting::u32 Update();
	};
	
private:
	ting::u16 dt;
	
	ting::u32 startedAt; //timestamp when update timer started.
	
	inline ting::u32 EndAt()const throw(){
		return this->startedAt + ting::u32(this->dt);
	}
	
	ting::Inited<bool, false> isUpdating;
	
	//pointer to the queue the updateable is inserted into
	ting::Inited<Updater::UpdateQueue*, 0> queue;
	
	Updater::UpdateQueue::iterator iter; //iterator into the queue.
	
	ting::Inited<bool, false> pendingAddition;
	
public:
	class Exc : public morda::Exc{
	public:
		Exc(const std::string& message) :
				morda::Exc(message)
		{}
	};
	
	~Updateable()throw(){}
	
	bool IsUpdating()const throw(){
		return this->isUpdating;
	}
	
	void StartUpdating(ting::u16 dt = 30);
	
	void StopUpdating()throw();
	
	virtual void Update(ting::u32 dt) = 0;
};

}//~namespace
