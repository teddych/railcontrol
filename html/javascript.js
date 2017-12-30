function loadLoco(form) {
	var formName = '#' + form;
  $.ajax({
   data: $(formName).serialize(),
   type: $(formName).attr('get'),
   url: $(formName).attr('/'),
   success: function(response) {
    var targetName = '#loco';
		console.log(targetName);
    $(targetName).html(response);
   }
  })
  return false;
}

