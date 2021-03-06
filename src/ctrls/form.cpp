/*
 *		ctrls::form Implementation
 *
 *      Nana C++ Library - Creator
 *      Author: besh81
 */

#include "config.h"
#include <iostream>
#include "ctrls/form.h"
#include "nana_extra/color_helper.h"
#include "filemanager.h"


extern filemanager		g_file_mgr;


namespace ctrls
{

	//form
	form::form(nana::window wd, const std::string& name, bool visible)
		: ctrl()
	{
		frm.create(wd, visible);
		boxmodel.bind(frm);

		// form IS ALWAYS mainclass
		properties.append("mainclass") = true;
		// project
		properties.append("filename").label("File Name").category(CAT_CPPCODE).type(pg_type::string) = DEFAULT_FILENAME;
		properties.append("rel_path").label("Relative Path").category(CAT_CPPCODE).type(pg_type::check) = true;
		properties.append("work_dir").label("Working Dir").category(CAT_CPPCODE).type(pg_type::folder).enabled("rel_path", true) = "";

		ctrl::init(&frm, CTRL_FORM, name, false);

		// common
		properties.append("caption").label("Caption").category(CAT_COMMON).type(pg_type::string) = "Form";
		properties.append("width").label("Width").category(CAT_COMMON).type(pg_type::string_uint) = MAIN_WDG_W;
		properties.append("height").label("Height").category(CAT_COMMON).type(pg_type::string_uint) = MAIN_WDG_H;
		// appearance
		properties.remove("bgcolor");
		properties.remove("fgcolor");
		properties.append("bgcolor").label("Background").category(CAT_APPEARANCE).type(pg_type::color) = nana::to_string(nanawdg->bgcolor(), false);
		properties.append("fgcolor").label("Foreground").category(CAT_APPEARANCE).type(pg_type::color) = nana::to_string(nanawdg->fgcolor(), false);
		properties.append("decoration").label("Decoration").category(CAT_APPEARANCE).type(pg_type::check) = true;
		properties.append("taskbar").label("Taskbar").category(CAT_APPEARANCE).type(pg_type::check) = true;
		properties.append("floating").label("Floating").category(CAT_APPEARANCE).type(pg_type::check) = false;
		properties.append("no_activate").label("NoActivate").category(CAT_APPEARANCE).type(pg_type::check) = false;
		properties.append("min").label("Min").category(CAT_APPEARANCE).type(pg_type::check) = false;
		properties.append("max").label("Max").category(CAT_APPEARANCE).type(pg_type::check) = false;
		properties.append("sizable").label("Sizable").category(CAT_APPEARANCE).type(pg_type::check) = false;
		// layout
		properties.remove("weight");
		properties.append("layout").label("Layout").category(CAT_LAYOUT).type(pg_type::layout) = static_cast<int>(layout_orientation::horizontal);
		properties.append("margin").label("Margin").category(CAT_LAYOUT).type(pg_type::string_uint) = 5;
		properties.append("gap").label("Gap").category(CAT_LAYOUT).type(pg_type::string_uint) = 2;
	}


	void form::update()
	{
		//ctrl::update();

		// manage absolute/relative path
		g_file_mgr.enable_relative(properties.property("rel_path").as_bool());
		g_file_mgr.basedir(properties.property("work_dir").as_string());

		auto pw = nana::API::get_widget(nanawdg->parent());
		pw = nana::API::get_widget(pw->parent());
		bool inherited;
		nana::color col;

		if(!properties.property("enabled").as_bool())
			nanawdg->enabled(properties.property("enabled").as_bool());

		col = nana::to_color(properties.property("bgcolor").as_string(), inherited);
		nanawdg->bgcolor(inherited ? pw->bgcolor() : col);

		col = nana::to_color(properties.property("fgcolor").as_string(), inherited);
		nanawdg->fgcolor(inherited ? pw->fgcolor() : col);

		frm.size(nana::size(properties.property("width").as_uint(), properties.property("height").as_uint()));


		boxmodel.set_type(static_cast<layout_orientation>(properties.property("layout").as_int()));
		boxmodel.set_attributes("", properties.property("margin").as_string(), properties.property("gap").as_string());
		boxmodel.update();
	}


	void form::generatecode(code_data_struct* cd, code_info_struct* ci)
	{
		//ctrl::generatecode(cd, ci);

		// form IS ALWAYS mainclass
		auto name = properties.property("name").as_string();

		// mainclass definition
		std::string sz = properties.property("width").as_string() + ", " + properties.property("height").as_string();
		std::string apr = properties.property("decoration").as_string() + ", " + properties.property("taskbar").as_string() + ", ";
		apr.append(properties.property("floating").as_string() + ", " + properties.property("no_activate").as_string() + ", ");
		apr.append(properties.property("min").as_string() + ", " + properties.property("max").as_string() + ", " + properties.property("sizable").as_string());

		cd->mainclass = name;
		cd->mainclass_base = "nana::form";
		cd->mainclass_ctor = "(nana::window wd, const ::nana::size& sz = {" + sz + "}, const nana::appearance& apr = {" + apr + "})";
		cd->mainclass_base_ctor = "(wd, sz, apr)";

		// filename
		cd->filename = properties.property("filename").as_string();

		// headers
		cd->hpps.add("#include <nana/gui.hpp>");
		cd->hpps.add("#include <nana/gui/place.hpp>");
		// declaration
		cd->decl.push_back("nana::place _place{ *this };");
		// init
		cd->init.push_back("_place.div(\"" + boxmodel.get(DEFAULT_FIELD, true) + "\");");
		if(!properties.property("enabled").as_bool())
			cd->init.push_back("enabled(" + properties.property("enabled").as_string() + ");");
		cd->init.push_back("caption(\"" + properties.property("caption").as_string() + "\");");
		generatecode_colors(cd, ci);
		// placement
		// ...
		// collocate
		cd->init_post.push_back("_place.collocate();");
		// children
		ci->place = "_place";
	}


	void form::update_children_info(nana::window child, const std::string& divtext, const std::string& weight)
	{
		boxmodel.update_children_info(child, divtext, weight);
	}


	bool form::children()
	{
		return boxmodel.children();
	}


	bool form::children_fields()
	{
		return boxmodel.children_fields();
	}


	bool form::append(nana::window child)
	{
		return boxmodel.append(child);
	}


	bool form::insert(nana::window pos, nana::window ctrl, bool after)
	{
		return boxmodel.insert(pos, ctrl, after);
	}


	bool form::remove(nana::window child)
	{
		return boxmodel.remove(child);
	}


	bool form::moveup(nana::window child)
	{
		return boxmodel.moveup(child);
	}


	bool form::movedown(nana::window child)
	{
		return boxmodel.movedown(child);
	}

}//end namespace ctrls
