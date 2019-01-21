function checkIntegerValue(name, min, max)
{
	if (min > max)
	{
		return;
	}
	var input = document.getElementById(name);
	if (!input)
	{
		return;
	}
	if (input.value < min)
	{
		input.value = min;
		return;
	}
	if (input.value > max)
	{
		input.value = max;
	}
}

function incrementIntegerValue(name, max)
{
	var input = document.getElementById(name);
	if (!input)
	{
		return;
	}
	var value = parseInt(input.value) + 1;
	if (value > max)
	{
		return;
	}
	input.value = value;
}

function decrementIntegerValue(name, min)
{
	var input = document.getElementById(name);
	if (!input)
	{
		return;
	}
	var value = parseInt(input.value) - 1;
	if (value < min)
	{
		return;
	}
	input.value = value;
}

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

function onClickAccessory(accessoryID)
{
	var element = document.getElementById('a_' + accessoryID);
	var url = '/?cmd=accessorystate';
	url += '&state=' + (element.classList.contains('accessory_off') ? 'on' : 'off');
	url += '&accessory=' + accessoryID;
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
}

function onContextAccessory(accessoryID)
{
	hideAllContextMenus();
	var menu = document.getElementById('a_' + accessoryID + '_context');
	if (menu)
	{
		menu.style.display = 'block';
	}
}

function onClickSwitch(switchID)
{
	var element = document.getElementById('sw_' + switchID);
	var url = '/?cmd=switchstate';
	url += '&state=' + (element.classList.contains('switch_straight') ? 'turnout' : 'straight');
	url += '&switch=' + switchID;
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
}

function onContextSwitch(switchID)
{
	hideAllContextMenus();
	var menu = document.getElementById('sw_' + switchID + '_context');
	if (menu)
	{
		menu.style.display = 'block';
	}
}

function onContextTrack(trackID)
{
	hideAllContextMenus();
	var menu = document.getElementById('t_' + trackID + '_context');
	if (menu)
	{
		menu.style.display = 'block';
	}
}

function updateLayoutItem(elementName, data)
{
	var parentElement = document.getElementById('layout');
	if (parentElement)
	{
		var elementContextName = elementName + '_context';
		deleteLayoutElement(elementName);
		deleteLayoutElement(elementContextName);
		parentElement.innerHTML += data;
		var i;
		var tags = document.getElementsByClassName('layout_item');
		for (i = 0; i < tags.length; i++)
		{
			var tag = tags[i];
			var clone = tag.cloneNode(true);
			tag.parentNode.replaceChild(clone, tag);
		}
		var scriptTags = document.getElementsByClassName('layout_item_script');
		for (i = 0; i < scriptTags.length; i++)
		{
			eval(scriptTags[i].innerHTML);
		}
	}
}

function requestUpdateLayoutItem(elementName, url)
{
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() {
		if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
		{
			updateLayoutItem(elementName, xmlHttp.responseText);
		}
	}
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
}

function updateItem(elementName, data)
{
	var element = document.getElementById(elementName);
	if (!element)
	{
		return;
	}
	element.innerHTML = data;
}

function requestUpdateItem(elementName, url)
{
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() {
		if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
		{
			updateItem(elementName, xmlHttp.responseText);
		}
	}
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
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
				}
				else
				{
					element.classList.remove('accessory_on');
					element.classList.add('accessory_off');
				}
			}
		}
	}
	else if (command == 'accessorysettings')
	{
		var accessoryID = argumentMap.get('accessory');
		elementName = 'a_' + accessoryID;
		var url = '/?cmd=accessoryget';
		url += '&accessory=' + accessoryID;
		requestUpdateLayoutItem(elementName, url);
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
			}
			else
			{
				element.classList.remove('switch_straight');
				element.classList.add('switch_turnout');
			}
		}
	}
	else if (command == 'switchsettings')
	{
		var switchID = argumentMap.get('switch');
		elementName = 'sw_' + switchID;
		var url = '/?cmd=switchget';
		url += '&switch=' + switchID;
		requestUpdateLayoutItem(elementName, url);
	}
	else if (command == 'switchdelete')
	{
		elementName = 'sw_' + argumentMap.get('switch');
		deleteLayoutElement(elementName);
		deleteLayoutElement(elementName + '_context');
	}
	else if (command == 'tracksettings')
	{
		var switchID = argumentMap.get('track');
		elementName = 't_' + switchID;
		var url = '/?cmd=trackget';
		url += '&track=' + switchID;
		requestUpdateLayoutItem(elementName, url);
	}
	else if (command == 'trackdelete')
	{
		elementName = 't_' + argumentMap.get('track');
		deleteLayoutElement(elementName);
		deleteLayoutElement(elementName + '_context');
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

function loadProtocol(type, ID)
{
	var selectControl = document.getElementById('s_control');
	if (!selectControl)
	{
		return;
	}
	var controlID = selectControl.value;
	var selectProtocol = document.getElementById('select_protocol');
	if (!selectProtocol)
	{
		return;
	}
	elementName = 'select_protocol';
	var url = '/?cmd=protocol' + type;
	url += '&control=' + controlID;
	url += '&' + type + '=' + ID;
	requestUpdateItem(elementName, url);
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
