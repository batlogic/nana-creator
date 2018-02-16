/*
 *		guimanager Implementation
 *
 *      Nana C++ Library - Creator
 *      Author: besh81
 */

#include <iostream>
#include "ctrls/panel.h"
#include "ctrls/button.h"
#include "ctrls/label.h"
#include "ctrls/textbox.h"
#include "ctrls/combox.h"
#include "ctrls/spinbox.h"
#include "ctrls/listbox.h"
#include "ctrls/checkbox.h"
#include "ctrls/date_chooser.h"
#include "ctrls/toolbar.h"
#include "ctrls/form.h"
#include "ctrls/categorize.h"
#include "ctrls/group.h"
#include "ctrls/menubar.h"
#include "ctrls/picture.h"
#include "ctrls/progress.h"
#include "ctrls/slider.h"
#include "ctrls/tabbar.h"
#include "guimanager.h"
#include "lock_guard.h"
#include "style.h"



void guimanager::clear()
{
	_selected = 0;

	auto root = _ctrls.get_root();
	if(root)
	{
		auto root_child = root->child;
		if(root_child)
		{
			_main_wd->remove(*root_child->value->nanawdg);
		}
	}
	
	_ctrls.clear();
	_op->clear();
	_name_mgr.clear();
}


void guimanager::cursor(cursor_state state)
{
	if(state.action == cursor_action::add)
	{
		if(state.type == CTRL_MAINFORM)
		{
			addmainform();
			// reset mouse cursor
			state = cursor_state{ cursor_action::select };
		}
		else if(state.type == CTRL_MAINPANEL)
		{
			addmainpanel();
			// reset mouse cursor
			state = cursor_state{ cursor_action::select };
		}
		else if(!_main_wd->haschild())
		{
			nana::msgbox m(_root_wd, CREATOR_NAME, nana::msgbox::ok);
			m.icon(m.icon_warning);
			m << "Create a main widget!";
			m();

			// reset mouse cursor
			state = cursor_state{ cursor_action::select };
		}
	}

	_cursor_state = state;
}


tree_node<control_struct>* guimanager::addmainform(const std::string& name)
{
	if(_main_wd->haschild())
	{
		nana::msgbox m(_root_wd, CREATOR_NAME, nana::msgbox::ok);
		m.icon(m.icon_warning);
		m << "Widget already exists. Try to add a layout/control!";
		m();
		return 0;
	}

	control_struct ctrl = control_struct(new ctrls::ctrl_struct());
	// attibutes
	ctrl->properties.append("mainclass") = true;
	// nana::widget + properties
	ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::form(*_main_wd, &ctrl->properties, name.empty() ? _name_mgr.add_numbered(CTRL_FORM) : name, _deserializing ? false : true));

	//TEMP
	ctrl->nanawdg->bgcolor(nana::API::bgcolor(nana::API::get_parent_window(*_main_wd)));
	//
	_main_wd->add(*ctrl->nanawdg);


	// events
	control_struct_ptr pctrl = ctrl;
	ctrl->nanawdg->events().mouse_enter([this, pctrl]()
	{
		auto pform = static_cast<ctrls::form*>(pctrl.lock()->nanawdg.get());
		if(pform->haschild())
			return; // already has a child

		if(!pctrl.lock()->ishighlighted && cursor().action != cursor_action::select && cursor().type == CTRL_LAYOUT)
		{
			pctrl.lock()->set_highlight(HIGHLIGHT_ON_PLACE);
		}
	});

	ctrl->nanawdg->events().mouse_leave([this, pctrl]()
	{
		if(pctrl.lock()->ishighlighted)
		{
			pctrl.lock()->reset_highlight();
		}
	});

	ctrl->nanawdg->events().click([this, pctrl](const nana::arg_click & arg)
	{
		clickctrl(pctrl.lock());
	});


	return _registerobject(ctrl, 0);
}


tree_node<control_struct>* guimanager::addmainpanel(const std::string& name)
{
	if(_main_wd->haschild())
	{
		nana::msgbox m(_root_wd, CREATOR_NAME, nana::msgbox::ok);
		m.icon(m.icon_warning);
		m << "Widget already exists. Try to add a layout/control!";
		m();
		return 0;
	}

	control_struct ctrl = control_struct(new ctrls::ctrl_struct());
	// attibutes
	ctrl->properties.append("mainclass") = true;
	// nana::widget + properties
	ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::panel(*_main_wd, &ctrl->properties, name.empty() ? _name_mgr.add_numbered(CTRL_PANEL) : name, _deserializing ? false : true));
	
	//TEMP
	ctrl->nanawdg->bgcolor(nana::API::bgcolor(nana::API::get_parent_window(*_main_wd)));
	//
	_main_wd->add(*ctrl->nanawdg);
	

	// events
	control_struct_ptr pctrl = ctrl;
	ctrl->nanawdg->events().mouse_enter([this, pctrl]()
	{
		auto ppanel = static_cast<ctrls::panel*>(pctrl.lock()->nanawdg.get());
		if(ppanel->haschild())
			return; // already has a child

		if(!pctrl.lock()->ishighlighted && cursor().action != cursor_action::select && cursor().type == CTRL_LAYOUT)
		{
			pctrl.lock()->set_highlight(HIGHLIGHT_ON_PLACE);
		}
	});

	ctrl->nanawdg->events().mouse_leave([this, pctrl]()
	{
		if(pctrl.lock()->ishighlighted)
		{
			pctrl.lock()->reset_highlight();
		}
	});

	ctrl->nanawdg->events().click([this, pctrl](const nana::arg_click & arg)
	{
		clickctrl(pctrl.lock());
	});


	return _registerobject(ctrl, 0);
}


tree_node<control_struct>* guimanager::addlayout(tree_node<control_struct>* parent, ctrls::layout_orientation layout, const std::string& name)
{
	control_struct parent_ = parent->value;
	if(!_checksonship(CTRL_LAYOUT, parent_->properties.property("type").as_string()))
		return 0;

	control_struct ctrl = control_struct(new ctrls::ctrl_struct());

	// nana::widget + properties
	ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::layout(*parent_->nanawdg, &ctrl->properties, name.empty() ? _name_mgr.add_numbered(CTRL_LAYOUT) : name, layout));
	// 
	if(parent_->properties.property("type").as_string() == CTRL_PANEL)
	{
		auto ppanel = static_cast<ctrls::panel*>(parent_->nanawdg.get());
		ppanel->append(*ctrl->nanawdg);
	}
	else if(parent_->properties.property("type").as_string() == CTRL_FORM)
	{
		auto pform = static_cast<ctrls::form*>(parent_->nanawdg.get());
		pform->append(*ctrl->nanawdg);
	}
	else
	{
		auto playout = static_cast<ctrls::layout*>(parent_->nanawdg.get());
		playout->append(*ctrl->nanawdg);
	}

	// events
	control_struct_ptr pctrl = ctrl;
	ctrl->nanawdg->events().mouse_enter([this, pctrl]()
	{
		if(!pctrl.lock()->ishighlighted && cursor().action != cursor_action::select)
		{
			pctrl.lock()->set_highlight(HIGHLIGHT_ON_PLACE);
		}
	});

	ctrl->nanawdg->events().mouse_leave([this, pctrl]()
	{
		if(pctrl.lock()->ishighlighted)
		{
			pctrl.lock()->reset_highlight();
		}
	});

	ctrl->nanawdg->events().click([this, pctrl](const nana::arg_click & arg)
	{
		clickctrl(pctrl.lock());
	});

	return _registerobject(ctrl, parent);
}


tree_node<control_struct>* guimanager::addcommonctrl(tree_node<control_struct>* parent, const std::string& type, const std::string& name)
{
	control_struct parent_ = parent->value;
	if(!_checksonship(type, parent_->properties.property("type").as_string()))
		return 0;

	control_struct ctrl = control_struct(new ctrls::ctrl_struct());
	std::string name_ = name.empty() ? _name_mgr.add_numbered(type) : name;

	// nana::widget + properties
	if(type == CTRL_PANEL)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::panel(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_BUTTON)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::button(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_LABEL)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::label(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_TEXTBOX)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::textbox(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_COMBOX)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::combox(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_SPINBOX)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::spinbox(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_LISTBOX)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::listbox(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_CHECKBOX)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::checkbox(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_DATECHOOSER)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::date_chooser(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_TOOLBAR)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::toolbar(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_CATEGORIZE)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::categorize(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_GROUP)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::group(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_MENUBAR)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::menubar(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_PICTURE)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::picture(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_PROGRESS)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::progress(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_SLIDER)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::slider(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else if(type == CTRL_TABBAR)
	{
		ctrl->nanawdg = std::unique_ptr<nana::widget>(new ctrls::tabbar(*parent_->nanawdg, &ctrl->properties, name_));
	}
	else
		return 0;

	ctrls::layout* playout = static_cast<ctrls::layout*>(parent_->nanawdg.get());
	playout->append(*ctrl->nanawdg);

	// events
	control_struct_ptr pctrl = ctrl;
	if(type == CTRL_PANEL)
	{
		ctrl->nanawdg->events().mouse_enter([this, pctrl]()
		{
			auto ppanel = static_cast<ctrls::panel*>(pctrl.lock()->nanawdg.get());
			if(ppanel->haschild())
				return; // already has a child

			if(!pctrl.lock()->ishighlighted && cursor().action != cursor_action::select && cursor().type == CTRL_LAYOUT)
			{
				pctrl.lock()->set_highlight(HIGHLIGHT_ON_PLACE);
			}
		});

		ctrl->nanawdg->events().mouse_leave([this, pctrl]()
		{
			if(pctrl.lock()->ishighlighted)
			{
				pctrl.lock()->reset_highlight();
			}
		});
	}

	ctrl->nanawdg->events().click.connect_front([this, pctrl](const nana::arg_click & arg)
	{
		clickctrl(pctrl.lock());
	});

	return _registerobject(ctrl, parent);
}


void guimanager::deleteselected()
{
	if(!_selected)
		return;

	
	// delete objectspanel item
	_op->remove(_selected->value->properties.property("name").as_string());


	auto toremove = _selected;
	_selected = 0;


	// delete ctrl name
	_ctrls.for_each(toremove, [this](tree_node<control_struct>* node) -> bool
	{
		_name_mgr.remove(node->value->properties.property("name").as_string());
		return true;
	});
	

	// delete ctrl
	if(toremove->owner)
	{
		control_struct parent_ = toremove->owner->value;
		if(parent_)
		{
			if(parent_->properties.property("type").as_string() == CTRL_PANEL)
			{
				auto ppanel = static_cast<ctrls::panel*>(parent_->nanawdg.get());
				ppanel->remove(*toremove->value->nanawdg);
			}
			else if(parent_->properties.property("type").as_string() == CTRL_FORM)
			{
				auto pform = static_cast<ctrls::form*>(parent_->nanawdg.get());
				pform->remove(*toremove->value->nanawdg);
			}
			else
			{
				ctrls::layout* playout = static_cast<ctrls::layout*>(parent_->nanawdg.get());
				playout->remove(*toremove->value->nanawdg);
			}
		}
	}

	auto parent = toremove->owner;

	if(toremove == _ctrls.get_root()->child)
	{
		parent = 0;
		_pp->set(0);
		_main_wd->remove(*toremove->value->nanawdg);
	}

	_ctrls.remove(toremove);

	printf("\nREMOVED:\n");
	// delete ctrl name
	_ctrls.for_each([this](tree_node<control_struct>* node) -> bool
	{
		printf("- %s\n", node->value->properties.property("name").as_string().c_str());
		return true;
	});


	if(parent)
	{
		// select parent
		clickctrl(parent->value);
	}
}


void guimanager::moveupselected()
{
	if(!_selected)
		return;

	// move one position up
	if(!_ctrls.move_before_sibling(_selected))
		return;

	// if here the parent is a layout with at least 2 children
	ctrls::layout* playout = static_cast<ctrls::layout*>(_selected->owner->value->nanawdg.get());
	playout->moveup(*_selected->value->nanawdg);

	// reorder objectspanel item
	_op->move_before_sibling(_selected->value->properties.property("name").as_string());
}


void guimanager::movedownselected()
{
	if(!_selected)
		return;

	// move one position down
	if(!_ctrls.move_after_sibling(_selected))
		return;

	// if here the parent is a layout with at least 2 children
	ctrls::layout* playout = static_cast<ctrls::layout*>(_selected->owner->value->nanawdg.get());
	playout->movedown(*_selected->value->nanawdg);

	// reorder objectspanel item
	_op->move_after_sibling(_selected->value->properties.property("name").as_string());
}


void guimanager::clickctrl(control_struct ctrl)
{
	if(cursor().action == cursor_action::select)
	{
		// search control to select
		_ctrls.for_each([this, &ctrl](tree_node<control_struct>* node) -> bool
		{
			if(node->value == ctrl)
			{
				_selected = node;
				return false;
			}

			return true;
		});

		// select objectspanel item
		_op->select(ctrl->properties.property("name").as_string());

		// set properties panel
		_pp->set(&ctrl->properties);
		return;
	}

	_ap->deselect();

	std::string type = ctrl->properties.property("type").as_string();
	if(type == CTRL_PANEL || type == CTRL_FORM)
	{
		if(cursor().type == CTRL_LAYOUT)
		{
			if(type == CTRL_PANEL)
			{
				auto ppanel = static_cast<ctrls::panel*>(ctrl->nanawdg.get());
				if(ppanel->haschild())
					return; // already has a child
			}
			else if(type == CTRL_FORM)
			{
				auto pform = static_cast<ctrls::form*>(ctrl->nanawdg.get());
				if(pform->haschild())
					return; // already has a child
			}

			ctrl->reset_highlight();

			control_struct_ptr pctrl = ctrl;
			_ctrls.for_each([this, pctrl](tree_node<control_struct>* node) -> bool
			{
				if(node->value == pctrl.lock())
				{
					addlayout(node, ctrls::layout_orientation::horizontal);
					return false;
				}

				return true;
			});
		}
		else
		{
			nana::msgbox m(_root_wd, CREATOR_NAME, nana::msgbox::ok);
			m.icon(m.icon_warning);
			m << "Impossible to create the control. Did you forget to add a layout?";
			m();
		}
		return;
	}
	
	if(type == CTRL_LAYOUT)
	{
		auto playout = static_cast<ctrls::layout*>(ctrl->nanawdg.get());

		ctrl->reset_highlight();

		control_struct_ptr pctrl = ctrl;
		_ctrls.for_each([this, pctrl](tree_node<control_struct>* node) -> bool
		{
			if(node->value == pctrl.lock())
			{
				if(cursor().type == CTRL_LAYOUT)
					addlayout(node, ctrls::layout_orientation::horizontal);
				else
					addcommonctrl(node, cursor().type);
				return false;
			}

			return true;
		});

		return;
	}
}


void guimanager::clickobjectspanel(const std::string& name)
{
	_ctrls.for_each([this, name](tree_node<control_struct>* node) -> bool
	{
		if(node->value->properties.property("name").as_string() == name)
		{
			_selected = node;

			// set properties panel
			_pp->set(&node->value->properties);

			// set focus to new object
			node->value->nanawdg->focus();

			return false;
		}

		return true;
	});
}


void guimanager::serialize(pugi::xml_node* xml_parent)
{
	serialize(0, xml_parent);
}

void guimanager::serialize(tree_node<control_struct>* node, pugi::xml_node* xml_parent)
{
	pugi::xml_node xml_child;
	if(!node)
	{
		node = _ctrls.get_root();
		xml_child = *xml_parent;
	}
	else
	{
		xml_child = xml_parent->append_child(node->value->properties.property("type").as_string().c_str());

		for(size_t i = 0; i < node->value->properties.count(); ++i)
			xml_child.append_attribute(node->value->properties[i].name().c_str()) = node->value->properties[i].as_string().c_str();
	}

	if(node->child)
		serialize(node->child, &xml_child);

	if(node->next)
		serialize(node->next, xml_parent);
}


bool guimanager::deserialize(pugi::xml_node* xml_parent)
{
	lock_guard des_lock(&_deserializing, true);
	
	_op->emit_events(false);
	_op->auto_draw(false);
	bool ret_val = deserialize(0, xml_parent);
	_op->auto_draw(true);
	_op->emit_events(true);

	if(!ret_val)
		return false;

	if(_ctrls.get_root()->child)
	{
		// show main panel
		_ctrls.get_root()->child->value->nanawdg->show();

		// select main panel
		clickctrl(_ctrls.get_root()->child->value);
	}
	return true;
}

bool guimanager::deserialize(tree_node<control_struct>* parent, pugi::xml_node* xml_parent)
{
	//ATTENTION: No check on deserialize


	// read children
	for(pugi::xml_node xml_node = xml_parent->first_child(); xml_node; xml_node = xml_node.next_sibling())
	{
		tree_node<control_struct>* node = 0;

		std::string node_name = xml_node.name();
		std::string ctrl_name = xml_node.attribute("name").as_string();


		if(node_name == CTRL_LAYOUT)
			node = addlayout(parent, static_cast<ctrls::layout_orientation>(xml_node.attribute("layout").as_int()), ctrl_name);
		else if(xml_node.attribute("mainclass").as_bool())
		{
			if(node_name == CTRL_FORM)
				node = addmainform(ctrl_name);
			else if(node_name == CTRL_PANEL)
				node = addmainpanel(ctrl_name);
		}
		else
			node = addcommonctrl(parent, node_name, ctrl_name);

		if(node == 0)
		{
			std::cout << "UNKNOWN NODE: " << xml_node.name() << std::endl;
			continue;
		}


		// add name to name manager
		_name_mgr.add(ctrl_name);

		// deserialize properties
		_deserializeproperties(&node->value->properties, &xml_node);

		// update nana::widget
		_updatectrl(node);


		// deserialize children
		if(node_name == CTRL_LAYOUT || node_name == CTRL_PANEL || node_name == CTRL_FORM)
		{
			if(!deserialize(node, &xml_node))
				return false;
		}
	}

	return true;
}


bool guimanager::_checksonship(const std::string& child, const std::string& parent)
{
	if(parent == CTRL_LAYOUT)
		return true;
	if(parent == CTRL_FORM && child == CTRL_LAYOUT)
		return true;
	if(parent == CTRL_PANEL && child == CTRL_LAYOUT)
		return true;

	return false;
}


tree_node<control_struct>* guimanager::_registerobject(control_struct ctrl, tree_node<control_struct>* parent)
{
	// append to controls tree
	tree_node<control_struct>* child = _ctrls.append(parent, ctrl);

	_updatectrl(child);

	// append to objectspanel
	std::string type = ctrl->properties.property("type").as_string();
	if(parent)
	{
		control_struct parent_ = parent->value;
		_op->append(parent_->properties.property("name").as_string(), ctrl->properties.property("name").as_string(), type);
	}
	else
		_op->append("", ctrl->properties.property("name").as_string(), type);


	if(!_deserializing)
	{
		// set properties panel
		_pp->set(&ctrl->properties);

		// set focus to new object
		ctrl->nanawdg->focus();
	}

	// select new control
	_selected = child;

	// reset mouse cursor
	cursor(cursor_state{ cursor_action::select });

	return _selected;
}


void guimanager::_deserializeproperties(ctrls::properties_collection* properties, pugi::xml_node* xml_node)
{
	for(auto i = xml_node->attributes_begin(); i != xml_node->attributes_end(); ++i)
		properties->property(i->name()) = i->value();
}


bool guimanager::_updatectrlname(ctrls::properties_collection* properties, const std::string& new_name)
{
	if(properties->property("name").as_string() == new_name)
		return false;

	// update name manager
	if(!_name_mgr.add(new_name))
		return false;
	_name_mgr.remove(properties->property("name").as_string());


	// update objects panel
	_op->change_name(properties->property("name").as_string(), new_name, properties->property("type").as_string());


	// update properties
	properties->property("name") = new_name;
	return true;
}


void guimanager::_updatectrl(tree_node<control_struct>* node, bool update_owner, bool update_children)
{
	auto ctrl = node->value;

	std::string type = ctrl->properties.property("type").as_string();
	if(type == CTRL_LAYOUT)
	{
		static_cast<ctrls::layout*>(ctrl->nanawdg.get())->update(&ctrl->properties);
		// update children ctrls
		if(update_children)
			_updatechildrenctrls(node);
	}
	else if(type == CTRL_FORM)
	{
		static_cast<ctrls::form*>(ctrl->nanawdg.get())->update(&ctrl->properties);
		// update children ctrls
		if(update_children)
			_updatechildrenctrls(node);
	}
	else if(type == CTRL_PANEL)
	{
		static_cast<ctrls::panel*>(ctrl->nanawdg.get())->update(&ctrl->properties);
		// update children ctrls
		if(update_children)
			_updatechildrenctrls(node);
	}
	else if(type == CTRL_BUTTON)
	{
		static_cast<ctrls::button*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_LABEL)
	{
		static_cast<ctrls::label*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_TEXTBOX)
	{
		static_cast<ctrls::textbox*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_COMBOX)
	{
		static_cast<ctrls::combox*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_SPINBOX)
	{
		static_cast<ctrls::spinbox*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_LISTBOX)
	{
		static_cast<ctrls::listbox*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_CHECKBOX)
	{
		static_cast<ctrls::checkbox*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_DATECHOOSER)
	{
		static_cast<ctrls::date_chooser*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_TOOLBAR)
	{
		static_cast<ctrls::toolbar*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_CATEGORIZE)
	{
		static_cast<ctrls::categorize*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_GROUP)
	{
		static_cast<ctrls::group*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_MENUBAR)
	{
		static_cast<ctrls::menubar*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_PICTURE)
	{
		static_cast<ctrls::picture*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_PROGRESS)
	{
		static_cast<ctrls::progress*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_SLIDER)
	{
		static_cast<ctrls::slider*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	else if(type == CTRL_TABBAR)
	{
		static_cast<ctrls::tabbar*>(ctrl->nanawdg.get())->update(&ctrl->properties);
	}
	
	// update parent ctrl
	if(node->owner && update_owner)
		_updateparentctrl(node);
}

void guimanager::_updateparentctrl(tree_node<control_struct>* node)
{
	auto ctrl = node->value;
	if(ctrl->properties.property("mainclass").as_bool())
		return;

	if(!node->owner)
		return;

	auto parent = node->owner->value;
	std::string type = parent->properties.property("type").as_string();
	if(type == CTRL_LAYOUT)
	{
		std::string weight = ctrl->properties.property("weight").as_string();
		if(weight[0] == '-')
			weight = "";

		std::string margin = ctrl->properties.property("margin").as_string();

		static_cast<ctrls::layout*>(parent->nanawdg.get())->updatefield(*ctrl->nanawdg, weight, margin);
	}
}

void guimanager::_updatechildrenctrls(tree_node<control_struct>* node)
{
	auto* this_node = node;
	_ctrls.for_each(node, [this, this_node](tree_node<control_struct>* node) -> bool
	{
		if(this_node == node)
			return true;

		_updatectrl(node, false, false);
		return true;
	});
}