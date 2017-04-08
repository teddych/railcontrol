function loadDiv(target) {
  debugger;
  $.ajax({
  data: $(this).serialize(),
   type: $(this).attr('get'),
   url: $(this).attr('/'),
   success: function(response) {
    var targetName = '#' + target;
		debugger;
		console.log(targetName);
		alert(targetName);
    $(targetName).html(response);
   }
  })
  return false;
}

function onChange(buttonID, formID) {
  debugger;
 $('#' + buttonID).on('change', submitForm(formID));
}

function submitForm(formID) {
  debugger;
  $('#' + formID).submit();
  return false;
}

