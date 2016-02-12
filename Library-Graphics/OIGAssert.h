/*
-----------------------------------------------------------------------------
File:        OIGAssert.h
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
#ifndef ASSERT_H
#define ASSERT_H

#ifdef _DEBUG
	#include <cassert>

    #define ASSERT_PREDICATE(p)     assert(p);

    #define ASSERT_RETURN(p)        assert(p); \
        if (p==0)\
        return;
    #define ASSERT_RETURN_ZERO(p)	  assert(p); \
        if (p==0)\
        return 0;

    #define ASSERT_PREDICATE_RETURN(p)	    assert((p)); \
        if ((p)==false)\
        return;

    #define ASSERT_PREDICATE_RETURN_ZERO(p)	  assert((p)); \
        if ((p)==false)\
        return 0;

    #define ASSERT_PREDICATE_RETURN_FALSE(p)  assert((p)); \
        if ((p)==false)\
        return false;

    #define ASSERT_PREDICATE_RETURN_TRUE(p)	  assert((p)); \
        if ((p)==false)\
        return true;

    #define ASSERT_RETURN_EMPTY_STRING(p)	  assert(p); \
        if (p==0)\
        return "";

    #define ASSERT_RETURN_FALSE(p)	  assert(p); \
        if (p==0)\
        return false;
    #define ASSERT_RETURN_TRUE(p)	  assert(p); \
        if (p==0)\
        return true;

    #define ASSERT_EQUAL_RETURN_ZERO(x,y)   assert(x==y);\
        if (x!=y)   \
        return 0;

#else 
	#ifdef ASSERT_IN_RELEASE
		// undefine the ndebug first
		#ifdef NDEBUG
			#undef NDEBUG
			#define RESET_NDEBUG
		#endif

		// do the assert 
        #include <cassert>

        #define ASSERT_PREDICATE(p)     assert(p);

        #define ASSERT_RETURN(p)        assert(p); \
            if (p==0)\
            return;
        #define ASSERT_RETURN_ZERO(p)	  assert(p); \
            if (p==0)\
            return 0;

        #define ASSERT_PREDICATE_RETURN(p)	    assert((p)); \
            if ((p)==false)\
            return;

        #define ASSERT_PREDICATE_RETURN_ZERO(p)	  assert((p)); \
            if ((p)==false)\
            return 0;

        #define ASSERT_PREDICATE_RETURN_FALSE(p)  assert((p)); \
            if ((p)==false)\
            return false;

        #define ASSERT_PREDICATE_RETURN_TRUE(p)	  assert((p)); \
            if ((p)==false)\
            return true;

        #define ASSERT_RETURN_EMPTY_STRING(p)	  assert(p); \
            if (p==0)\
            return "";

        #define ASSERT_RETURN_FALSE(p)	  assert(p); \
            if (p==0)\
            return false;
        #define ASSERT_RETURN_TRUE(p)	  assert(p); \
            if (p==0)\
            return true;

        #define ASSERT_EQUAL_RETURN_ZERO(x,y)   assert(x==y);\
            if (x!=y)   \
            return 0;

		//redefine ndebug if it has been defined previously
		#ifdef RESET_NDEBUG
		#undef RESET_NDEBUG
		#define NDEBUG
		#endif
	#else
        #define ASSERT_PREDICATE(p)                 static_cast<void>(0);
        #define ASSERT_RETURN(p)                    static_cast<void>(0);
        #define ASSERT_RETURN_ZERO(p)               static_cast<void>(0);
        #define ASSERT_PREDICATE_RETURN(p)          static_cast<void>(0);
        #define ASSERT_PREDICATE_RETURN_ZERO(p)	    static_cast<void>(0);
        #define ASSERT_PREDICATE_RETURN_FALSE(p)    static_cast<void>(0);
        #define ASSERT_PREDICATE_RETURN_TRUE(p)	    static_cast<void>(0);
        #define ASSERT_RETURN_EMPTY_STRING(p)	    static_cast<void>(0);
        #define ASSERT_RETURN_FALSE(p)	            static_cast<void>(0);
        #define ASSERT_RETURN_TRUE(p)               static_cast<void>(0);
        #define ASSERT_EQUAL_RETURN_ZERO(x,y)       static_cast<void>(0);
	#endif
#endif

  
;

#endif
