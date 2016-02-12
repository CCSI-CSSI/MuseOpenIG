//#******************************************************************************
//#*
//#*      Copyright (C) 2015  Compro Computer Services
//#*      http://openig.compro.net
//#*
//#*      Source available at: https://github.com/CCSI-CSSI/MuseOpenIG
//#*
//#*      This software is released under the LGPL.
//#*
//#*   This software is free software; you can redistribute it and/or modify
//#*   it under the terms of the GNU Lesser General Public License as published
//#*   by the Free Software Foundation; either version 2.1 of the License, or
//#*   (at your option) any later version.
//#*
//#*   This software is distributed in the hope that it will be useful,
//#*   but WITHOUT ANY WARRANTY; without even the implied warranty of
//#*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
//#*   the GNU Lesser General Public License for more details.
//#*
//#*   You should have received a copy of the GNU Lesser General Public License
//#*   along with this library; if not, write to the Free Software
//#*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//#*
//#*****************************************************************************

//#*****************************************************************************
//#* Original code borrowed from the MyGUI Samples
//#*****************************************************************************

/*!
	@file
	@author		Albert Semenov
	@date		09/2008
*/
#ifndef COLOUR_PANEL_H_
#define COLOUR_PANEL_H_

#include <MYGUI/MyGUI.h>

namespace OpenIG {
	namespace Plugins {

		namespace attribute
		{

			// класс обертка для удаления данных из статического вектора
			template <typename Type>
			struct DataHolder
			{
				~DataHolder()
				{
					for (typename Type::iterator item = data.begin(); item != data.end(); ++item)
						delete (*item).first;
				}

				Type data;
			};

			// интерфейс для обертки поля
			template <typename OwnerType, typename SetterType>
			struct Field
			{
				virtual ~Field() { }
				virtual bool set(OwnerType* _target, typename SetterType::BaseValueType* _value) = 0;
				virtual const std::string& getFieldTypeName() = 0;
			};

			// шаблон для обертки поля
			template <typename OwnerType, typename FieldType, typename SetterType>
			struct FieldHolder : public Field<OwnerType, SetterType>
			{
				FieldHolder(FieldType* OwnerType::* offset) : m_offset(offset) {  }
				FieldType* OwnerType::* const m_offset;

				virtual bool set(OwnerType* _target, typename SetterType::BaseValueType* _value)
				{
					_target->*m_offset = SetterType::template convert<FieldType>(_value);
					return _target->*m_offset != 0;
				}
				virtual const std::string& getFieldTypeName()
				{
					return FieldType::getClassTypeName();
				}
			};

			// шаблон для атрибута поля
			template <typename OwnerType, typename ValueType, typename SetterType>
			struct AttributeField
			{
				typedef std::pair<Field<OwnerType, SetterType>*, ValueType> BindPair;
				typedef std::vector<BindPair> VectorBindPair;

				template <typename FieldType>
				AttributeField(FieldType* OwnerType::* _offset, const ValueType& _value)
				{
					getData().push_back(BindPair(new FieldHolder<OwnerType, FieldType, SetterType>(_offset), _value));
				}
				static VectorBindPair& getData()
				{
					static DataHolder<VectorBindPair> data;
					return data.data;
				}
			};

			// макрос для инстансирования атрибута поля
#define DECLARE_ATTRIBUTE_FIELD(_name, _type, _setter) \
	template <typename OwnerType, typename ValueType = _type, typename SetterType = _setter> \
	struct _name : public attribute::AttributeField<OwnerType, ValueType, SetterType> \
					{ \
		template <typename FieldType> \
		_name(FieldType* OwnerType::* _offset, const ValueType& _value) : \
			AttributeField<OwnerType, ValueType, SetterType>(_offset, _value) { } \
					}

			// макрос для инстансирования экземпляра атрибута
#define ATTRIBUTE_FIELD(_attribute, _class, _field, _value) \
	struct _attribute##_##_field \
					{ \
		_attribute##_##_field() \
						{ \
			static attribute::_attribute<_class> bind(&_class::_field, _value); \
						} \
					} _attribute##_##_field


			// шаблон для атрибута класса
			template <typename Type, typename ValueType>
			struct ClassAttribute
			{
				ClassAttribute(const ValueType& _value)
				{
					getData() = _value;
				}
				static ValueType& getData()
				{
					static ValueType data;
					return data;
				}
			};

			// макрос для инстансирования атрибута класса
#define DECLARE_ATTRIBUTE_CLASS(_name, _type) \
	template <typename Type, typename ValueType = _type> \
	struct _name : public attribute::ClassAttribute<_name<Type>, ValueType> \
					{ \
		_name(const ValueType& _value) : \
			ClassAttribute<_name<Type>, ValueType>(_value) { } \
					}

			// макрос для инстансирования экземпляра класса
#define ATTRIBUTE_CLASS(_attribute, _class, _value) \
	class _class; \
	static attribute::_attribute<_class> _attribute##_##_class(_value)
		}

		namespace attribute
		{

			struct FieldSetterWidget
			{
				typedef MyGUI::Widget BaseValueType;

				template <typename Type>
				static Type* convert(BaseValueType* _value)
				{
					return _value == 0 ? 0 : _value->castType<Type>(false);
				}
			};

			DECLARE_ATTRIBUTE_FIELD(AttributeFieldWidgetName, std::string, FieldSetterWidget);

#define ATTRIBUTE_FIELD_WIDGET_NAME(_class, _field, _value) \
	ATTRIBUTE_FIELD(AttributeFieldWidgetName, _class, _field, _value)


			DECLARE_ATTRIBUTE_CLASS(AttributeSize, MyGUI::IntSize);

#define ATTRIBUTE_CLASS_SIZE(_class, _value) \
	ATTRIBUTE_CLASS(AttributeSize, _class, _value)


			DECLARE_ATTRIBUTE_CLASS(AttributeLayout, std::string);

#define ATTRIBUTE_CLASS_LAYOUT(_class, _value) \
	ATTRIBUTE_CLASS(AttributeLayout, _class, _value)

		}

		namespace wraps
		{

			class BaseLayout
			{
			protected:
				BaseLayout() :
					mMainWidget(nullptr)
				{
				}

				BaseLayout(const std::string& _layout, MyGUI::Widget* _parent = nullptr) :
					mMainWidget(nullptr)
				{
					initialise(_layout, _parent);
				}

				template <typename T>
				void assignWidget(T * & _widget, const std::string& _name, bool _throw = true, bool _createFakeWidgets = true)
				{
					_widget = nullptr;
					for (MyGUI::VectorWidgetPtr::iterator iter = mListWindowRoot.begin(); iter != mListWindowRoot.end(); ++iter)
					{
						MyGUI::Widget* find = (*iter)->findWidget(mPrefix + _name);
						if (nullptr != find)
						{
							T* cast = find->castType<T>(false);
							if (nullptr != cast)
							{
								_widget = cast;
							}
							else
							{
								MYGUI_LOG(Warning, "Widget with name '" << _name << "' have wrong type ('" <<
									find->getTypeName() << "instead of '" << T::getClassTypeName() << "'). [" << mLayoutName << "]");
								MYGUI_ASSERT(!_throw, "Can't assign widget with name '" << _name << "'. [" << mLayoutName << "]");
								if (_createFakeWidgets)
									_widget = _createFakeWidget<T>(mMainWidget);
							}

							return;
						}
					}
					MYGUI_LOG(Warning, "Widget with name '" << _name << "' not found. [" << mLayoutName << "]");
					MYGUI_ASSERT(!_throw, "Can't assign widget with name '" << _name << "'. [" << mLayoutName << "]");
					if (_createFakeWidgets)
						_widget = _createFakeWidget<T>(mMainWidget);
				}

				template <typename T>
				void assignBase(T * & _widget, const std::string& _name, bool _throw = true, bool _createFakeWidgets = true)
				{
					_widget = nullptr;
					for (MyGUI::VectorWidgetPtr::iterator iter = mListWindowRoot.begin(); iter != mListWindowRoot.end(); ++iter)
					{
						MyGUI::Widget* find = (*iter)->findWidget(mPrefix + _name);
						if (nullptr != find)
						{
							_widget = new T(find);
							mListBase.push_back(_widget);
							return;
						}
					}

					MYGUI_LOG(Warning, "Widget with name '" << _name << "' not found. [" << mLayoutName << "]");
					MYGUI_ASSERT(!_throw, "Can't assign base widget with name '" << _name << "'. [" << mLayoutName << "]");
					if (_createFakeWidgets)
					{
						_widget = new T(_createFakeWidget<MyGUI::Widget>(mMainWidget));
						mListBase.push_back(_widget);
					}
				}

				void initialise(const std::string& _layout, MyGUI::Widget* _parent = nullptr, bool _throw = true, bool _createFakeWidgets = true)
				{
					const std::string MAIN_WINDOW1 = "_Main";
					const std::string MAIN_WINDOW2 = "Root";
					mLayoutName = _layout;

					// оборачиваем
					if (mLayoutName.empty())
					{
						mMainWidget = _parent;
						if (mMainWidget != nullptr)
						{
							mListWindowRoot.push_back(mMainWidget);
							mPrefix = FindParentPrefix(mMainWidget);
						}
					}
					// загружаем лейаут на виджет
					else
					{
						mPrefix = MyGUI::utility::toString(this, "_");
						mListWindowRoot = MyGUI::LayoutManager::getInstance().loadLayout(mLayoutName, mPrefix, _parent);

						const std::string mainName1 = mPrefix + MAIN_WINDOW1;
						const std::string mainName2 = mPrefix + MAIN_WINDOW2;
						for (MyGUI::VectorWidgetPtr::iterator iter = mListWindowRoot.begin(); iter != mListWindowRoot.end(); ++iter)
						{
							if ((*iter)->getName() == mainName1 || (*iter)->getName() == mainName2)
							{
								mMainWidget = (*iter);

								snapToParent(mMainWidget);

								break;
							}
						}

						if (mMainWidget == nullptr)
						{
							MYGUI_LOG(Warning, "Root widget with name '" << MAIN_WINDOW1 << "' or '" << MAIN_WINDOW2 << "'  not found. [" << mLayoutName << "]");
							MYGUI_ASSERT(!_throw, "No root widget. ['" << mLayoutName << "]");
							if (_createFakeWidgets)
								mMainWidget = _createFakeWidget<MyGUI::Widget>(_parent);
						}

						mMainWidget->setUserString("BaseLayoutPrefix", mPrefix);
					}
				}

				void shutdown()
				{
					// удаляем все классы
					for (VectorBasePtr::reverse_iterator iter = mListBase.rbegin(); iter != mListBase.rend(); ++iter)
						delete (*iter);
					mListBase.clear();

					// удаляем все рутовые виджеты
					if (!mLayoutName.empty())
						MyGUI::LayoutManager::getInstance().unloadLayout(mListWindowRoot);
					mListWindowRoot.clear();
				}

				template <typename Type>
				void initialiseByAttributes(Type* _owner, MyGUI::Widget* _parent = nullptr, bool _throw = true, bool _createFakeWidgets = true)
				{
					initialise(attribute::AttributeLayout<Type>::getData(), _parent, _throw, _createFakeWidgets);

					typename attribute::AttributeFieldWidgetName<Type>::VectorBindPair& data = attribute::AttributeFieldWidgetName<Type>::getData();
					for (typename attribute::AttributeFieldWidgetName<Type>::VectorBindPair::iterator item = data.begin(); item != data.end(); ++item)
					{
						MyGUI::Widget* value = nullptr;
						assignWidget(value, item->second, _throw, false);

						bool result = item->first->set(_owner, value);

						if (!result && _createFakeWidgets)
						{
							value = _createFakeWidgetT(item->first->getFieldTypeName(), mMainWidget);
							item->first->set(_owner, value);
						}
					}
				}

			private:
				std::string FindParentPrefix(MyGUI::Widget* _parent)
				{
					std::string prefix = _parent->getUserString("BaseLayoutPrefix");
					if (!prefix.empty())
						return prefix;
					if (_parent->getParent() != nullptr)
						return FindParentPrefix(_parent->getParent());

					return "";
				}

				void snapToParent(MyGUI::Widget* _child)
				{
					if (_child->isUserString("SnapTo"))
					{
						MyGUI::Align align = MyGUI::Align::parse(_child->getUserString("SnapTo"));

						MyGUI::IntCoord coord = _child->getCoord();
						MyGUI::IntSize size = _child->getParentSize();

						if (align.isHStretch())
						{
							coord.left = 0;
							coord.width = size.width;
						}
						else if (align.isLeft())
						{
							coord.left = 0;
						}
						else if (align.isRight())
						{
							coord.left = size.width - coord.width;
						}
						else
						{
							coord.left = (size.width - coord.width) / 2;
						}

						if (align.isVStretch())
						{
							coord.top = 0;
							coord.height = size.height;
						}
						else if (align.isTop())
						{
							coord.top = 0;
						}
						else if (align.isBottom())
						{
							coord.top = size.height - coord.height;
						}
						else
						{
							coord.top = (size.height - coord.height) / 2;
						}

						_child->setCoord(coord);
					}
				}

				template <typename T>
				T* _createFakeWidget(MyGUI::Widget* _parent)
				{
					return static_cast<T*>(_createFakeWidgetT(T::getClassTypeName(), _parent));
				}

				MyGUI::Widget* _createFakeWidgetT(const std::string& _typeName, MyGUI::Widget* _parent)
				{
					if (_parent)
						return _parent->createWidgetT(_typeName, MyGUI::SkinManager::getInstance().getDefaultSkin(), MyGUI::IntCoord(), MyGUI::Align::Default);

					return MyGUI::Gui::getInstance().createWidgetT(_typeName, MyGUI::SkinManager::getInstance().getDefaultSkin(), MyGUI::IntCoord(), MyGUI::Align::Default, "");
				}

			public:
				virtual ~BaseLayout()
				{
					shutdown();
				}

			protected:
				MyGUI::Widget* mMainWidget;

			private:
				std::string mPrefix;
				std::string mLayoutName;
				MyGUI::VectorWidgetPtr mListWindowRoot;
				typedef std::vector<BaseLayout*> VectorBasePtr;
				VectorBasePtr mListBase;
			};

		} // namespace wraps


		namespace demo
		{
			ATTRIBUTE_CLASS_LAYOUT(ColourPanel, "ColourPanel.layout");
			class ColourPanel : public wraps::BaseLayout
			{
			public:
				ColourPanel();
				virtual ~ColourPanel();

				void setColour(const MyGUI::Colour& _colour);
				const MyGUI::Colour& getColour() const;

				MyGUI::delegates::CDelegate1<ColourPanel*> eventColourAccept;

				void setVisible(bool visible);
				bool isVisible() const;
				void centerOnScreen(unsigned width, unsigned height);

			private:
				void notifyMouseDrag(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id);
				void notifyMouseButtonPressed(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id);
				void notifyScrollChangePosition(MyGUI::ScrollBar* _sender, size_t _position);
				void notifyEditTextChange(MyGUI::EditBox* _sender);
				void notifyMouseButtonClick(MyGUI::Widget* _sender);
				void notifyCancelMouseButtonClick(MyGUI::Widget* _sender);

				void updateFirst();

				void createTexture();
				void updateTexture(const MyGUI::Colour& _colour);
				void destroyTexture();

				void updateFromPoint(const MyGUI::IntPoint& _point);
				void updateFromColour(const MyGUI::Colour& _colour);

				MyGUI::Colour getSaturate(const MyGUI::Colour& _colour) const;

				float& byIndex(MyGUI::Colour& _colour, size_t _index);

			private:
				ATTRIBUTE_FIELD_WIDGET_NAME(ColourPanel, mColourRect, "widget_ColourRect");
				MyGUI::ImageBox* mColourRect;

				ATTRIBUTE_FIELD_WIDGET_NAME(ColourPanel, mColourView, "widget_ColourView");
				MyGUI::Widget* mColourView;

				ATTRIBUTE_FIELD_WIDGET_NAME(ColourPanel, mImageColourPicker, "image_Picker");
				MyGUI::ImageBox* mImageColourPicker;

				ATTRIBUTE_FIELD_WIDGET_NAME(ColourPanel, mEditRed, "edit_Red");
				MyGUI::EditBox* mEditRed;

				ATTRIBUTE_FIELD_WIDGET_NAME(ColourPanel, mEditGreen, "edit_Green");
				MyGUI::EditBox* mEditGreen;

				ATTRIBUTE_FIELD_WIDGET_NAME(ColourPanel, mEditBlue, "edit_Blue");
				MyGUI::EditBox* mEditBlue;

				ATTRIBUTE_FIELD_WIDGET_NAME(ColourPanel, mScrollRange, "scroll_Range");
				MyGUI::ScrollBar* mScrollRange;

				ATTRIBUTE_FIELD_WIDGET_NAME(ColourPanel, mOk, "button_OK");
				MyGUI::Button* mOk;

				ATTRIBUTE_FIELD_WIDGET_NAME(ColourPanel, mCancel, "Button_Cancel");
				MyGUI::Button* mCancel;

				MyGUI::Colour mCurrentColour;
				MyGUI::Colour mBaseColour;

				std::vector<MyGUI::Colour> mColourRange;

				MyGUI::ITexture* mTexture;
			};
		}
	}
} // namespace demo

#endif // COLOUR_PANEL_H_
