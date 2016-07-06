/*
-----------------------------------------------------------------------------
File:        CommonUtils.h
Copyright:   Copyright (C) 2007-2015 Poojan Prabhu. All rights reserved.
Created:     10/20/2015
Last edit:   12/15/2015
Author:      Poojan Prabhu
E-mail:      poojanprabhu@gmail.com

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
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef COMMONUTILS_H
#define COMMONUTILS_H

#include <string>

#pragma warning( push )
#pragma warning( disable : 4005 )


#define TYPE_TO_STRING(T) std::string(#T)

#define VAR_NAME_TO_STRING(T) std::string(#T)

#define SAFE_DELETE(p) if (p) \
					   { \
					   delete p; \
					   p = 0; \
					   } \

#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }

#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }

#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return hr; } }

#define SWITCH_BEGIN(x) switch(x){
#define SWITCH_END(x) CASE_DEFAULT}
#define CASE_EXECUTE(x, y) case x: y; break;
#define CASE_EXECUTE_RETURN(x, y) case x: y; return; break;
#define CASE_RETURN(x,y) case x: return y; break;
#define CASE_DEFAULT default: ASSERT_PREDICATE(false); break;

#define OIG_UNREFERENCED_VARIABLE(x) static_cast<void>(x)

#pragma warning ( pop ) 

#endif
