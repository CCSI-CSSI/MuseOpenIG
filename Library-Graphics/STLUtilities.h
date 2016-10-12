/*
-----------------------------------------------------------------------------
File:        STLUtilities.h
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
#ifndef STLUTILITIES_H
#define STLUTILITIES_H

#include <utility>
#include <functional>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <algorithm>

#define STLUTILS_USE_CPLUS_PLUS_UNORDERED_CONTAINERS 0

#if STLUTILS_USE_CPLUS_PLUS_UNORDERED_CONTAINERS
#include <unordered_map>
#include <unordered_set>
#define unordered_namespace std
#else
#define unordered_namespace boost
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#endif

#if defined(OPENIG_SDK)
	#include <OpenIG-Graphics/OIGAssert.h>
#else
	#include <Library-Graphics/OIGAssert.h>
#endif

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			template<typename P>
			struct extract_first
			{
				const typename P::first_type&
					operator()(const P& p) const
				{
					return p.first;
				}
			};

			template<typename P>
			struct extract_second
			{
				const typename P::second_type&
					operator()(const P& p) const
				{
					return p.second;
				}
			};

			template<class _Fn>
			class binderNoArgs
				: public std::unary_function < typename _Fn::argument_type, typename _Fn::result_type >
			{	// functor adapter _Func(stored, right)
			public:
				typedef std::unary_function < typename _Fn::argument_type,
					typename _Fn::result_type > _Base;
				typedef typename _Base::argument_type argument_type;
				typedef typename _Base::result_type result_type;

				binderNoArgs(const _Fn& _Func,
					const typename _Fn::argument_type& _Left)
					: op(_Func), value(_Left)
				{	// construct from functor and left operand
				}

				result_type operator()(void) const
				{	// apply functor to operands
					return (op(value));
				}

				result_type operator()(void)
				{	// apply functor to operands
					return (op(value));
				}

			protected:
				_Fn op;	// the functor to apply
				typename _Fn::argument_type value;	// the left operand
			};

			// TEMPLATE FUNCTION bind (based upon std::bind1st)
			template<class _Fn,
			class _Ty> inline
				binderNoArgs<_Fn> bind(const _Fn& _Func, const _Ty& _Left)
			{	// return a binderNoArgs functor adapter
				typename _Fn::argument_type _Val(_Left);
				return (binderNoArgs<_Fn>(_Func, _Val));
			}


			template <class T> struct DeletePtr
			{
				void operator()(T* obj)
				{
					delete obj;
				}
			};

			template <class T>
			void verify_index_for_container(size_t index, const T& t)
			{
#ifndef _DEBUG
				static_cast<void>(t);
				static_cast<void>(index);
#endif
				ASSERT_PREDICATE((index == 0 && t.size() == 1)
					|| (t.size() > 1 && index <= t.size() - 1));
			}

			template <class C, class T> void destroy_from_container(C& _container, T* pT)
			{
				typename C::const_iterator it = std::find(_container.begin(), _container.end(), pT);
				if (it != _container.end())
				{
					delete pT; pT = 0;
					_container.erase(it);
				}
			}

			template <class T> void destroy_from_container(std::list<T*>& _container, T* pT)
			{
				typename std::list<T*>::iterator it = std::find(_container.begin(), _container.end(), pT);
				if (it != _container.end())
				{
					delete pT; pT = 0;
					_container.erase(it);
				}
			}

			template <class C, class T> void remove_from_container(C& _container, T* pT)
			{
				typename C::const_iterator it = std::find(_container.begin(), _container.end(), pT);
				if (it != _container.end())
				{
					delete pT; pT = 0;
					_container.erase(it);
				}
			}

			template <class K, class T> T* get_from_map(std::map<K, T*>& _map, const K& key)
			{
				typename std::map<K, T*>::const_iterator it = _map.find(key);
				if (it == _map.end())
					return 0;
				return it->second;
			}

			template <class K, class T> T* get_from_map(unordered_namespace::unordered_map<K, T*>& _map, const K& key)
			{
				typename unordered_namespace::unordered_map<K, T*>::const_iterator it = _map.find(key);
				if (it == _map.end())
					return 0;
				return it->second;
			}

			template <class K, class T> void destroy_from_map(std::map<K, T*>& _map, const K& key)
			{
				typename std::map<K, T*>::iterator it = _map.find(key);
				ASSERT_PREDICATE(it != _map.end());

				T* pT = it->second;

				_map.erase(key);

				delete pT; pT = 0;
			}

			template <class K, class T> void destroy_from_map(unordered_namespace::unordered_map<K, T*>& _map, const K& key)
			{
				typename unordered_namespace::unordered_map<K, T*>::iterator it = _map.find(key);
				ASSERT_PREDICATE(it != _map.end());

				T* pT = it->second;

				_map.erase(key);

				delete pT; pT = 0;
			}

			template <class T> void destroy_all_from_vector(std::vector<T*>& _vector)
			{
				while (_vector.empty() == false)
				{
					OpenIG::Library::Graphics::destroy_from_container(_vector, *_vector.begin());
				}
			};

			template <class T> void destroy_all_from_list(std::list<T*>& _list)
			{
				while (_list.empty() == false)
				{
					OpenIG::Library::Graphics::destroy_from_container(_list, *_list.begin());
				}
			}

			template <class T> void destroy_all_from_unordered_set(unordered_namespace::unordered_set<T*>& _set)
			{
				while (_set.empty() == false)
				{
					OpenIG::Library::Graphics::destroy_from_container(_set, *_set.begin());
				}
			}

			template <class K, class T> void destroy_all_from_map(std::map<K, T*>& _map)
			{
				while (_map.empty() == false)
				{
					typename std::map<K, T*>::iterator it = _map.begin();

					T* pT = it->second;
					delete pT; pT = 0;

					_map.erase(it);
				}
				_map.clear();
			}
			template <class K, class T> void destroy_all_from_list(std::list<T*>& _list)
			{
				while (_list.empty() == false)
				{
					typename std::list<T*>::iterator it = _list.begin();

					T* pT = *it;
					delete pT; pT = 0;

					_list.erase(it);
				}
				_list.clear();
			}
			template <class K, class T> void destroy_all_from_map(unordered_namespace::unordered_map<K, T*>& _map)
			{
				while (_map.empty() == false)
				{
					typename unordered_namespace::unordered_map<K, T*>::iterator it = _map.begin();

					T* pT = it->second;
					delete pT; pT = 0;

					_map.erase(it);
				}
				_map.clear();
			}

			template <class T> T* get_nth_item(unordered_namespace::unordered_set<T*>& _set, size_t index)
			{
				if (_set.empty())
					return 0;

				OpenIG::Library::Graphics::verify_index_for_container(index, _set);

				typename unordered_namespace::unordered_set<T*>::const_iterator it = _set.begin();
				std::advance(it, index);
				return *it;
			}

			template <class T> const T* get_nth_item(const unordered_namespace::unordered_set<T*>& _set, size_t index)
			{
				if (_set.empty())
					return 0;

				OpenIG::Library::Graphics::verify_index_for_container(index, _set);

				typename unordered_namespace::unordered_set<T*>::const_iterator it = _set.begin();
				std::advance(it, index);
				return *it;
			}

			template <class T> T* get_nth_item(std::list<T*>& _list, size_t index)
			{
				if (_list.empty())
					return 0;

				OpenIG::Library::Graphics::verify_index_for_container(index, _list);

				typename std::list<T*>::const_iterator it = _list.begin();
				std::advance(it, index);
				return *it;
			}

			template <class T> const T* get_nth_item(const std::list<T*>& _list, size_t index)
			{
				if (_list.empty())
					return 0;

				OpenIG::Library::Graphics::verify_index_for_container(index, _list);

				typename std::list<T*>::const_iterator it = _list.begin();
				std::advance(it, index);
				return *it;
			}

			template <class T> T* get_nth_item(std::vector<T*>& _vector, size_t index)
			{
				if (_vector.empty())
					return 0;

				OpenIG::Library::Graphics::verify_index_for_container(index, _vector);

				return _vector[index];
			}

			template <class T> T* get_nth_item(const std::vector<T*>& _vector, size_t index)
			{
				if (_vector.empty())
					return 0;

				OpenIG::Library::Graphics::verify_index_for_container(index, _vector);

				return _vector[index];
			}

			template <class K, class T> T* get_nth_item(std::map<K, T*>& _map, size_t index)
			{
				if (_map.empty())
					return 0;

				OpenIG::Library::Graphics::verify_index_for_container(index, _map);

				typename std::map<K, T*>::const_iterator it = _map.begin();
				std::advance(it, index);
				return it->second;
			}

			template <class K, class T> T* get_nth_item(const std::map<K, T*>& _map, size_t index)
			{
				if (_map.empty())
					return 0;

				OpenIG::Library::Graphics::verify_index_for_container(index, _map);

				typename std::map<K, T*>::const_iterator it = _map.begin();
				std::advance(it, index);
				return it->second;
			}

			template <class K, class T> T* get_nth_item(unordered_namespace::unordered_map<K, T*>& _map, size_t index)
			{
				if (_map.empty())
					return 0;

				OpenIG::Library::Graphics::verify_index_for_container(index, _map);

				typename unordered_namespace::unordered_map<K, T*>::const_iterator it = _map.begin();
				std::advance(it, index);
				return it->second;
			}

			template <class K, class T> const T& get_nth_item(const unordered_namespace::unordered_map<K, T>& _map, size_t index)
			{
				ASSERT_PREDICATE(_map.empty() == false);

				OpenIG::Library::Graphics::verify_index_for_container(index, _map);

				typename unordered_namespace::unordered_map<K, T>::const_iterator it = _map.begin();
				std::advance(it, index);
				return it->second;
			}

			template <class K, class T> T* get_nth_item(const unordered_namespace::unordered_map<K, T*>& _map, size_t index)
			{
				if (_map.empty())
					return 0;

				OpenIG::Library::Graphics::verify_index_for_container(index, _map);

				typename unordered_namespace::unordered_map<K, T*>::const_iterator it = _map.begin();
				std::advance(it, index);
				return it->second;
			}
		}
	}
}
#endif

