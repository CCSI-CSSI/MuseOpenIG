/*
-----------------------------------------------------------------------------
File:        Signal.h
Copyright:   Copyright (C) 2007-2015 Poojan Prabhu. All rights reserved.
Created:     10/20/2015
Last edit:   12/15/2015
Author:      Poojan Prabhu
E-mail:      openig@compro.net

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
#pragma once

#if defined(OPENIG_SDK)
	#include <OpenIG-Graphics/Delegate.h>
#else
	#include <Library-Graphics/Delegate.h>
#endif

#include <set>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			template< class Param0 = void >
			class signal0
			{
			public:
				typedef Delegate0< void > _Delegate;

			private:
				typedef std::set<_Delegate> DelegateList;
				typedef typename DelegateList::const_iterator DelegateIterator;
				DelegateList delegateList;

			public:
				void connect(_Delegate delegate)
				{
					delegateList.insert(delegate);
				}

				template< class X, class Y >
				void connect(void (X::*func)(), Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void connect(void (X::*func)() const, Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				void disconnect(_Delegate delegate)
				{
					delegateList.erase(delegate);
				}

				template< class X, class Y >
				void disconnect(void (X::*func)(), Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void disconnect(void (X::*func)() const, Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				void clear()
				{
					delegateList.clear();
				}

				void emit() const
				{
					for (DelegateIterator i = delegateList.begin(); i != delegateList.end();)
					{
						(*(i++))();
					}
				}

				void operator() () const
				{
					emit();
				}

				bool empty() const
				{
					return delegateList.empty();
				}
			};


			template< class Param1 >
			class signal1
			{
			public:
				typedef Delegate1< Param1 > _Delegate;

			private:
				typedef std::set<_Delegate> DelegateList;
				typedef typename DelegateList::const_iterator DelegateIterator;
				DelegateList delegateList;

			public:
				void connect(_Delegate delegate)
				{
					delegateList.insert(delegate);
				}

				template< class X, class Y >
				void connect(void (X::*func)(Param1 p1), Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void connect(void (X::*func)(Param1 p1) const, Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				void disconnect(_Delegate delegate)
				{
					delegateList.erase(delegate);
				}

				template< class X, class Y >
				void disconnect(void (X::*func)(Param1 p1), Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void disconnect(void (X::*func)(Param1 p1) const, Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				void clear()
				{
					delegateList.clear();
				}

				void emit(Param1 p1) const
				{
					for (DelegateIterator i = delegateList.begin(); i != delegateList.end();)
					{
						(*(i++))(p1);
					}
				}

				void operator() (Param1 p1) const
				{
					emit(p1);
				}

				bool empty() const
				{
					return delegateList.empty();
				}
			};


			template< class Param1, class Param2 >
			class signal2
			{
			public:
				typedef Delegate2< Param1, Param2 > _Delegate;

			private:
				typedef std::set<_Delegate> DelegateList;
				typedef typename DelegateList::const_iterator DelegateIterator;
				DelegateList delegateList;

			public:
				void connect(_Delegate delegate)
				{
					delegateList.insert(delegate);
				}

				template< class X, class Y >
				void connect(void (X::*func)(Param1 p1, Param2 p2), Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void connect(void (X::*func)(Param1 p1, Param2 p2) const, Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				void disconnect(_Delegate delegate)
				{
					delegateList.erase(delegate);
				}

				template< class X, class Y >
				void disconnect(void (X::*func)(Param1 p1, Param2 p2), Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void disconnect(void (X::*func)(Param1 p1, Param2 p2) const, Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				void clear()
				{
					delegateList.clear();
				}

				void emit(Param1 p1, Param2 p2) const
				{
					for (DelegateIterator i = delegateList.begin(); i != delegateList.end();)
					{
						(*(i++))(p1, p2);
					}
				}

				void operator() (Param1 p1, Param2 p2) const
				{
					emit(p1, p2);
				}

				bool empty() const
				{
					return delegateList.empty();
				}
			};


			template< class Param1, class Param2, class Param3 >
			class signal3
			{
			public:
				typedef Delegate3< Param1, Param2, Param3 > _Delegate;

			private:
				typedef std::set<_Delegate> DelegateList;
				typedef typename DelegateList::const_iterator DelegateIterator;
				DelegateList delegateList;

			public:
				void connect(_Delegate delegate)
				{
					delegateList.insert(delegate);
				}

				template< class X, class Y >
				void connect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3), Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void connect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3) const, Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				void disconnect(_Delegate delegate)
				{
					delegateList.erase(delegate);
				}

				template< class X, class Y >
				void disconnect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3), Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void disconnect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3) const, Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				void clear()
				{
					delegateList.clear();
				}

				void emit(Param1 p1, Param2 p2, Param3 p3) const
				{
					for (DelegateIterator i = delegateList.begin(); i != delegateList.end();)
					{
						(*(i++))(p1, p2, p3);
					}
				}

				void operator() (Param1 p1, Param2 p2, Param3 p3) const
				{
					emit(p1, p2, p3);
				}

				bool empty() const
				{
					return delegateList.empty();
				}
			};


			template< class Param1, class Param2, class Param3, class Param4 >
			class signal4
			{
			public:
				typedef Delegate4< Param1, Param2, Param3, Param4 > _Delegate;

			private:
				typedef std::set<_Delegate> DelegateList;
				typedef typename DelegateList::const_iterator DelegateIterator;
				DelegateList delegateList;

			public:
				void connect(_Delegate delegate)
				{
					delegateList.insert(delegate);
				}

				template< class X, class Y >
				void connect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4), Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void connect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const, Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				void disconnect(_Delegate delegate)
				{
					delegateList.erase(delegate);
				}

				template< class X, class Y >
				void disconnect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4), Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void disconnect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const, Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				void clear()
				{
					delegateList.clear();
				}

				void emit(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const
				{
					for (DelegateIterator i = delegateList.begin(); i != delegateList.end();)
					{
						(*(i++))(p1, p2, p3, p4);
					}
				}

				void operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4) const
				{
					emit(p1, p2, p3, p4);
				}

				bool empty() const
				{
					return delegateList.empty();
				}
			};


			template< class Param1, class Param2, class Param3, class Param4, class Param5 >
			class signal5
			{
			public:
				typedef Delegate5< Param1, Param2, Param3, Param4, Param5 > _Delegate;

			private:
				typedef std::set<_Delegate> DelegateList;
				typedef typename DelegateList::const_iterator DelegateIterator;
				DelegateList delegateList;

			public:
				void connect(_Delegate delegate)
				{
					delegateList.insert(delegate);
				}

				template< class X, class Y >
				void connect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5), Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void connect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const, Y * obj)
				{
					delegateList.insert(MakeDelegate(obj, func));
				}

				void disconnect(_Delegate delegate)
				{
					delegateList.erase(delegate);
				}

				template< class X, class Y >
				void disconnect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5), Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				template< class X, class Y >
				void disconnect(void (X::*func)(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const, Y * obj)
				{
					delegateList.erase(MakeDelegate(obj, func));
				}

				void clear()
				{
					delegateList.clear();
				}

				void emit(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const
				{
					for (DelegateIterator i = delegateList.begin(); i != delegateList.end();)
					{
						(*(i++))(p1, p2, p3, p4, p5);
					}
				}

				void operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const
				{
					emit(p1, p2, p3, p4, p5);
				}

				bool empty() const
				{
					return delegateList.empty();
				}
			};


		}
	}
}
