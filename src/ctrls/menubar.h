/*
 *		ctrls::menubar Implementation
 *
 *      Nana C++ Library - Creator
 *      Author: besh81
 */

#ifndef NANA_CREATOR_MENUBAR_H
#define NANA_CREATOR_MENUBAR_H

#include <nana/gui/widgets/menubar.hpp>
#include "ctrls/ctrl.h"
#include "codegenerator_data.h"
#include "tree.h"


namespace ctrls
{

	class menubar
		: public ctrl
	{
	public:
		menubar(nana::window wd, const std::string& name);

		void update() override;

		void generatecode(code_data_struct* cd, code_info_struct* ci) override;


	protected:
		nana::menubar mnb;


		//utility data
		struct menu_data
		{
			std::string key;
			std::string text;
			bool		separator{ false };

			nana::menu*	submenu{ 0 };
			std::string submenu_name;
		};
		tree<menu_data> menu_tree;

		void create_menu_tree();
	};

}//end namespace ctrls

#endif //NANA_CREATOR_MENUBAR_H
