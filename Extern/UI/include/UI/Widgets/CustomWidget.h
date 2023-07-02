#pragma once

#include <string>

#include <../Tools/Event.h>

#include "UI/Widgets/AWidget.h"

namespace UI::Widgets {
	class CustomWidget :public AWidget {
	protected:
		void _Draw_Impl() {
			DrawIn.Invoke();
		}

	public:
		Tools::Eventing::Event<> DrawIn;
	};
		
}