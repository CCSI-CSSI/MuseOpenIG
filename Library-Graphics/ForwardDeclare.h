/*
-----------------------------------------------------------------------------
File:        ForwardDeclare.h
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
#ifndef FORWARDDECLARE_H
#define FORWARDDECLARE_H

#include <list>
#include <map>
#include <vector>
#include <deque>

#define FWD_DECLARE_USE_CPLUS_PLUS_UNORDERED_CONTAINERS 0

#if FWD_DECLARE_USE_CPLUS_PLUS_UNORDERED_CONTAINERS
#include <unordered_map>
#include <unordered_set>
#else
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#endif

#include <string>

#if FWD_DECLARE_USE_CPLUS_PLUS_UNORDERED_CONTAINERS

#define FORWARD_DECLARE(T) namespace OpenIG { namespace Library { namespace Graphics {\
                           class T;\
						   typedef std::list<T*> List##T##s; \
						   typedef std::vector<T*> Vector##T##s; \
						   typedef std::map<std::string, T*> Map##T##s; \
						   typedef std::list<const T*> ConstList##T##s; \
						   typedef std::vector<const T*> ConstVector##T##s; \
						   typedef std::map<std::string, const T*> ConstMap##T##s; \
                           typedef std::unordered_set<T*> UnorderedSet##T##s; \
                           typedef std::unordered_map<std::string, T*> UnorderedMap##T##s; \
                           typedef std::unordered_set<const T*> ConstUnorderedSet##T##s; \
                           typedef std::unordered_map<std::string, const T*> ConstUnorderedMap##T##s; \
						   }}}

#define FORWARD_DECLARE_ALT_PLURAL(T) namespace OpenIG { namespace Library { namespace Graphics {\
                            class T;\
							typedef std::list<T*> List##T##es; \
							typedef std::vector<T*> Vector##T##es; \
							typedef std::map<std::string, T*> Map##T##es; \
							typedef std::list<const T*> ConstList##T##es; \
							typedef std::vector<const T*> ConstVector##T##es; \
							typedef std::map<std::string, const T*> ConstMap##T##es; \
                            typedef std::unordered_set<T*> UnorderedSet##T##es; \
                            typedef std::unordered_map<std::string, T*> UnorderedMap##T##es; \
                            typedef std::unordered_set<const T*> ConstUnorderedSet##T##es; \
                            typedef std::unordered_map<std::string, const T*> ConstUnorderedMap##T##es; \
							}}}
#else

#define FORWARD_DECLARE(T) namespace OpenIG { namespace Library { namespace Graphics {\
                           class T;\
						   typedef std::list<T*> List##T##s; \
						   typedef std::vector<T*> Vector##T##s; \
						   typedef std::deque<T*> Deque##T##s; \
						   typedef std::map<std::string, T*> Map##T##s; \
						   typedef std::list<const T*> ConstList##T##s; \
						   typedef std::vector<const T*> ConstVector##T##s; \
						   typedef std::deque<const T*> ConstDeque##T##s; \
						   typedef std::map<std::string, const T*> ConstMap##T##s; \
                           typedef boost::unordered_set<T*> UnorderedSet##T##s; \
                           typedef boost::unordered_map<std::string, T*> UnorderedMap##T##s; \
                           typedef boost::unordered_set<const T*> ConstUnorderedSet##T##s; \
                           typedef boost::unordered_map<std::string, const T*> ConstUnorderedMap##T##s; \
						   }}}

#define FORWARD_DECLARE_ALT_PLURAL(T) namespace OpenIG { namespace Library { namespace Graphics {\
                            class T;\
							typedef std::list<T*> List##T##es; \
							typedef std::vector<T*> Vector##T##es; \
							typedef std::map<std::string, T*> Map##T##es; \
							typedef std::list<const T*> ConstList##T##es; \
							typedef std::vector<const T*> ConstVector##T##es; \
							typedef std::map<std::string, const T*> ConstMap##T##es; \
                            typedef boost::unordered_set<T*> UnorderedSet##T##es; \
                            typedef boost::unordered_map<std::string, T*> UnorderedMap##T##es; \
                            typedef boost::unordered_set<const T*> ConstUnorderedSet##T##es; \
                            typedef boost::unordered_map<std::string, const T*> ConstUnorderedMap##T##es; \
							}}}

#endif

#endif
