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

function deleteLayoutElement(elementName)
{
	var element = document.getElementById(elementName);
	if (element)
	{
		element.parentNode.removeChild(element);
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

	var elementName = "";
	var command = argumentMap.get('command');
	if (command == 'booster')
	{
		elementName = 'b_booster';
		var on = argumentMap.get('on');
		setToggleButton(elementName, on);
	}
	else if (command == 'locospeed')
	{
		elementName = 'locospeed_' + argumentMap.get('loco');
		var element = document.getElementById(elementName);
		if (element)
		{
			element.value = argumentMap.get('speed');
		}
	}
	else if (command == 'locofunction')
	{
		elementName = 'b_locofunction_' + argumentMap.get('loco') + '_' + argumentMap.get('function');
		var on = argumentMap.get('on');
		setToggleButton(elementName, on);
	}
	else if (command == 'locodirection')
	{
		elementName = 'b_locodirection_' + argumentMap.get('loco');
		var on = argumentMap.get('direction');
		setToggleButton(elementName, on);
	}
	else if (command == 'accessory')
	{
		elementName = 'a_' + argumentMap.get('accessory');
		var element = document.getElementById(elementName);
		if (element)
		{
			if (argumentMap.has('state'))
			{
				var state = argumentMap.get('state');
				if (state == 'green')
				{
					element.classList.remove('accessory_off');
					element.classList.add('accessory_on');
				} else {
					element.classList.remove('accessory_on');
					element.classList.add('accessory_off');
				}
			}
		}
	}
	else if (command == 'accessorydelete')
	{
		elementName = 'a_' + argumentMap.get('accessory');
		var element = document.getElementById(elementName);
		if (element)
		{
			element.parentNode.removeChild(element);
		}
	}
	else if (command == 'switch')
	{
		elementName = 'sw_' + argumentMap.get('switch');
		var element = document.getElementById(elementName);
		if (element)
		{
			var state = argumentMap.get('state');
			if (state == 'straight')
			{
				element.classList.remove('switch_turnout');
				element.classList.add('switch_straight');
			} else {
				element.classList.remove('switch_straight');
				element.classList.add('switch_turnout');
			}
		}
	}
	else if (command == 'switchdelete')
	{
		elementName = 'sw_' + argumentMap.get('switch');
		deleteLayoutElement(elementName);
		deleteLayoutElement(elementName + '_context');
	}

	if (argumentMap.has('posx') && argumentMap.has('posy'))
	{
		var element = document.getElementById(elementName);
		if (element)
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

	if (argumentMap.has('rotation'))
	{
		var rotation = argumentMap.get('rotation');
		var imgElement = document.getElementById(elementName + '_img');
		if (imgElement)
		{
			imgElement.style.transform = 'rotate(' + rotation + 'deg)';
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
