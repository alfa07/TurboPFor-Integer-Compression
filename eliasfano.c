/**
    Copyright (C) powturbo 2013-2015
    GPL v2 License
  
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    - homepage : https://sites.google.com/site/powturbo/
    - github   : https://github.com/powturbo
    - twitter  : https://twitter.com/powturbo
    - email    : powturbo [_AT_] gmail [_DOT_] com
**/

//   eliasfano.c - "Integer Compression" Elias Fano
  #ifndef USIZE //------------------------------------ functions ----------------------------------------------------------------- 
#include <stdint.h>
#include <string.h>
  #ifdef __SSE2__
#include <emmintrin.h>
  #endif

#include "conf.h"
#include "bitpack.h"
#include "bitunpack.h"
#include "bitutil.h"
#include "eliasfano.h"

#define bit_t unsigned long long
#define EFE(__x,__i,__start) ((__x[__i] - __start)-(__i)*EF_INC)
 
#define PAD8(__x) ( (((__x)+8-1)/8) )

#define BITPACK bitpack
#define BITUNPACK bitunpack
#define EF_INC 1
#define EFANOENC efano1enc
#define EFANODEC efano1dec

#define USIZE 32
#include __FILE__
#undef USIZE

/*#define USIZE 16
#include __FILE__
#undef USIZE*/

#undef EF_INC
#undef EFANOENC
#undef EFANODEC

//----------
#define EF_INC 0
#define EFANOENC efanoenc
#define EFANODEC efanodec

#define USIZE 32
#include __FILE__
#undef USIZE

#define USIZE 64
#include __FILE__
#undef USIZE

/*#define USIZE 16
#include __FILE__
#undef USIZE*/

#undef BITPACK
#undef BITUNPACK

#undef EF_INC
#undef EFANOENC
#undef EFANODEC

//----------------------
#define BITPACK bitpackv
#define BITUNPACK bitunpackv
#define EF_INC 1
#define EFANOENC efano1encv
#define EFANODEC efano1decv

#define USIZE 32
#include __FILE__
#undef USIZE

/*#define USIZE 16
#include __FILE__
#undef USIZE*/

#undef EF_INC
#undef EFANOENC
#undef EFANODEC

//------------------------------------------
#define EF_INC 0
#define EFANOENC efanoencv
#define EFANODEC efanodecv

#define USIZE 32
#include __FILE__
#undef USIZE

/*#define USIZE 16
#include __FILE__
#undef USIZE*/

  #else //--------------------------------------------- implementation ---------------------------------------------------------------
#define uint_t TEMPLATE3(uint, USIZE, _t)
#pragma clang diagnostic push 
#pragma clang diagnostic ignored "-Wparentheses"

unsigned char *TEMPLATE2(EFANOENC, USIZE)(uint_t *__restrict in, unsigned n, unsigned char *__restrict out, uint_t start) {
  if(!n) return out; 
  uint_t *ip, e = EFE(in,n-1,start);					 		
  if(!e) { out[0] = 0; return out+1; }	
  
  unsigned char *op;
  unsigned lb = TEMPLATE2(bsr, USIZE)(e/n); uint_t x = ((uint_t)1 << lb)-1, hl = PAD8((e>>lb)+n),i; 
  
  uint_t pa[n+64];  					
  for(i = 0; i != n&~3;) {
    pa[i] = EFE(in,i,start) & x; ++i;
    pa[i] = EFE(in,i,start) & x; ++i;
    pa[i] = EFE(in,i,start) & x; ++i;
    pa[i] = EFE(in,i,start) & x; ++i;
  }
  while(i < n) pa[i] = EFE(in,i,start) & x, ++i;
  *out = lb+1; op = TEMPLATE2(BITPACK,USIZE)(pa, n, out+1, lb);
  
  memset(op, 0, hl);
  for(i = 0; i != n&~3; ) {
    x = i + (EFE(in,i,start) >> lb), op[x >> 3] |= 1u << (x & 7); ++i;
    x = i + (EFE(in,i,start) >> lb), op[x >> 3] |= 1u << (x & 7); ++i;
    x = i + (EFE(in,i,start) >> lb), op[x >> 3] |= 1u << (x & 7); ++i;
    x = i + (EFE(in,i,start) >> lb), op[x >> 3] |= 1u << (x & 7); ++i;
  }
  while(i < n) x = i + (EFE(in,i,start) >> lb), op[x >> 3] |= (uint_t)1 << (x & 7),++i;
  return op+hl;
}

unsigned char *TEMPLATE2(EFANODEC, USIZE)(unsigned char *__restrict in, unsigned n, uint_t *__restrict out, uint_t start) {
  if(!n) return in;
  unsigned char *ip = in;  																										
  unsigned      i,j,lb   = *ip++;           	
  bit_t         b; 
   
  if(!lb) { 
      #if defined(__SSE2__) && USIZE == 32
        #if EF_INC == 1
    BITFORZERO(out, n, start, 1);
        #else
    BITZERO( out, n, start);
        #endif
	  #else 
    _BITFORZERO(out, n, start, EF_INC);
      #endif
	return ip; 
  }
  
  ip = TEMPLATE2(BITUNPACK,USIZE)(ip, n, out, --lb);
  for(i=j=0;; j += sizeof(bit_t)*8) 
    for(b = *(bit_t *)(ip+(j>>3)); b; b &= b-1) { 
	  out[i] = ((uint_t)(j+__builtin_ctzll(b)-i) << lb | out[i]) + start+i*EF_INC; 
	  if(unlikely(++i >= n)) 
	    return ip + PAD8((EFE(out,n-1,start)>>lb)+n); 
	}
}
#pragma clang diagnostic pop
  #endif
