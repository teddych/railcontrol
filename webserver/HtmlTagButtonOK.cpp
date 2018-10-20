#include <sstream>

#include "webserver/HtmlTagButtonOK.h"

namespace webserver
{
	HtmlTagButtonOK::HtmlTagButtonOK()
	:	HtmlTagButton("OK", "popup_ok")
	{
		std::stringstream s;
		s <<
			"function sleep(secs) { secs = (+new Date) + secs *1000; while ((+new Date) < secs); }"
			"$(function() {"
			" $('#editform').on('submit', function() {"
			"  $.ajax({"
			"   data: $(this).serialize(),"
			"   type: $(this).attr('get'),"
			"   url: $(this).attr('/'),"
			"   success: function(response) {"
			"    $('#popup').html(response);"
			"   }"
			"  });"
			"  setTimeout(function() {"
			"   $('#popup').hide(300);"
			"  }, 1500);"
			"  return false;"
			" });"
			"});"
			"$(function() {"
			" $('#" << commandID << "').on('click', function() {"
			"  $('#editform').submit();"
			"  return false;"
			" });"
			"});";
		AddJavaScript(s.str());
	}
};
