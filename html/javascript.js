function loadDivFromForm(form, target) {
 var formName = '#' + form;
 $.ajax({
  data: $(formName).serialize(),
  type: $(formName).attr('get'),
  url: $(formName).attr('/'),
  success: function(response) {
  var targetName = '#' + target;
   $(targetName).html(response);
  }
 })
 return false;
}

function reloadProtocol() {
 console.log('X');
}
