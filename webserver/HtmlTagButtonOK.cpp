#include <sstream>

#include "HtmlTagButtonOK.h"

namespace webserver
{
	HtmlTagButtonOK::HtmlTagButtonOK()
	:	HtmlTagButton("OK", "popup_ok")
	{
		std::stringstream s;
		s <<
			"function sleep(secs) { secs = (+new Date) + secs *1000; while ((+new Date) < secs); }\n"
			"$(function() {\n"
			" $('#editform').on('submit', function() {\n"
			"  $.ajax({\n"
			"   data: $(this).serialize(),\n"
			"   type: $(this).attr('get'),\n"
			"   url: $(this).attr('/'),\n"
			"   success: function(response) {\n"
			"    $('#popup').html(response);\n"
			"   }\n"
			"  })\n"
			"  setTimeout(function() {\n"
			"   $('#popup').hide(300);\n"
			"  }, 1500);\n"
			"  return false;\n"
			" });\n"
			"});\n"
			"$(function() {\n"
			" $('#" << commandID << "').on('click', function() {\n"
			"  $('#editform').submit();\n"
			"  return false;\n"
			" });\n"
			"});\n";
		AddJavaScript(s.str());
	}
};
