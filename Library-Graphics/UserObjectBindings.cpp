/*
-----------------------------------------------------------------------------
File:        UserObjectBindings.cpp
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
#include "UserObjectBindings.h"
#include <cassert>

namespace OpenIG {
	namespace Library {
		namespace Graphics {

			UserObjectBindings::UserObjectBindings()
			{

			}
			UserObjectBindings::~UserObjectBindings()
			{

			}

			void UserObjectBindings::SetUserAny(const std::string& key, const boost::any& anything)
			{
				UserObjectsMap::const_iterator it = m_UserObjectsMap.find(key);
				if (it == m_UserObjectsMap.end())
				{
					m_UserObjectsMap.insert(std::make_pair(key, anything));
				}
				else
				{
					m_UserObjectsMap[key] = anything;
				}
			}

			bool UserObjectBindings::HasKey(const std::string& key) const
			{
				UserObjectsMap::const_iterator it = m_UserObjectsMap.find(key);
				return it != m_UserObjectsMap.end();
			}

			const boost::any& UserObjectBindings::GetUserAny(const std::string& key)
			{
				assert(HasKey(key));
				return m_UserObjectsMap[key];
			}

			void UserObjectBindings::EraseKey(const std::string& key)
			{
				m_UserObjectsMap.erase(key);
			}

		}
	}
}
