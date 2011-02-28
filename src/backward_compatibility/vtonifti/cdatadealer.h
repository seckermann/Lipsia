//
// C++ Interface: cdatadealer
//
// Description:
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CDATADEALER_H
#define CDATADEALER_H

#define BUFF_SIZE (1024*1024)

#include <iostream>
/**
	@author
*/
template<class CC> class CDataDealer{
protected:
	CC &client;
	void *data_buffer;
	size_t curr_buff_size;
	CDataDealer(CC &client_):client(client_),data_buffer(NULL),curr_buff_size(0){}
	~CDataDealer(){
		if(data_buffer)resizeBuffer(0);
	}
public:
	bool swap[4],mirror[4];
	bool resizeBuffer(size_t size){
		data_buffer=realloc(data_buffer,size);
		if(data_buffer || size==0)
			curr_buff_size=size;
		else
			std::cerr << "E: could not allocate memory for buffer" << __FILE__ << ":" << __LINE__ << std::endl;
		return data_buffer!=NULL;
	}
	bool createBuffer(){
		if(data_buffer)
			std::cout << "W: buffer was already created when calling createBuffer" << std::endl;
		return resizeBuffer(BUFF_SIZE);
	}
	bool growBuffer(){
		resizeBuffer(curr_buff_size+BUFF_SIZE);
	}
	template<class T> bool checkBuffer(size_t len, T dummy){
		const size_t needed=sizeof(T)*len;
		if(curr_buff_size<needed)
			return resizeBuffer(needed);
		else return true;
	}
	virtual inline size_t getIndex(const unsigned short &x,const unsigned short &y,const unsigned short &z,const unsigned short &t)const=0;
	virtual bool continous()
	{
		return !(swap[0]||swap[1]||swap[2]||swap[3]);
	}
	virtual void* getBuffer()
	{
		if(!data_buffer)std::cerr << "E: fetching zero buffer" << __FILE__ << ":" << __LINE__ << std::endl;
		return data_buffer;
	}
	virtual size_t sizePrognosis()=0;
};

#endif
