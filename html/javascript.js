function selectValue(key, value, elementId)
{
	var dropdown = document.getElementById('d_' + elementId);
	if (!dropdown)
	{
		return;
	}
	dropdown.classList.remove('show');
	var dropdownElements = dropdown.childNodes;
	for (var i = 0; i < dropdownElements.length; ++i)
	{
		var element = dropdownElements[i];
		element.classList.remove('selected_option');
		var attributes = element.attributes;
		if (attributes.key.value == key)
		{
			element.classList.add('selected_option');
		}
	}

	var textElement = document.getElementById('skip_' + elementId);
	if (!textElement)
	{
		return;
	}
	textElement.value = value;

	var keyElement = document.getElementById(elementId);
	if (!keyElement)
	{
		return;
	}
	keyElement.value = key;
	keyElement.onchange();
}

function selectMultipleValue(changedKey, elementId, none, several)
{
	var dropdown = document.getElementById('d_' + elementId);
	if (!dropdown)
	{
		return;
	}
	var keyElement = document.getElementById(elementId);
	if (!keyElement)
	{
		return;
	}
	var valueElement = document.getElementById('skip_' + elementId);
	if (!valueElement)
	{
		return;
	}
	var dropdownElements = dropdown.childNodes;
	var key = keyElement.value;
	var value = valueElement.value;
	for (var i = 0; i < dropdownElements.length; ++i)
	{
		var element = dropdownElements[i];
		var attributes = element.attributes;
		if (attributes.key.value != changedKey)
		{
			continue;
		}

		if (element.classList.contains('selected_option'))
		{
			key &= ~changedKey;
		}
		else
		{
			key |= changedKey;
		}
	}

	var optionsCount = 0;
	for (var i = 0; i < dropdownElements.length; ++i)
	{
		var element = dropdownElements[i];
		var attributes = element.attributes;
		if ((key & attributes.key.value) == attributes.key.value)
		{
			element.classList.add('selected_option');
			value = element.textContent;
			++optionsCount;
		}
		else
		{
			element.classList.remove('selected_option');
		}
	}

	var textElement = document.getElementById('skip_' + elementId);
	if (!textElement)
	{
		return;
	}

	switch(optionsCount)
	{
		case 0:
			textElement.value = none;
			break;

		case 1:
			textElement.value = value;
			break;

		default:
			textElement.value = several;
			break;
	}

	keyElement.value = key;
	keyElement.onchange();
}

window.addEventListener('click', function(event) {
	for (const select of document.querySelectorAll('.dropdown'))
	{
		if (!select.contains(event.target))
		{
			select.classList.remove('show');
		}
	}
}, true);

function toggleClass(elementId, name)
{
	var element = document.getElementById(elementId);
	if (!element)
	{
		return;
	}
	element.classList.toggle(name);
}

function modifierKeyPressed(event)
{
	return (event.shiftKey || event.ctrlKey || event.altKey);
}

function onClickWithoutMenu(event, identifier)
{
	if (!modifierKeyPressed(event))
	{
		return false;
	}
	rotateObject(identifier);
	return false;
}

function onClickWithMenu(event, identifier)
{
	if (!modifierKeyPressed(event))
	{
		return showOnClickMenu(event, identifier);
	}
	rotateObject(identifier);
	return false;
}

function rotateObject(identifier)
{
	var url = "/?cmd=rotate&" + parseObjectIdentifier(identifier);
	fireRequestAndForget(url);
}

function drag(event)
{
	if (!modifierKeyPressed(event))
	{
		event.dataTransfer.setData("Text", "");
		return;
	}
	event.dataTransfer.setData("Text", event.target.id);
}

function allowDrop(event)
{
	event.preventDefault();
}

function drop(event) {
	if (!modifierKeyPressed(event))
	{
		return;
	}
	event.preventDefault();
	var data = event.dataTransfer.getData("Text");
	var x = Math.round(event.target.scrollLeft + event.offsetX, 0);
	var y = Math.round(event.target.scrollTop + event.offsetY, 0);
	x = Math.floor(x / 36);
	y = Math.floor(y / 36);

	var url = "/?cmd=newposition&" + parseObjectIdentifier(data) + "&x=" + x + "&y=" + y;
	fireRequestAndForget(url);
}

function parseObjectIdentifier(identifier)
{
	var dataArray = identifier.split('_');
	var type;
	switch(dataArray[0])
	{
		case "t":
			type = "track";
			break;

		case "sw":
			type = "switch";
			break;

		case "si":
			type = "signal";
			break;

		case "f":
			type = "feedback";
			break;

		case "a":
			type = "accessory";
			break;

		case "r":
			type = "route";
			break;

		case "tx":
			type = "text";
			break;

		default:
			type = "object";
			break;
	}
	return type + "=" + dataArray[1];
}

function showMenuConfig()
{
	var menuConfig = document.getElementById('menu_config');
	if (!menuConfig)
	{
		return;
	}

	if (menuConfig.classList.contains('menu_config'))
	{
		menuConfig.classList.replace('menu_config', 'menu_config_visible');
	}
	else
	{
		menuConfig.classList.replace('menu_config_visible', 'menu_config');
	}
}

function activateFullScreen()
{
	var element = document.documentElement;
	if (element.requestFullscreen)
	{
		element.requestFullscreen();
	}
}

function closeFullScreen()
{
	if (document.exitFullscreen)
	{
		document.exitFullscreen();
	}
}

function fullScreen()
{
	if (document.fullscreen)
	{
		closeFullScreen();
	}
	else
	{
		activateFullScreen();
	}
}

function onChangeLocoFunctionType(nr)
{
	var locoFunctionType = document.getElementById('s_f' + nr + '_type');
	if (!locoFunctionType)
	{
		return false;
	}
	var locoFunctionIcon = document.getElementById('s_f' + nr + '_icon_container');
	if (!locoFunctionIcon)
	{
		return false;
	}
	var locoFunctionTimer = document.getElementById('d_f' + nr + '_timer');
	if (!locoFunctionTimer)
	{
		return false;
	}
	var type = locoFunctionType.value;
	if (type == 0) // LocoFunctionTypeNone
	{
		locoFunctionIcon.classList.add('hidden');
	}
	else
	{
		locoFunctionIcon.classList.remove('hidden');
	}
	if (type == 4) // LocoFunctionTypeTimer
	{
		locoFunctionTimer.classList.remove('hidden');
	}
	else
	{
		locoFunctionTimer.classList.add('hidden');
	}
	return false;
}

function onClickAddresses(signal)
{
	var addressElement = document.getElementById('address');
	if (!addressElement)
	{
		return false;
	}
	var address = addressElement.value;

	var typeElement = document.getElementById('s_signaltype');
	if (!typeElement)
	{
		return false;
	}
	var type = typeElement.value;

	var url = '?cmd=signaladdresses&address=' + address + '&type=' + type + '&signal=' + signal;

	requestUpdateItem('addresses', url);
}

function onClickProgramRead(cv)
{
	var controlElement = document.getElementById('s_controlraw');
	if (!controlElement)
	{
		return false;
	}
	var control = controlElement.value;

	var modeElement = document.getElementById('s_moderaw');
	if (!modeElement)
	{
		return false;
	}
	var mode = modeElement.value;

	var addressElement = document.getElementById('addressraw');
	if (!addressElement)
	{
		return false;
	}
	var address = addressElement.value;

	var indexElement = document.getElementById('indexraw');
	if (!indexElement)
	{
		return false;
	}
	var index = indexElement.value;

	var cvElement = document.getElementById('cvraw');
	if (!cvElement)
	{
		return false;
	}
	var cv = parseInt(cvElement.value) + (parseInt(index) * 1024);

	var valueElement = document.getElementById('valueraw');
	if (!valueElement)
	{
		return false;
	}
	valueElement.value = '-';

	var url = '?cmd=programread&control=' + control + '&mode=' + mode + '&address=' + address + '&cv=' + cv;
	fireRequestAndForget(url);
	return false;
}

function onClickProgramWrite()
{
	var controlElement = document.getElementById('s_controlraw');
	if (!controlElement)
	{
		return false;
	}
	var control = controlElement.value;

	var modeElement = document.getElementById('s_moderaw');
	if (!modeElement)
	{
		return false;
	}
	var mode = modeElement.value;

	var addressElement = document.getElementById('addressraw');
	if (!addressElement)
	{
		return false;
	}
	var address = addressElement.value;

	var indexElement = document.getElementById('indexraw');
	if (!indexElement)
	{
		return false;
	}
	var index = indexElement.value;

	var cvElement = document.getElementById('cvraw');
	if (!cvElement)
	{
		return false;
	}
	var cv = parseInt(cvElement.value) + (parseInt(index) * 1024);

	var valueElement = document.getElementById('valueraw');
	if (!valueElement)
	{
		return false;
	}
	var value = valueElement.value;
	var url = '?cmd=programwrite&control=' + control + '&mode=' + mode + '&address=' + address + '&cv=' + cv + '&value=' + value;
	fireRequestAndForget(url);
	return false;
}

function loadProgramModeSelector()
{
	var selectControl = document.getElementById('s_controlraw');
	if (!selectControl)
	{
		return;
	}
	var control = selectControl.value;

	var modeElement = document.getElementById('s_moderaw');
	if (!modeElement)
	{
		return false;
	}
	var mode = modeElement.value;

	var elementName = 'program_mode_selector';
	var url = '/?cmd=programmodeselector';
	url += '&control=' + control;
	url += '&mode=' + mode;
	requestUpdateItem(elementName, url);
}

function onChangeProgramModeSelector()
{
	var selectControl = document.getElementById('s_controlraw');
	if (!selectControl)
	{
		return;
	}
	var control = selectControl.value;

	var modeElement = document.getElementById('s_moderaw');
	if (!modeElement)
	{
		return false;
	}
	var mode = modeElement.value;

	var elementName = 'cv_fields';
	var url = '/?cmd=getcvfields';
	url += '&control=' + control;
	url += '&mode=' + mode;
	requestUpdateItem(elementName, url);
}

function updateName()
{
	var nameField = document.getElementById('name');
	if (!nameField)
	{
		return false;
	}
	var title = document.getElementById('popup_title');
	if (!title)
	{
		return false;
	}
	if (nameField.value.length > 0)
	{
		title.innerHTML = nameField.value;
	}
	else
	{
		title.innerHTML = 'NN';
	}
	return true;
}

function getArgumentsOfHardwareType()
{
	var hardwareType = document.getElementById('s_hardwaretype');
	if (!hardwareType)
	{
		return false;
	}
	var url = '?cmd=controlarguments&hardwaretype=' + hardwareType.value;
	requestUpdateItem('controlarguments', url);
	return false;
}

function updateFeedbacksOfTrack()
{
	var track = document.getElementById('s_totrack');
	if (!track)
	{
		return false;
	}
	var url = '?cmd=feedbacksoftrack&track=' + track.value;
	requestUpdateItem('feedbacks', url);
	return false;
}

function addSlave(prefix)
{
	var counter = document.getElementById(prefix + 'counter');
	if (!counter)
	{
		return false;
	}
	var div = document.getElementById(prefix + 's');
	if (!div)
	{
		return false;
	}

	counter.value++;
	var url = '/?cmd=slaveadd&priority=' + counter.value + '&prefix=' + prefix;
	requestAddItem(prefix + '_new_' + counter.value, url);
	return false;
}

function addRelation(type)
{
	var relationCounter = document.getElementById('relationcounter' + type);
	if (!relationCounter)
	{
		return false;
	}
	var relationDiv = document.getElementById('relation' + type);
	if (!relationDiv)
	{
		return false;
	}

	var url = '/?cmd=relationadd&priority=' + relationCounter.value + '&type=' + type;
	requestAddItem('new_' + type + '_priority_' + relationCounter.value, url);
	relationCounter.value++;
	return false;
}

function addFeedback()
{
	var feedbackCounter = document.getElementById('feedbackcounter');
	if (!feedbackCounter)
	{
		return false;
	}

	var identifier = '';
	var track = document.getElementById('track');
	if (track)
	{
		identifier = '&track=' + track.value;
	}
	else
	{
		var signal = document.getElementById('signal');
		if (signal)
		{
			identifier = '&signal=' + signal.value;
		}
	}

	if (identifier.length == 0)
	{
		return false;
	}

	feedbackCounter.value++;
	var url = '/?cmd=feedbackadd&counter=' + feedbackCounter.value + identifier;
	requestUpdateItem('div_feedback_' + feedbackCounter.value, url);
	return false;
}

function execFunctionByName(name)
{
	if (typeof window[name] !== 'function')
	{
		return;
	}

	window[name]();
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

	if (isNaN(input.value) || (input.value < min))
	{
		input.value = min;
	}
	else if (input.value > max)
	{
		input.value = max;
	}

	execFunctionByName('update_' + name);
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

	execFunctionByName('update_' + name);
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

	execFunctionByName('update_' + name);
}

function update_valueraw()
{
	var element = document.getElementById('valueraw');
	if (!element)
	{
		return;
	}
	for (var bit = 0; bit < 8; ++bit)
	{
		updateCvBit(bit, (element.value >> bit) & 0x01);
	}
}

function updateCvBit(bit, value)
{
	var name = 'valueraw' + bit;
	var element = document.getElementById(name);
	if (!element)
	{
		return;
	}
	element.checked = value ? true : false;
}

function updateCvValue()
{
	var value = 0;
	for (var bit = 0; bit < 8; ++bit)
	{
		var element = document.getElementById('valueraw' + bit);
		if (!element)
		{
			return;
		}
		value |= (element.checked ? (1 << bit) : 0);
	}
	var element = document.getElementById('valueraw');
	if (!element)
	{
		return;
	}
	element.value = value;
}

function setToggleButton(elementName, on)
{
	var elements = document.getElementsByName(elementName);
	for (var i = 0; i < elements.length; ++i)
	{
		if (on == 'true')
		{
			elements[i].classList.remove('button_off');
			elements[i].classList.add('button_on');
		}
		else
		{
			elements[i].classList.remove('button_on');
			elements[i].classList.add('button_off');
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
	var identifier = 'a_' + accessoryID;
	if (modifierKeyPressed(event))
	{
		rotateObject(identifier);
		return;
	}
	var element = document.getElementById(identifier);
	var url = '/?cmd=accessorystate';
	url += '&state=' + (element.classList.contains('accessory_off') ? 'on' : 'off');
	url += '&accessory=' + accessoryID;
	fireRequestAndForget(url);
	return false;
}

function onPointerDownAccessory(accessoryID)
{
	var identifier = 'a_' + accessoryID;
	if (modifierKeyPressed(event))
	{
		return;
	}
	if (event.button != 0)
	{
		return;
	}
	var element = document.getElementById(identifier);
	var url = '/?cmd=accessorystate';
	url += '&state=on';
	url += '&accessory=' + accessoryID;
	fireRequestAndForget(url);
	return false;
}

function onPointerUpAccessory(accessoryID)
{
	var identifier = 'a_' + accessoryID;
	if (modifierKeyPressed(event))
	{
		rotateObject(identifier);
		return;
	}
	if (event.button != 0)
	{
		return;
	}
	var element = document.getElementById(identifier);
	var url = '/?cmd=accessorystate';
	url += '&state=off';
	url += '&accessory=' + accessoryID;
	fireRequestAndForget(url);
	return false;
}

function onClickRoute(routeID)
{
	var element = document.getElementById('r_' + routeID);
	var url = '/?cmd=routeexecute';
	url += '&route=' + routeID;
	fireRequestAndForget(url);
	return false;
}

function onClickSwitch(event, switchID)
{
	var identifier = 'sw_' + switchID;
	if (modifierKeyPressed(event))
	{
		rotateObject(identifier);
		return;
	}
	var element = document.getElementById(identifier);
	var url = '/?cmd=switchstate&state='
	if (element.classList.contains('switch_straight'))
	{
		url += 'turnout'
	}
	else
	{
		url += 'straight'
	}
	url += '&switch=' + switchID;
	fireRequestAndForget(url);
	return false;
}

function onClickFeedback(feedbackID)
{
	var identifier = 'f_' + feedbackID;
	if (modifierKeyPressed(event))
	{
		rotateObject(identifier);
		return;
	}
	var element = document.getElementById(identifier);
	var url = '/?cmd=feedbackstate';
	url += '&state=' + (element.classList.contains('feedback_free') ? 'occupied' : 'free');
	url += '&feedback=' + feedbackID;
	fireRequestAndForget(url);
	return false;
}

function onClickSignal(signalID)
{
	var identifier = 'si_' + signalID;
	if (modifierKeyPressed(event))
	{
		rotateObject(identifier);
		return;
	}
	var element = document.getElementById(identifier);
	var url = '/?cmd=signalstate';
	url += '&state=' + (element.classList.contains('signal_clear') ? 'stop' : 'clear');
	url += '&signal=' + signalID;
	fireRequestAndForget(url);
	return false;
}

function showMenu(elementName) {
	var element = document.getElementById(elementName);
	if (!element)
	{
		return;
	}

	var mouseX = event.clientX;
	var mouseY = event.clientY;
	var windowX = window.innerWidth;
	var windowY = window.innerHeight;

	if (windowX > (mouseX * 2)) {
		element.style.left = mouseX + "px";
		element.style.right = "auto";
	}
	else {
		element.style.left = "auto";
		element.style.right = (windowX - mouseX) + "px";
	}

	if (windowY > (mouseY * 2)) {
		element.style.top = mouseY + "px";
		element.style.bottom = "auto";
	}
	else {
		element.style.top = "auto";
		element.style.bottom = (windowY - mouseY) + "px";
	}
	element.style.display = 'block';
}

function showOnClickMenu(event, ID)
{
	if (modifierKeyPressed(event))
	{
		return true;
	}
	event.stopPropagation();
	hideAllContextMenus();
	showMenu(ID + '_onclick');
	return false;
}

function showContextMenu(event, ID)
{
	if (modifierKeyPressed(event))
	{
		return true;
	}
	event.stopPropagation();
	hideAllContextMenus();
	showMenu(ID + '_context');
	return false;
}

function onChangeCheckboxShowHide(checkboxId, divId, tabId)
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

	var tab = document.getElementById(tabId);
	if (!tab)
	{
		return;
	}
	if (checkbox.checked)
	{
		tab.classList.remove("hidden");
	}
	else
	{
		tab.classList.add("hidden");
	}
}

function onChangeTrackTypeMainTrack()
{
	let feedbacks = document.getElementById('tab_button_feedbacks');
	if (!feedbacks)
	{
		return;
	}
	let signals = document.getElementById('tab_button_signals');
	if (!signals)
	{
		return;
	}
	let automode = document.getElementById('tab_button_automode');
	if (!automode)
	{
		return;
	}
	let trackType = document.getElementById('s_tracktype');
	if (!trackType)
	{
		return;
	}
	let main = document.getElementById('s_main');
	if (!main)
	{
		return;
	}
	let name = document.getElementById('i_name');
	if (!name)
	{
		return;
	}
	let showname = document.getElementById('i_showname');
	if (!showname)
	{
		return;
	}
	let shownamecb = document.getElementById('showname');
	if (!shownamecb)
	{
		return;
	}
	let displayname = document.getElementById('i_displayname');
	if (!displayname)
	{
		return;
	}
	let length = document.getElementById('i_length');
	if (!length)
	{
		return;
	}

	if (main.value != 0)
	{
		feedbacks.classList.add('hidden');
		signals.classList.add('hidden');
		automode.classList.add('hidden');
		name.classList.add('hidden');
	}
	else
	{
		feedbacks.classList.remove('hidden');
		signals.classList.remove('hidden');
		automode.classList.remove('hidden');
		name.classList.remove('hidden');
	}

	let trackTypeValue = trackType.value;
	if ((trackTypeValue != 0) && (main.value != 0))
	{
		showname.classList.add('hidden');
	}
	else
	{
		showname.classList.remove('hidden');
	}

	if ((main.value != 0) || (trackTypeValue != 0) || (shownamecb.checked == false))
	{
		displayname.classList.add('hidden');
	}
	else
	{
		displayname.classList.remove('hidden');
	}

	if (trackTypeValue == 1 || trackTypeValue == 5 || trackTypeValue == 7 || trackTypeValue == 8 || trackTypeValue == 9)
	{
		length.classList.add('hidden');
	}
	else
	{
		length.classList.remove('hidden');
	}
}

function updateLayoutItem(elementName, data)
{
	var parentElement = document.getElementById('layout');
	if (parentElement)
	{
		deleteElement(elementName);
		var elementOnClickName = elementName + '_onclick';
		deleteElement(elementOnClickName);
		var elementContextName = elementName + '_context';
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

function requestUpdateItem(elementName, url, followUpFunction)
{
	updateItem(elementName, "Loading...");
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() {
		if (xmlHttp.readyState != 4 || xmlHttp.status != 200)
		{
			return;
		}
		updateItem(elementName, xmlHttp.responseText);
		if (followUpFunction === undefined)
		{
			return;
		}
		followUpFunction();
	}
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
}

function updateTitle()
{
	var locoName = document.getElementById("loconame");
	if (!locoName)
	{
		return;
	}
	updateItem("title", locoName.innerHTML + " - RailControl");
}

function updateText(textID)
{
	elementName = 'tx_' + textID;
	var url = '/?cmd=textget';
	url += '&text=' + textID;
	requestUpdateLayoutItem(elementName, url);
}

function updateTrack(trackID)
{
	elementName = 't_' + trackID;
	var url = '/?cmd=trackget';
	url += '&track=' + trackID;
	requestUpdateLayoutItem(elementName, url);
}

function updateTrackState(argumentMap)
{
	var trackID = argumentMap.get('track');
	var signalID = argumentMap.get('signal');
	if (trackID > 0)
	{
		elementName = 't_' + trackID;
	}
	else if (signalID > 0)
	{
		elementName = 'si_' + signalID;
	}
	var element = document.getElementById(elementName);
	if (element)
	{
		var reserved = false;
		var occupied = false;
		var blocked = false;
		var error = false;
		var orientation = true;

		if (argumentMap.has('occupied'))
		{
			occupied = argumentMap.get('occupied') == 'true';
		}

		if (argumentMap.has('reserved'))
		{
			reserved = argumentMap.get('reserved') == 'true';
		}

		if (argumentMap.has('blocked'))
		{
			blocked = argumentMap.get('blocked') == 'true';
		}

		element.classList.remove('track_free');
		element.classList.remove('track_reserved');
		element.classList.remove('track_reserved_occupied');
		element.classList.remove('track_occupied');
		element.classList.remove('track_error');
		element.classList.remove('track_blocked');

		if (reserved && occupied)
		{
			element.classList.add('track_reserved_occupied');
		}
		else if (reserved)
		{
			element.classList.add('track_reserved');
		}
		else if (occupied)
		{
			element.classList.add('track_occupied');
		}
		else if (blocked)
		{
			element.classList.add('track_blocked');
		}
		else
		{
			element.classList.add('track_free');
		}

		if (error)
		{
			element.classList.add('track_error');
		}

		if (argumentMap.has('orientation'))
		{
			orientation = argumentMap.get('orientation') == 'true';
		}

		if (reserved)
		{
			element.classList.remove('loco_unknown');
			element.classList.add('loco_known');
		}
		else
		{
			element.classList.remove('loco_known');
			element.classList.add('loco_unknown');
		}
	}

	var onClickElement = document.getElementById(elementName + '_onclick');
	if (onClickElement)
	{
		if (reserved)
		{
			onClickElement.classList.remove('loco_unknown');
			onClickElement.classList.add('loco_known');
		}
		else
		{
			onClickElement.classList.remove('loco_known');
			onClickElement.classList.add('loco_unknown');
		}

		if (blocked)
		{
			onClickElement.classList.remove('track_unblocked');
			onClickElement.classList.add('track_blocked');
		}
		else
		{
			onClickElement.classList.remove('track_blocked');
			onClickElement.classList.add('track_unblocked');
		}

		if (orientation)
		{
			onClickElement.classList.remove('orientation_left');
			onClickElement.classList.add('orientation_right');
		}
		else
		{
			onClickElement.classList.remove('orientation_right');
			onClickElement.classList.add('orientation_left');
		}
	}

	var locoElement = document.getElementById(elementName + '_text_loconame');
	if (locoElement)
	{
		var orientationArrow = orientation ? '&rarr; ' : '&larr; ';
		var locoName = argumentMap.has('loconame') ? argumentMap.get('loconame') : '';
		locoElement.innerHTML = orientationArrow + locoName;
	}
}

function updateSwitchState(argumentMap)
{
	if (!argumentMap.has('switch') || !argumentMap.has('state'))
	{
		return;
	}
	var elementName = 'sw_' + argumentMap.get('switch');
	var element = document.getElementById(elementName);
	if (!element)
	{
		return;
	}
	var state = argumentMap.get('state');
	updateSwitchStateDiv(element, state);
	elementName += '_onclick';
	element = document.getElementById(elementName);
	if (!element)
	{
		return;
	}
	updateSwitchStateDiv(element, state);
}

function updateSwitchStateDiv(element, state)
{
	element.classList.remove('switch_turnout');
	element.classList.remove('switch_straight');
	element.classList.remove('switch_third');
	switch (state)
	{
		case 'turnout':
			element.classList.add('switch_turnout');
			break;

		case 'straight':
			element.classList.add('switch_straight');
			break;

		case 'third':
			element.classList.add('switch_third');
			break;
	}
}

function updateSignalState(argumentMap)
{
	if (!argumentMap.has('signal') || !argumentMap.has('state'))
	{
		return;
	}
	var elementName = 'si_' + argumentMap.get('signal');
	var element = document.getElementById(elementName);
	if (!element)
	{
		return;
	}
	var state = argumentMap.get('state');
	updateSignalStateDiv(element, state);
	elementName += '_onclick';
	element = document.getElementById(elementName);
	if (!element)
	{
		return;
	}
	updateSignalStateDiv(element, state);
}

function updateSignalStateDiv(element, state)
{
	element.classList.remove('signal_stop');
	element.classList.remove('signal_clear');
	element.classList.remove('signal_aspect2');
	element.classList.remove('signal_aspect3');
	element.classList.remove('signal_aspect4');
	element.classList.remove('signal_aspect5');
	element.classList.remove('signal_aspect6');
	element.classList.remove('signal_aspect7');
	element.classList.remove('signal_aspect8');
	element.classList.remove('signal_aspect9');
	element.classList.remove('signal_aspect10');
	element.classList.remove('signal_dark');
	element.classList.remove('signal_stopexpected');
	element.classList.remove('signal_clearexpected');
	element.classList.remove('signal_aspect2expected');
	element.classList.remove('signal_aspect3expected');
	element.classList.remove('signal_aspect4expected');
	element.classList.remove('signal_aspect5expected');
	element.classList.remove('signal_aspect6expected');
	element.classList.remove('signal_aspect7expected');
	element.classList.remove('signal_aspect8expected');
	element.classList.remove('signal_aspect9expected');
	element.classList.remove('signal_aspect10expected');
	switch (state)
	{
		case 'stop':
			element.classList.add('signal_stop');
			break;

		case 'clear':
			element.classList.add('signal_clear');
			break;

		case 'aspect2':
			element.classList.add('signal_aspect2');
			break;

		case 'aspect3':
			element.classList.add('signal_aspect3');
			break;

		case 'aspect4':
			element.classList.add('signal_aspect4');
			break;

		case 'aspect5':
			element.classList.add('signal_aspect5');
			break;

		case 'aspect6':
			element.classList.add('signal_aspect6');
			break;

		case 'aspect7':
			element.classList.add('signal_aspect7');
			break;

		case 'aspect8':
			element.classList.add('signal_aspect8');
			break;

		case 'aspect9':
			element.classList.add('signal_aspect9');
			break;

		case 'aspect10':
			element.classList.add('signal_aspect10');
			break;

		case 'dark':
			element.classList.add('signal_dark');
			break;

		case 'stopexpected':
			element.classList.add('signal_stopexpected');
			break;

		case 'clearexpected':
			element.classList.add('signal_clearexpected');
			break;

		case 'aspect2expected':
			element.classList.add('signal_aspect2expected');
			break;

		case 'aspect3expected':
			element.classList.add('signal_aspect3expected');
			break;

		case 'aspect4expected':
			element.classList.add('signal_aspect4expected');
			break;

		case 'aspect5expected':
			element.classList.add('signal_aspect5expected');
			break;

		case 'aspect6expected':
			element.classList.add('signal_aspect6expected');
			break;

		case 'aspect7expected':
			element.classList.add('signal_aspect7expected');
			break;

		case 'aspect8expected':
			element.classList.add('signal_aspect8expected');
			break;

		case 'aspect9expected':
			element.classList.add('signal_aspect9expected');
			break;

		case 'aspect10expected':
			element.classList.add('signal_aspect10expected');
			break;
	}
}

function updateSignal(signalID)
{
	elementName = 'si_' + signalID;
	var url = '/?cmd=signalget';
	url += '&signal=' + signalID;
	requestUpdateLayoutItem(elementName, url);
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
			var statusData = status.innerHTML + parts[1] + '<br>';
			status.innerHTML = statusData.substring(statusData.length - 2500);
			status.scrollTop = status.scrollHeight - status.clientHeight;
		}
		argumentMap.set(parts[0], parts[1]);
	});

	var elementName = "";
	var command = argumentMap.get('command');
	if (command == 'warning')
	{
		var state = argumentMap.get('status');
		addInfoBox('w', state);
	}
	else if (command == 'booster')
	{
		elementName = 'skip_booster';
		var on = argumentMap.get('on');
		setToggleButton(elementName, on);
	}
	else if (command == 'locospeed')
	{
		var speed = argumentMap.get('speed');
		elementName = 'locospeed_' + argumentMap.get('loco');
		var elements = document.getElementsByName(elementName);
		for (var i = 0; i < elements.length; ++i)
		{
			elements[i].value = speed;
		}
	}
	else if (command == 'locofunction')
	{
		elementName = 'skip_locofunction_' + argumentMap.get('loco') + '_' + argumentMap.get('function');
		var on = argumentMap.get('on');
		setToggleButton(elementName, on);
	}
	else if (command == 'locoorientation')
	{
		elementName = 'skip_locoorientation_' + argumentMap.get('loco');
		var on = argumentMap.get('orientation');
		setToggleButton(elementName, on);
	}
	else if (command == 'accessory')
	{
		elementName = 'a_' + argumentMap.get('accessory');
		var element = document.getElementById(elementName);
		if (element && argumentMap.has('state'))
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
		updateSwitchState(argumentMap);
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
		deleteElement(elementName + '_onclick');
		deleteElement(elementName + '_context');
	}
	else if (command == 'signal')
	{
		updateSignalState(argumentMap);
	}
	else if (command == 'signalsettings')
	{
		updateSignal(argumentMap.get('signal'));
	}
	else if (command == 'signaldelete')
	{
		elementName = 'si_' + argumentMap.get('signal');
		deleteElement(elementName);
		deleteElement(elementName + '_onclick');
		deleteElement(elementName + '_context');
	}
	else if (command == 'routesettings')
	{
		var routeID = argumentMap.get('route');
		elementName = 'r_' + routeID;
		var url = '/?cmd=routeget';
		url += '&route=' + routeID;
		requestUpdateLayoutItem(elementName, url);
	}
	else if (command == 'routedelete')
	{
		elementName = 'r_' + argumentMap.get('route');
		deleteElement(elementName);
		deleteElement(elementName + '_context');
	}
	else if (command == 'textsettings')
	{
		updateText(argumentMap.get('text'));
	}
	else if (command == 'textdelete')
	{
		elementName = 'tx_' + argumentMap.get('text');
		deleteElement(elementName);
		deleteElement(elementName + '_context');
	}
	else if (command == 'trackstate')
	{
		updateTrackState(argumentMap);
	}
	else if (command == 'tracksettings')
	{
		updateTrack(argumentMap.get('track'));
	}
	else if (command == 'trackdelete')
	{
		elementName = 't_' + argumentMap.get('track');
		deleteElement(elementName);
		deleteElement(elementName + '_onclick');
		deleteElement(elementName + '_context');
	}
	else if (command == 'feedback')
	{
		elementName = 'f_' + argumentMap.get('feedback');
		var element = document.getElementById(elementName);
		if (element && argumentMap.has('state'))
		{
			var state = argumentMap.get('state');
			if (state == 'on')
			{
				element.classList.remove('feedback_free');
				element.classList.add('feedback_occupied');
			}
			else
			{
				element.classList.remove('feedback_occupied');
				element.classList.add('feedback_free');
			}
		}
	}
	else if (command == 'feedbacksettings')
	{
		var feedbackID = argumentMap.get('feedback');
		var layerID = document.getElementById('s_layer').value;
		elementName = 'f_' + feedbackID;
		var url = '/?cmd=feedbackget';
		url += '&feedback=' + feedbackID;
		url += '&layer=' + layerID;
		requestUpdateLayoutItem(elementName, url);
	}
	else if (command == 'feedbackdelete')
	{
		elementName = 'f_' + argumentMap.get('feedback');
		deleteElement(elementName);
		deleteElement(elementName + '_context');
	}
	else if ((command == 'locosettings')
		|| (command == 'locodelete')
		|| (command == 'multipleunitsettings')
		|| (command == 'multipleunitdelete'))
	{
		loadLocoSelectors();
	}
	else if ((command == 'layersettings')
		|| (command == 'layerdelete'))
	{
		loadLayerSelector();
	}
	else if (command == 'dcccvvalue')
	{
		var cv = argumentMap.get('cv');
		var cvElementName = 'cvraw';
		var cvElement = document.getElementById(cvElementName);
		if (cvElement)
		{
			cvElement.value = cv;
		}
		var value = argumentMap.get('value');
		var valueElementName = 'valueraw';
		var valueElement = document.getElementById(valueElementName);
		if (valueElement)
		{
			valueElement.value = value;
			update_valueraw();
		}
	}
}

function dataUpdateError()
{
	var body = document.getElementById("body");
	body.innerHTML = '<h1>RailControl shut down</h2><p>Please start RailControl server again.</p><hr><h1>RailControl wurde heruntergefahren</h1><p>Bitte starte den RailControl server wieder.</p><hr><h1>RailControl se ha apagado</h2><p>Por favor reinicie el servidor RailControl otra vez.</p>';
	checkAvailability();
}

function loadPopup(url)
{
	url += '&posx=' + window.layoutPosX;
	url += '&posy=' + window.layoutPosY;
	url += '&posz=' + document.getElementById('s_layer').value;
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() {
		if (xmlHttp.readyState !== 4 || xmlHttp.status !== 200)
		{
			return;
		}
		var popup = document.getElementById('popup');
		popup.innerHTML = xmlHttp.responseText;
		popup.style.display = 'block';
	}
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
}

function loadLocoSelector(selector)
{
	var locoValue = document.getElementById('s_loco_' + selector).value;
	var url = '/?cmd=locoselector&selector=' + selector + '&loco=' + locoValue;
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onload = function()
	{
		if (xmlHttp.status !== 200)
		{
			return;
		}
		var element = document.getElementById('loco_selector_' + selector);
		element.innerHTML = xmlHttp.responseText;
		loadLoco(selector);
	}
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
}

function loadLocoSelectors()
{
	for (var selector = 1; selector <= MaxNumberOfLocoControls; ++selector)
	{
		loadLocoSelector(selector);
	}
}

function loadLayerSelector()
{
	var layerValue = document.getElementById('s_layer').value;
	var url = '/?cmd=layerselector&layer=' + layerValue;
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onload = function()
	{
		if (xmlHttp.status !== 200)
		{
			return;
		}
		var element = document.getElementById('layer_selector');
		element.innerHTML = xmlHttp.responseText;
		loadLayout();
	}
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
}

function loadProtocol(type, ID)
{
	var selectControl = document.getElementById('s_control');
	if (!selectControl)
	{
		return;
	}
	var controlID = selectControl.value;
	var elementName = 'select_protocol';
	var selectProtocol = document.getElementById(elementName);
	if (!selectProtocol)
	{
		return;
	}
	var url = '/?cmd=protocol';
	url += '&control=' + controlID;
	url += '&' + type + '=' + ID;
	requestUpdateItem(elementName, url);
}

function loadAccessoryAddress()
{
	var selectAccessoryType = document.getElementById('s_connectiontype');
	if (!selectAccessoryType)
	{
		return;
	}
	var type = selectAccessoryType.value;
	var elementName = 'select_address';
	var selectAddress = document.getElementById(elementName);
	if (!selectAddress)
	{
		return;
	}
	var intAddress = document.getElementById('address');
	if (!intAddress)
	{
		return;
	}
	var address = intAddress.value;
	var selectPort = document.getElementById('s_port');
	if (!selectPort)
	{
		selectPort = document.getElementById('port');
		if (!selectPort)
		{
			return;
		}
	}
	var port = selectPort.value;
	var url = '/?cmd=accessoryaddress';
	url += '&type=' + type;
	url += '&address=' + address;
	url += '&port=' + port;
	requestUpdateItem(elementName, url);
}

function loadRelationObject(atlock, priority, objectType = 0, id = 0, state = 0)
{
	var elementName = 'relation_' + atlock + '_' + priority;
	var object = document.getElementById(elementName);
	if (!object)
	{
		return;
	}
	if (objectType == 0)
	{
		var typeSelector = document.getElementById('s_' + elementName + '_type');
		if (!typeSelector)
		{
			return;
		}
		objectType = typeSelector.value;
	}
	var url = '/?cmd=relationobject';
	url += '&objecttype=' + objectType;
	url += '&priority=' + priority;
	url += '&atlock=' + atlock;
	if (objectType != 0)
	{
		url += '&id=' + id;
		url += '&state=' + state;
	}
	requestUpdateItem(elementName + "_object", url);
}

function loadRelationObjectStates(type, name)
{
	var elementName = name + '_state';
	var object = document.getElementById(elementName);
	if (!object)
	{
		return;
	}
	var object = document.getElementById('s_' + name + '_id');
	if (!object)
	{
		return;
	}
	var objectId = object.value;
	var url = '/?cmd=' + type + 'states';
	url += '&' + type + '=' + objectId;
	url += '&name=' + name;
	requestUpdateItem(elementName, url);
}

function loadLayoutContext(event)
{
	if (modifierKeyPressed(event))
	{
		return true;
	}
	event.preventDefault();
	hideAllContextMenus();
	showMenu('layout_context');
	var x = Math.round(event.target.scrollLeft + event.offsetX, 0);
	var y = Math.round(event.target.scrollTop + event.offsetY, 0);
	window.layoutPosX = Math.floor(x / 36);
	window.layoutPosY = Math.floor(y / 36);
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

function hideElement(name)
{
	var element = document.getElementById(name);
	if (!element)
	{
		return;
	}
	element.style.display = 'none';
}

function loadLoco(selector)
{
	var loco = document.getElementById('s_loco_' + selector);
	if (!loco)
	{
		return;
	}
	requestUpdateItem('loco_' + selector, '/?cmd=loco&loco=' + loco.value, updateTitle);
}

function loadLayout()
{
	var layer = document.getElementById('s_layer');
	if (layer)
	{
		requestUpdateItem('layout', '/?cmd=layout&layer=' + layer.value);
		var context = document.getElementById('layout_context');
		if (layer.value > 0)
		{
			context.classList.remove('feedback_layer');
		}
		else
		{
			context.classList.add('feedback_layer');
		}
	}
}

function sendTimestamp()
{
	var url = '/?cmd=timestamp&timestamp=';
	var timestamp = Math.round(Date.now() / 1000);
	url += timestamp;
	fireRequestAndForget(url);
}

function startUp()
{
	let body = document.getElementById('body');
	if (body)
	{
		body.addEventListener('click', function(event) {
			if (event.button == 2)
			{
				return false;
			}
			hideAllContextMenus();
			return true;
		}, true);
		body.addEventListener('keydown', function(event) {
			let popup = document.getElementById('popup');
			if (popup)
			{
				let popupstyle = window.getComputedStyle(popup, null).display;
				if (popupstyle != 'none')
				{
					return true;
				}
			}
			if (event.key === ' ')
			{
				let booster_button = document.getElementById('skip_booster');
				if (!booster_button)
				{
					return true;
				}
				let on = !booster_button.classList.contains('button_on');
				let url = '/?cmd=booster&on=' + (on ? '1' : '0') + '';
				fireRequestAndForget(url);
				return false; 
			}
			else if ((event.keyCode == 38) || (event.key === '8')) // arrow up
			{
				locoSpeedChange(+8);
			}
			else if ((event.keyCode == 40) || (event.key === '2')) // arrow down
			{
				locoSpeedChange(-8);
			}
			else if ((event.keyCode == 33) || (event.key === '9')) // arrow page up
			{
				locoSpeedChange(+80);
			}
			else if ((event.keyCode == 34) || (event.key === '3')) // arrow page down
			{
				locoSpeedChange(-80);
			}
			else if ((event.keyCode == 45) || (event.key === '0')) // 0
			{
				locoSpeedChange(-1024);
			}
			else if ((event.keyCode == 37) || (event.key === '4')) // arrow left
			{
				locoOrientationChange(0);
			}
			else if ((event.keyCode == 39) || (event.key === '6')) // arrow right
			{
				locoOrientationChange(1);
			}
		}, true);
	}
	updateLocoControls();
	loadLoco(1);
	loadLoco(2);
	loadLoco(3);
	loadLoco(4);
	loadLoco(5);
	loadLayout();
	sendTimestamp();
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

function hidePopup()
{
	var popup = document.getElementById('popup');
	popup.style.display = 'none';
}

var infoID = 0;

function addInfoBox(type, info)
{
	var id = 'res_' + ++infoID;

	var className;

	switch (type)
	{
		case 'i': // info
			hidePopup();
			className = 'infoboxinfo';
			setTimeout(function()
			{
				deleteElement(id);
			}, 2000);
			break;

		case 'w': // warning
			hidePopup();
			className = 'infoboxwarning';
			break;

		case 'e': // error
			className = 'infoboxerror';
			break;

		default:
			className  = 'infoboxunknown';
	}

	var preparedInfo = '<div id="' + id + '" class="' + className + '" onClick="deleteElement(\'' + id + '\');">' + info + '</div>';
	var infobox = document.getElementById('infobox');
	infobox.innerHTML += preparedInfo;
}

function submitEditForm()
{
	var url = '/?';
	var form = document.getElementById('editform');
	var i = 0;
	while (true)
	{
		var formElement = form[i];
		if (formElement == undefined)
		{
			break;
		}
		if (formElement.name.substr(0, 5) == "skip_")
		{
			++i;
			continue;
		}
		if (i > 0)
		{
			url += '&';
		}
		url += formElement.name;
		url += '=';
		if (formElement.type == 'checkbox')
		{
			url += formElement.checked;
		}
		else
		{
			url += encodeURIComponent(formElement.value);
		}
		++i;
	}
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onreadystatechange = function() {
		if (xmlHttp.readyState !== 4 || xmlHttp.status !== 200)
		{
			return;
		}
		var response = xmlHttp.responseText;
		addInfoBox(response[21], response.substring(22, response.length - 7));
	}
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
	return false;
}

var updateSliderAllowed = true;

function locoSpeedSliderOnInput(locoId)
{
	if (!updateSliderAllowed)
	{
		return false;
	}
	setTimeout(function()
	{
		updateSliderAllowed = true;
	}, 200);
	locoSpeedSliderOnChange(locoId);
	updateSliderAllowed = false;
	return false;
}

function locoSpeedSliderOnChange(locoId)
{
	var slider = document.getElementById('locospeed_' + locoId);
	if (!slider)
	{
		return false;
	}
	var url = '/?cmd=locospeed&loco=';
	url += locoId;
	url += '&speed=';
	url += slider.value;
	fireRequestAndForget(url);
	updateSliderAllowed = true;
	return false;
}

function locoSpeedChange(delta)
{
	let loco = document.getElementById('loco');
	if (!loco)
	{
		return false;
	}
	let locoId = loco.value;
	let slider = document.getElementById('locospeed_' + locoId);
	if (!slider)
	{
		return false;
	}
	let oldValue = +slider.value;
	let newValue = 0 + oldValue;
	newValue += delta;
	if (newValue > 1023)
	{
		newValue = 1023;
	}
	if (newValue < 0)
	{
		newValue = 0;
	}
	let url = '/?cmd=locospeed&loco=';
	url += locoId;
	url += '&speed=';
	url += newValue;
	fireRequestAndForget(url);
	return false;
}

function locoOrientationChange(orientation)
{
	let loco = document.getElementById('loco');
	if (!loco)
	{
		return false;
	}
	let locoId = loco.value;
	let url = '/?cmd=locoorientation&loco=';
	url += locoId;
	url += '&on=';
	url += orientation;
	fireRequestAndForget(url);
	return false;
}

function fireRequestAndForget(url)
{
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open('GET', url, true);
	xmlHttp.send(null);
}

function checkAvailability()
{
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.onerror = function()
	{
		setTimeout(function() { checkAvailability(); }, 3000);
	}
	xmlHttp.onload = function()
	{
		location.reload();
	}
	xmlHttp.open('GET', '/', true);
	xmlHttp.send(null);
}

function updateLocoControls()
{
	var layout = document.getElementById('layout');
	if (!layout)
	{
		return;
	}
	var layerSelector = document.getElementById('layer_selector');
	if (!layerSelector)
	{
		return;
	}

	maxNumberOfLocoControlsInScreen = Math.floor(document.documentElement.clientWidth / 252);
	if (numberOfLocoControls > maxNumberOfLocoControlsInScreen)
	{
		numberOfLocoControls = maxNumberOfLocoControlsInScreen;
	}

	var left = numberOfLocoControls * 252;
	left += "px";
	layout.style.left = left;
	layerSelector.style.left = left;

	for (var control = 0; control <= MaxNumberOfLocoControls; ++control)
	{
		var element = document.getElementById('loco_container_' + control);
		if (!element)
		{
			continue;
		}
		if (control > numberOfLocoControls)
		{
			element.classList.add('hidden');
		}
		else
		{
			element.classList.remove('hidden');
		}
	}

	var reduce = document.getElementById("reduce_locos");
	if (reduce)
	{
		if (numberOfLocoControls == 0)
		{
			reduce.classList.add("hidden");
		}
		else
		{
			reduce.classList.remove("hidden");
			if (numberOfLocoControls >= maxNumberOfLocoControlsInScreen || numberOfLocoControls >= MaxNumberOfLocoControls)
			{
				reduce.style.left = (numberOfLocoControls * 252 - 30) + "px";
				reduce.style.width = "30px";
			}
			else
			{
				reduce.style.left = (numberOfLocoControls * 252 - 15) + "px";
				reduce.style.width = "15px";
			}
		}
	}

	var extend = document.getElementById("extend_locos");
	if (extend)
	{
		if (numberOfLocoControls >= maxNumberOfLocoControlsInScreen || numberOfLocoControls >= MaxNumberOfLocoControls)
		{
			extend.classList.add("hidden");
		}
		else
		{
			extend.classList.remove("hidden");
			extend.style.left = (numberOfLocoControls * 252) + "px";
			if (numberOfLocoControls > 0)
			{
				extend.style.width = "15px";
			}
			else
			{
				extend.style.width = "30px";
			}
		}
	}
}

function reduceLocos()
{
	if (numberOfLocoControls == 0)
	{
		return;
	}
	--numberOfLocoControls;
	updateLocoControls();
}

function extendLocos()
{
	if (numberOfLocoControls >= maxNumberOfLocoControlsInScreen)
	{
		return;
	}
	++numberOfLocoControls;
	updateLocoControls();
}

function swapRelations(atlock, ownPriority, up)
{
	var ownElementName = 's_relation_' + atlock + '_' + ownPriority + '_';
	var ownElementType = document.getElementById(ownElementName + 'type');
	var ownElementId = document.getElementById(ownElementName + 'id');
	var ownElementState = document.getElementById(ownElementName + 'state');
	if (!ownElementType)
	{
		alert("Own element does not exist: " + ownElementName);
		return;
	}
	var ownType = ownElementType.value;
	var ownId = 0;
	if (ownElementId)
	{
		ownId = ownElementId.value;
	}
	var ownState = 0;
	if (ownElementState)
	{
		ownState = ownElementState.value;
	}

	var otherPriority = ownPriority;
	var otherElementName;
	var otherElementType = false;
	while (!otherElementType)
	{
		if (up == 'up')
		{
			--otherPriority;
			if (otherPriority == 0)
			{
				return;
			}
		}
		else
		{
			++otherPriority;
			if (otherPriority == 255)
			{
				return;
			}
		}
		otherElementName = 's_relation_' + atlock + '_' + otherPriority + '_';
		otherElementType = document.getElementById(otherElementName + 'type');
	}
	var otherElementId = document.getElementById(otherElementName + 'id');
	var otherElementState = document.getElementById(otherElementName + 'state');

	var otherType = otherElementType.value;

	var otherId = 0;
	if (otherElementId)
	{
		otherId = otherElementId.value;
	}
	var otherState = 0;
	if (otherElementState)
	{
		otherState = otherElementState.value;
	}

	loadRelationObject(atlock, ownPriority, otherType, otherId, otherState);
	loadRelationObject(atlock, otherPriority, ownType, ownId, ownState);
}

function enableNoSleep()
{
	document.removeEventListener('click', enableNoSleep, false);
	noSleep.enable();
}

window.layoutPosX = 0;
window.layoutPosY = 0;

var numberOfLocoControls = 1;
var maxNumberOfLocoControlsInScreen = Math.floor(document.documentElement.clientWidth / 252);
const MaxNumberOfLocoControls = 5;

var noSleep = new NoSleep();
document.addEventListener('click', enableNoSleep, false);

var updater = new EventSource('/?cmd=updater');
updater.addEventListener('message', dataUpdate);

updater.addEventListener('error', dataUpdateError);

window.addEventListener('resize', updateLocoControls);
