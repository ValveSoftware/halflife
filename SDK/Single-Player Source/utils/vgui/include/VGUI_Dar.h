//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#ifndef VGUI_DAR_H
#define VGUI_DAR_H

#include<stdlib.h>
#include<string.h>
#include<VGUI.h>



namespace vgui
{

//Simple lightweight dynamic array implementation
template<class ELEMTYPE> class VGUIAPI Dar
{
public:
	Dar()
	{
		_count=0;
		_capacity=0;
		_data=null;
		ensureCapacity(4);
	}
	Dar(int initialCapacity)
	{
		_count=0;
		_capacity=0;
		_data=null;
		ensureCapacity(initialCapacity);
	}
public:
	void ensureCapacity(int wantedCapacity)
	{
		if(wantedCapacity<=_capacity){return;}

		//double capacity until it is >= wantedCapacity
		//this could be done with math, but iterative is just so much more fun
		int newCapacity=_capacity;
		if(newCapacity==0){newCapacity=1;}
		while(newCapacity<wantedCapacity){newCapacity*=2;}

		//allocate and zero newData
		ELEMTYPE* newData=new ELEMTYPE[newCapacity]; 
		if(newData==null){exit(0);return;}
		memset(newData,0,sizeof(ELEMTYPE)*newCapacity);
		_capacity=newCapacity;
   
		//copy data into newData
		for(int i=0;i<_count;i++){newData[i]=_data[i];}

		delete[] _data;
		_data=newData;
	}
	void setCount(int count)
	{
		if((count<0)||(count>_capacity))
		{
			return;
		}
		_count=count;
	}
	int getCount()
	{
		return _count;
	}
	void addElement(ELEMTYPE elem)
	{
		ensureCapacity(_count+1);
		_data[_count]=elem;
		_count++;
	}
	bool hasElement(ELEMTYPE elem)
	{
		for(int i=0;i<_count;i++)
		{
			if(_data[i]==elem)
			{
				return true;
			}
		}
	return false;
	}
	void putElement(ELEMTYPE elem)
	{
		if(hasElement(elem))
		{
			return;
		}
		addElement(elem);
	}
	void insertElementAt(ELEMTYPE elem,int index)
	{
		if((index<0)||(index>_count))
		{
			return;
		}
		if((index==_count)||(_count==0))
		{
			addElement(elem);
		}
		else
		{
			addElement(elem); //just to make sure it is big enough
			for(int i=_count-1;i>index;i--)
			{
				_data[i]=_data[i-1];
			}
			_data[index]=elem;
		}
	}
	void setElementAt(ELEMTYPE elem,int index)
	{
		if((index<0)||(index>=_count))
		{
			return;
		}
		_data[index]=elem;
	}
	void removeElementAt(int index)
	{
		if((index<0)||(index>=_count))
		{
			return;
		}
   
		//slide everything to the right of index, left one.
		for(int i=index;i<(_count-1);i++)
		{
			_data[i]=_data[i+1];
		}
		_count--;
	} 
	void removeElement(ELEMTYPE elem)
	{
		for(int i=0;i<_count;i++)
		{
			if(_data[i]==elem)
			{
				removeElementAt(i);
				break;
			}
		}
	}
	void removeAll()
	{
		_count=0;
	}
	ELEMTYPE operator[](int index)
	{
		if((index<0)||(index>=_count))
		{
			return null;
		}
		return _data[index];
	}
protected:
	int       _count;
	int       _capacity;
	ELEMTYPE* _data;
};

//forward referencing all the template types used so they get exported
template class VGUIAPI Dar<char>;
template class VGUIAPI Dar<char*>;
template class VGUIAPI Dar<int>;
template class VGUIAPI Dar<class Button*>;
template class VGUIAPI Dar<class SurfaceBase*>;
template class VGUIAPI Dar<class InputSignal*>;
template class VGUIAPI Dar<class FocusChangeSignal*>;
template class VGUIAPI Dar<class FrameSignal*>;
template class VGUIAPI Dar<class ActionSignal*>;
template class VGUIAPI Dar<class IntChangeSignal*>;
template class VGUIAPI Dar<class TickSignal*>;
template class VGUIAPI Dar<class Dar<char>*>;
template class VGUIAPI Dar<class Frame*>;
template class VGUIAPI Dar<class DesktopIcon*>;
template class VGUIAPI Dar<class ChangeSignal*>;
template class VGUIAPI Dar<class Panel*>;
template class VGUIAPI Dar<class Label*>;
template class VGUIAPI Dar<class RepaintSignal*>;


}


#endif