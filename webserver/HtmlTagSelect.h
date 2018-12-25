#pragma once

#include <atomic>
#include <map>
#include <string>

#include "webserver/HtmlTag.h"

namespace webserver
{
	class HtmlTagSelect : public HtmlTag
	{
		private:
			static std::atomic<unsigned int> selectID;
			const std::string commandID;

		public:
			HtmlTagSelect(const std::string& name, const std::map<std::string,std::string>& options, const std::string& defaultValue = "");
			// FIXME: add map with <int,string>
	};
};

