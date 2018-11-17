function setToggleButton(elementName, on)
{
	var element = document.getElementById(elementName);
	if (element)
	{
		if (on == 'true')
		{
			element.classList.remove('button_off');
			element.classList.add('button_on');
		}
		else
		{
			element.classList.remove('button_on');
			element.classList.add('button_off');
		}
	}
}

function dataUpdate(event)
{
	var status = document.getElementById('status');
	var arguments = event.data.split(';');
	var argumentMap = new Map();
	arguments.forEach(function(argument) {
		var parts = argument.split('=');
		if (parts[0] == 'status')
		{
			status.innerHTML += parts[1] + '<br>';
			status.scrollTop = status.scrollHeight - status.clientHeight;
		}
		else
		{
			argumentMap.set(parts[0], parts[1]);
		}
	});

	if (argumentMap.get('command') == 'booster')
	{
		var elementName = 'b_booster';
		var on = argumentMap.get('on');
		setToggleButton(elementName, on);
	}
	else if (argumentMap.get('command') == 'locospeed')
	{
		var elementName = 'locospeed_' + argumentMap.get('loco');
		var element = document.getElementById(elementName);
		if (element)
		{
			element.value = argumentMap.get('speed');
		}
	}
	else if (argumentMap.get('command') == 'locofunction')
	{
		var elementName = 'b_locofunction_' + argumentMap.get('loco') + '_' + argumentMap.get('function');
		var on = argumentMap.get('on');
		setToggleButton(elementName, on);
	}
	else if (argumentMap.get('command') == 'locodirection')
	{
		var elementName = 'b_locodirection_' + argumentMap.get('loco');
		var on = argumentMap.get('direction');
		setToggleButton(elementName, on);
	}
	else if (argumentMap.get('command') == 'accessory')
	{
		var elementName = 'a_' + argumentMap.get('accessory');
		var element = document.getElementById(elementName);
		if (element)
		{
			if (argumentMap.has('state'))
			{
				var state = argumentMap.get('state');
				if (state == 'green')
				{
					element.classList.add('accessory_on');
				} else if (state == 'red')
				{
					element.classList.remove('accessory_on');
				}
			}
			if (argumentMap.has('posx'))
			{
				var posx = argumentMap.get('posx') * 35;
				var posy = argumentMap.get('posy') * 35;
				element.style.left = posx + 'px';
				element.style.top = posy + 'px';
				var contextElement = document.getElementById(elementName + '_context');
				if (contextElement)
				{
					contextElement.style.left = (posx + 5) + 'px';
					contextElement.style.top = (posy + 30) + 'px';
				}
			}
		}
	}
}

function loadDivFromForm(form, target)
{
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

window.layoutPosX = 0;
window.layoutPosY = 0;

function loadPopup(url)
{
	$('#popup').show(300);
	url += '&posx=' + window.layoutPosX + '&posy=' + window.layoutPosY;
	$('#popup').load(url);
}

function isInLayout(position)
{
	layoutPosition = document.querySelector('.layout').getBoundingClientRect();
	return (position.pageX >= layoutPosition.left && position.pageX <= layoutPosition.right && position.pageY >= layoutPosition.top && position.pageY <= layoutPosition.bottom);
}

function hideAllContextMenus()
{
	var menus = document.getElementsByClassName('contextmenu');
	for (var i = 0; i < menus.length; ++i)
	{
		menus[i].style.display = 'none';
	}
}

function startUp()
{
	$('#body').on('click', function(event) {
		if (event.button == 2)
		{
			return false;
		}
		hideAllContextMenus();
		return true;
	});
	loadDivFromForm('selectLoco_form', 'loco');
	loadDivFromForm('selectLayout_form', 'layout');
}
