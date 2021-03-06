/* 
   Sesc: Super ESCalar simulator
   Copyright (C) 2003 University of Illinois.

   Contributed by Karin Strauss

This file is part of SESC.

SESC is free software; you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation;
either version 2, or (at your option) any later version.

SESC is    distributed in the  hope that  it will  be  useful, but  WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should  have received a copy of  the GNU General  Public License along with
SESC; see the file COPYING.  If not, write to the  Free Software Foundation, 59
Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/ 

#ifndef SMPCACHESTATE_H
#define SMPCACHESTATE_H

#include "CacheCore.h"

// these states are the most basic ones you can have
// all classes that inherit from this class should 
// have at least the following states and bits, with the same encoding

enum SMPState_t {
  SMP_INVALID       = 0x00000000, // data is invalid
  SMP_TRANS_BIT     = 0x10000000, // all transient states start with 0x1
  SMP_TRANS_RSV     = 0x14000000, // newly allocated line, reserved
  SMP_TRANS_INV     = 0x18000000, // used while invalidating upper levels
  SMP_TRANS_INV_D   = 0x18000001, // used while invalidating upper levels,
                                  // and data is dirty (needs writeback)

  SMP_INV_BIT       = 0x08000000, // data is in the process of being invalidated
  SMP_VALID_BIT     = 0x00100000, // all valid states start with 0x001

  // other masks 
  SMP_DIRTY_BIT     = 0x00000001, // data is different from memory
  SMP_READABLE_BIT  = 0x00100000, // has permission to be read
  
  //#ifdef TLS
  //	SMP_WRITEABLE_BIT = 0x00300000, // has permission to be written
  //#else
  	SMP_WRITEABLE_BIT = 0x00200000, // has permission to be written
 // #endif
  
};

class SMPCacheState : public StateGeneric<> {

private:
protected:
  uint32_t state;
#if (defined TLS)
  tls::Epoch *epoch;
  typedef tls::CacheFlags CFlags;
  CFlags *cacheFlags;
#endif
public:
 #ifdef TLS
	//cacheFlags->clearFlags();
  	tls::Epoch * getEpoch(void)
  	{
  		return epoch;
  	}
  	
  	void setEpoch(tls::Epoch *newepoch)
  	{
  		epoch=newepoch;
  	}
    tls::CacheFlags * getCacheFlags()
  	{
  		return cacheFlags;
  	}
  	
//#ifdef TLS
//template<class State, class Addr_t, bool Energy>
//typename CacheAssoc<State, Addr_t, Energy>::Line *CacheAssoc<State, Addr_t, Energy>::getLine(long int32_t index)
//{
//	Line *line =content[index];
//	return line;
//}
//template<class State, class Addr_t, bool Energy>
//typename CacheDM<State, Addr_t, Energy>::Line *CacheDM<State, Addr_t, Energy>::getLine(long int32_t index)
//{
//	Line *line =content[index];
//	return line;
//}
//#endif
  	
#endif
  SMPCacheState() 
      : StateGeneric<>()
    {
      #if (defined TLS)
    	epoch=0;
   		cacheFlags= new tls::CacheFlags;
   		cacheFlags->clearFlags();
        if (SescConf->getInt("TLS","blksize")!=32 || SescConf->getInt("TLS","wordsize")!=4)
		{
			printf("Change CacheFlag default template settings to match config\n");
			I(0);
		}        
        
  	  #endif
      state = SMP_INVALID;
    }

    // BEGIN CacheCore interface 
    bool isValid() {
      return (state != SMP_INVALID);
    }

    void invalidate() {
      // cannot invalidate if line is in transient state,
      // except when this is the end of an invalidate chain
 //     GI(isLocked(), (state & SMP_TRANS_BIT) && (state & SMP_INV_BIT));
  #ifdef TLS
	if (epoch!=0)
	{
		//printf("Non zero Epoch\n");
		epoch=0;
	}
  #endif   
      clearTag();
      state = SMP_INVALID;
    }
    
    bool isLocked() const {
      return (state & SMP_TRANS_BIT);
    }

    // END CacheCore interface
 
    unsigned getState() {
      return state;
    }

    void changeStateTo(unsigned newstate) {

      // not supposed to invalidate through this interface
      I(newstate != SMP_INVALID);

      state = newstate;
    }

    // all these functions rely on the fact that 
    // the rules described above are followed by all protocols
    bool isDirty() {
      return (state & SMP_DIRTY_BIT);
    }

    bool canBeRead() {
      return (state & SMP_READABLE_BIT);
    }

    bool canBeWritten() {
      return (state & SMP_WRITEABLE_BIT);
    }
};

#endif //SMPCACHESTATE_H
