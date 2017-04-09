function loadFormToDiv(form, target) {
  debugger;
	var formName = '#' + form;
  $.ajax({
  data: $(formName).serialize(),
   type: $(formName).attr('get'),
   url: $(formName).attr('/'),
   success: function(response) {
    var targetName = '#' + target;
		debugger;
		console.log(targetName);
		alert(targetName);
		alert(response);
    $(targetName).html(response);
   }
  })
  return false;
}

function onChange(buttonID, formID) {
  debugger;
 $(function() {
  $('#' + buttonID).on('change', submitForm(formID));
	return false;
 });
}

function submitForm(formID) {
  debugger;
  $('#' + formID).submit();
  return false;
}

