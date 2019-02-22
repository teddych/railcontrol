function addRelation()
{
	var relationCounter = document.getElementById('relationcounter');
	if (!relationCounter)
	{
		return false;
	}
	var relationDiv = document.getElementById('relation');
	if (!relationDiv)
	{
		return false;
	}

	relationCounter.value++;
	var url = '/?cmd=relationadd&priority=' + relationCounter.value;
	requestAddItem('new_priority_' + relationCounter.value, url);
	return false;
}

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

function deleteElement(elementName)
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
	return false;
}

function onContextAccessory(event, accessoryID)
{
	if (event.shiftKey)
	{
		return true;
	}
	event.stopPropagation();
	hideAllContextMenus();
	var menu = document.getElementById('a_' + accessoryID + '_context');
	if (menu)
	{
		menu.style.display = 'block';
	}
	return false;
}

function onClickStreet(streetID)
{
	var element = document.getElementById('st_' + streetID);
	var url = '/?cmd=streetexecute';
	url += '&street=' + streetID;
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
	return false;
}

function onContextStreet(event, streetID)
{
	if (event.shiftKey)
	{
		return true;
	}
	event.stopPropagation();
	hideAllContextMenus();
	var menu = document.getElementById('st_' + streetID + '_context');
	if (menu)
	{
		menu.style.display = 'block';
	}
	return false;
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
	return false;
}

function onContextSwitch(event, switchID)
{
	if (event.shiftKey)
	{
		return true;
	}
	event.stopPropagation();
	hideAllContextMenus();
	var menu = document.getElementById('sw_' + switchID + '_context');
	if (menu)
	{
		menu.style.display = 'block';
	}
	return false;
}

function onContextTrack(event, trackID)
{
	if (event.shiftKey)
	{
		return true;
	}
	event.stopPropagation();
	hideAllContextMenus();
	var menu = document.getElementById('t_' + trackID + '_context');
	if (menu)
	{
		menu.style.display = 'block';
	}
	return false;
}

function onChangeCheckboxShowHide(checkboxId, divId)
{
	var checkbox = document.getElementById(checkboxId);
	if (!checkbox)
	{
		return;
	}
	var div = document.getElementById(divId);
	if (!div)
	{
		return;
	}
	div.hidden = !checkbox.checked;
}

function updateLayoutItem(elementName, data)
{
	var parentElement = document.getElementById('layout');
	if (parentElement)
	{
		var elementContextName = elementName + '_context';
		deleteElement(elementName);
		deleteElement(elementContextName);
		parentElement.innerHTML += data;
		var i;
		var tags = document.getElementsByClassName('layout_item');
		for (i = 0; i < tags.length; i++)
		{
			var tag = tags[i];
			var clone = tag.cloneNode(true);
			tag.parentNode.replaceChild(clone, tag);
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

function addItem(elementName, data)
{
	var element = document.getElementById(elementName);
	if (!element)
	{
		return;
	}
	element.innerHTML += data;
}

function requestAddItem(elementName, url)
{
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() {
		if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
		{
			addItem(elementName, xmlHttp.responseText);
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
	else if (command == 'streetsettings')
	{
		var streetID = argumentMap.get('street');
		elementName = 'st_' + streetID;
		var url = '/?cmd=streetget';
		url += '&street=' + streetID;
		requestUpdateLayoutItem(elementName, url);
	}
	else if (command == 'streetdelete')
	{
		elementName = 'st_' + argumentMap.get('street');
		deleteElement(elementName);
		deleteElement(elementName + '_context');
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
		deleteElement(elementName);
		deleteElement(elementName + '_context');
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
		deleteElement(elementName);
		deleteElement(elementName + '_context');
	}
}

window.layoutPosX = 0;
window.layoutPosY = 0;

function loadPopup(url)
{
	$('#popup').show(300);
	url += '&posx=' + window.layoutPosX + '&posy=' + window.layoutPosY;
	$('#popup').load(url);
}

function loadLocoSelector()
{
	var elementName = 'loco_selector';
	var url = '/?cmd=locoselector';
	requestUpdateItem(elementName, url);
}

function loadLayerSelector()
{
	var elementName = 'layer_selector';
	var url = '/?cmd=layerselector';
	requestUpdateItem(elementName, url);
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
	var elementName = 'select_protocol';
	var url = '/?cmd=protocol' + type;
	url += '&control=' + controlID;
	url += '&' + type + '=' + ID;
	requestUpdateItem(elementName, url);
}

function loadLayoutContext(event)
{
	if (event.shiftKey)
	{
		return true;
	}
	event.preventDefault();
	hideAllContextMenus();
	menu = document.getElementById('layout_context');
	if (!menu)
	{
		return true;
	}
	menu.style.display = 'block';
	menu.style.left = event.pageX + 'px';
	menu.style.top = event.pageY + 'px';
	window.layoutPosX = Math.floor((event.pageX - 254) / 35);
	window.layoutPosY = Math.floor((event.pageY - 92) / 35);
	return true;
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

function loadLoco()
{
	var loco = document.getElementById('s_loco');
	if (loco)
	{
		requestUpdateItem('loco', '/?cmd=loco&loco=' + loco.value);
	}
}

function loadLayout()
{
	var layer = document.getElementById('s_layer');
	if (layer)
	{
		requestUpdateItem('layout', '/?cmd=layout&layer=' + layer.value);
	}
}

function startUp()
{
	var body = document.getElementById('body');
	if (body)
	{
		body.onclick = function(event) {
			if (event.button == 2)
			{
				return false;
			}
			hideAllContextMenus();
			return true;
		};
	}
	loadLoco();
	loadLayout();
}

function ShowTab(tabName)
{
	var tabs = document.getElementsByClassName('tab_content');
	if (!tabs)
	{
		return;
	}

	for (var i = 0; i < tabs.length; ++i)
	{
		var tab = tabs[i];
		tab.classList.add('hidden');
	}

	var tab = document.getElementById('tab_' + tabName);
	if (!tab)
	{
		return;
	}
	tab.classList.remove('hidden');

	var tabButtons = document.getElementsByClassName('tab_button');
	if (!tabButtons)
	{
		return;
	}

	for (var i = 0; i < tabButtons.length; ++i)
	{
		var tabButton = tabButtons[i];
		tabButton.classList.remove('tab_button_selected');
	}

	var tabButton = document.getElementById('tab_button_' + tabName);
	if (!tabButton)
	{
		return;
	}
	tabButton.classList.add('tab_button_selected');
}

function submitEditForm()
{
	$.ajax({
		data: $('#editform').serialize(),
		type: $('#editform').attr('get'),
		url: $('#editform').attr('/'),
		success: function(response) {
			$('#popup').html(response);
		}
	});
	setTimeout(function() {
		$('#popup').hide(300);
	}, 1500);
	return false;
}

function fireRequestAndForget(url)
{
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
}

