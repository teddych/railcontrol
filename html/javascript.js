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

function loadPopup(url)
{
	$('#popup').show(300);
	$('#popup').load(url);
}

function isInLayout(position)
{
	layoutPosition = document.querySelector(".layout").getBoundingClientRect();
	return (position.pageX >= layoutPosition.left && position.pageX <= layoutPosition.right && position.pageY >= layoutPosition.top && position.pageY <= layoutPosition.bottom);
}

window.addEventListener("click", e => {
	if (e.button == 2)
	{
		return;
	}

	var menus = document.getElementsByClassName('contextmenu');
	for (var i = 0; i < menus.length; ++i)
	{
		menus[i].style.display = 'none';
	}
});

