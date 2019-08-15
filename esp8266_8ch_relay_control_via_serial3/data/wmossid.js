var g_orderSsidArr = [];

// 1 dimension which shows the order of the Ssid's
// var g_orderSsidArr;

// 2 dimension ssid and password
var g_newSsidArr;

// 1 dimension of items deleted
var g_deleteSsidArr;


function addSsidField(){
	// ssidlist each ssid item needs to provided
	//
	// get the table elememnt that we going to add the ssid item too
	var ssidTabND = document.getElementById('ssidlist');
	
	while (ssidTabND.firstChild) {
		ssidTabND.removeChild(ssidTabND.firstChild);
	}
	if(g_orderSsidArr !== null){
		for(var i = 0; i < g_orderSsidArr.length; i++){
			ssidTabND.appendChild(ssidNRow(i+1, g_orderSsidArr[i]));
		}
	}
	// add row for a new/additional SSID
	ssidTabND.appendChild(addssidRow());
	ssidLimit();
	
	var ssidAddEle = document.getElementById("addssidbutt");
	
	var nussidEle = document.getElementById("nussid");
	
	if(nussidEle.value.length === 0){
		disableAdd(ssidAddEle, true);
		return false;
	}
	
	if(g_orderSsidArr.length > 0){
		var eleN = document.getElementById("chkbxssid1");
		eleN.checked = true;
		disableAdd(ssidAddEle, false);
		
		return true;
	}
	disableAdd(ssidAddEle, true);
	return false;
}

function disableAdd(ssidAddEle, state){
	ssidAddEle.disabled = state;
	
	var conndataEle = document.getElementById("conndatadiv");
		
	if(ssidAddEle.disabled){
		conndataEle.style.display = 'none';
	}else{
		conndataEle.style.display = 'block';
	}
}

function subdata(){
	// sub-data applies to the item that is selected, so look for it
	for(var i = 1; i <= 5; i++){
		var ssidStr = 'ssid' + i;
		
		var chkbxEle = document.getElementById('chkbx' + ssidStr);
		if(chkbxEle !== null){	
			if(chkbxEle.checked){
				var ssidinpEle = document.getElementById(ssidStr);
				var nuSsid = ssidinpEle.value;
		
				newaddssidSend(false, nuSsid, "N/A");
				break;
			}
		}
	}
}



function newaddssid(){
	if(g_newSsidArr === undefined){
		g_newSsidArr = new Array();
	}
	// ssid and pwd into an array
	var nuSsid = document.getElementById('nussid').value;
	var nuSsidPwd = document.getElementById('nussidpw').value;
	
	if(ssidduplicate(nuSsid)){
		return;
	}
	newaddssidSend(true, nuSsid, nuSsidPwd);
}

function subDataGetIpGrp(idPrefix, numFields, delimiter){
	var ipAddrStr = "";
	
	for(var i = 1; i < numFields + 1; i++){
		var ipField = document.getElementById("" + idPrefix + i);
		
		if(ipField ===null){
			break;
		}
		var value = ipField.value;
		
		ipAddrStr += delimiter;
		ipAddrStr += value;
	}
	return ipAddrStr.substring(1); // removing leading period
}

function mcpAddrSend(){
	var mcpadrLocal = document.getElementById("mcpadr").value;
	var	sendData = "mcpadr=" + mcpadrLocal;
	
	var xhttp = new XMLHttpRequest();

	xhttp.open("POST", "entdata", true);
	xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	xhttp.onreadystatechange = function() {
		if (this.readyState === 4 && this.status === 200) {
			// document.getElementById("relaystateid").innerHTML =
			// this.responseText;
			// relayLabelA = this.responseText;
			// setButtonNames();
		
			// this alert happens if the user (http browser) is
			// by passed to cause a submission
			if(this.responseText !== "okay"){
				alert("A data field is invalid.");
				return;
			}else{
				mcpadr = document.getElementById("mcpadr").value;
				validateSsid();
			}
		}	
	};
	xhttp.send(sendData);
}

function newaddssidSend(isNew, nuSsid, nuSsidPwd){
	var useDevMacEle = document.getElementById('macchk');
	var useDhcpEle = document.getElementById('dhcpchk');
	
	var macAdr;
	if(!useDevMacEle.checked){
		macAdr = subDataGetIpGrp("mac", 6, ":");
	}
	
	var ipAdr;
	var ipGway;
	var ipSubnet;
	
	if(!useDhcpEle.checked){
	 ipAdr = subDataGetIpGrp("ip", 4, ".");
	 ipGway = subDataGetIpGrp("ipgw", 4, ".");
	 ipSubnet = subDataGetIpGrp("ipmsk", 4, ".");
	 }
	
	var portHttp = document.getElementById("httpport").value;
	var portTcp = document.getElementById("tcpport").value;
	
	var ssidCh = document.getElementById("ssidch").value;
	
	var sendData = "";
	if(isNew){
		sendData += "ssidnuname=" + nuSsid + "&";
		sendData += "ssidpwd=" + nuSsidPwd + "&";
	}else{
		sendData += "ssidchgname=" + nuSsid + "&";
	}
	sendData += "ssidch=" + ssidCh + "&";
	
	if(!useDevMacEle.checked){
		sendData += "mac=false" +  "&";
		sendData += "macAdr=" + macAdr + "&";
	}else{
		sendData += "mac=true" +  "&";
	}
	
	if(!useDhcpEle.checked){
		sendData += "dhcp=false" +  "&";
		sendData += "ipAdr=" + ipAdr + "&";
		sendData += "ipGway=" + ipGway + "&";
		sendData += "ipSubnet=" + ipSubnet + "&";
	}else{
		sendData += "dhcp=true" +  "&";
	}
	sendData += "portHttp=" + portHttp + "&";
	sendData += "portTcp=" + portTcp;
	
	var xhttp = new XMLHttpRequest();

	xhttp.isNewStack = isNew;
	xhttp.nuSsidStack = nuSsid;
	xhttp.nuSsidPwdStack = nuSsidPwd;
  
	xhttp.open("POST", "entdata", true);
	xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	xhttp.onreadystatechange = function() {
		if (this.readyState === 4 && this.status === 200) {
			// document.getElementById("relaystateid").innerHTML =
			// this.responseText;
			// relayLabelA = this.responseText;
			// setButtonNames();
		
			// this alert happens if the user (http browser) is
			// by passed to cause a submission
			if(this.responseText !== "okay"){
				alert("A data field is invalid.");
				return;
			}
			if(this.isNewStack){
				var a = this.nuSsidStack;
				var b = this.nuSsidPwdStack;
				g_newSsidArr.push( {a, b} );
				newaddssidDone(this.nuSsidStack);
			}else{
				var addssidbutt = document.getElementById("addssidbutt");
				
				disableAdd(addssidbutt, true)
				clearSsidListChkbox(false);
			}
		}	
	};
	xhttp.send(sendData);
}

function newaddssidDone(nuSsid){
	// when the server has responded to the process
	// g_newSsidArr.push( {nuSsid,nuSsidPwd} );
	
	addSsidToList(nuSsid);
	addSsidField();
}


function ssidduplicate(nuSsidName){
	var ssidAddInpEle = document.getElementById('nussid');
	var ssiderrEle = document.getElementById('ssiderr');
	var ssidaddbuttEle = document.getElementById('addssidbutt');
	
	if(g_orderSsidArr !== undefined){
		for(var i = 0; i < g_orderSsidArr.length; i++){
			if(nuSsidName === g_orderSsidArr[i]){
				setInvalidState(ssidAddInpEle, true);
				ssidaddbuttEle.disabled = true;
				ssiderrEle.innerHTML = 'SSID already exists.';
				
				// alert('SSID already exists.');
				return true;
			}
		}
	}
	return false;
}

function removessidD(ele){
	// id='remssid1'
	var ssidNId = ele.id.substring(3);
	var ssidName = document.getElementById(ssidNId).value;
	
	if(g_deleteSsidArr === undefined){
		g_deleteSsidArr = new Array();
	}
	
	g_deleteSsidArr.push(ssidName);
	
	// remove from the new array first
	
	if(g_newSsidArr !== undefined){
		for(var i = 0; i < g_newSsidArr.length; i++){
			if(ssidName === g_newSsidArr[i][0]){
				// delete g_newSsidArr[i];
				g_newSsidArr.splice(i, 1);
			}
		}
	}
	if(g_orderSsidArr !== undefined){
		for(var i = 0; i < g_orderSsidArr.length; i++){
			if(ssidName === g_orderSsidArr[i]){
				// delete g_orderSsidArr[i];
				g_orderSsidArr.splice(i, 1);
				break;
			}
		}
	}
	addSsidField();
	ssidLimit();
}
function upssidD(ele){
	// id='upssid1'
	var ssidNId = ele.id.substring(2);
	var ssidName = document.getElementById(ssidNId).value;

	if(g_orderSsidArr !== undefined){
		for(var i = 0; i < g_orderSsidArr.length; i++){
			if(ssidName === g_orderSsidArr[i]){
				if(i !== 0){
					var datatmp = g_orderSsidArr[i];
					g_orderSsidArr[i] = g_orderSsidArr[i - 1];
					g_orderSsidArr[i-1] = datatmp;
					break;
				}
			}
		}
	}
	addSsidField();
}

function downssidD(ele){
	// id='downssid1'
	var ssidNId = ele.id.substring(4);
	var ssidName = document.getElementById(ssidNId).value;

	if(g_orderSsidArr !== null){
		for(var i = 0; i < g_orderSsidArr.length; i++){
			if(ssidName === g_orderSsidArr[i]){
				if( i !==  g_orderSsidArr.length - 1){
					var datatmp = g_orderSsidArr[i];
					g_orderSsidArr[i] = g_orderSsidArr[i + 1];
					g_orderSsidArr[i+1] = datatmp;
					break;
				}
			}
		}
	}
	addSsidField();
}
function addSsidToList(ssidname){
	g_orderSsidArr.push(ssidname);
}

function ssidNRow(number, ssidName){
	var ssidNId = 'ssid' + number;
	
	var ssidRowTr = document.createElement('tr');
	
	var td1Row = document.createElement('td');
	ssidRowTr.appendChild(td1Row);
	
	var ssidNTable = document.createElement('table');
	ssidNTable.setAttribute('class', 'ssiditem');
	td1Row.appendChild(ssidNTable);
	
	var ssidNTr = document.createElement('tr');
	ssidNTable.appendChild(ssidNTr);
	
	var td0 = document.createElement('td');
	ssidNTr.appendChild(td0);
	
	var inpchkbox = document.createElement('input');
	td0.appendChild(inpchkbox);
	
	inpchkbox.setAttribute('type', 'checkbox');
	inpchkbox.setAttribute('id', "chkbx" + ssidNId);
	inpchkbox.setAttribute('name', "chkbx" + ssidNId);
	inpchkbox.setAttribute('value', ssidName);
	inpchkbox.setAttribute('onclick', 'ssidcbaction(this);');
	inpchkbox.checked = false;

	var td1 = document.createElement('td');
	ssidNTr.appendChild(td1);
	
	td1.innerHTML = ssidNId.toUpperCase() + " ";
	
	var inpssidname = document.createElement('input');
	td1.appendChild(inpssidname);
		
	inpssidname.setAttribute('class', 'clrwhite');
	inpssidname.setAttribute('type', 'text');
	inpssidname.setAttribute('id', ssidNId);
	inpssidname.setAttribute('name', ssidNId);
	inpssidname.setAttribute('value', ssidName);
	inpssidname.setAttribute('size', '20');
	inpssidname.setAttribute('readonly', '');
	
	var td3 = document.createElement('td');
	ssidNTr.appendChild(td3);
	
	var inpssidremovebutt = document.createElement('button');
	inpssidremovebutt.setAttribute('id', 'rem' + ssidNId);
	inpssidremovebutt.setAttribute('onclick', 'removessid(this);');
	inpssidremovebutt.innerHTML = 'Remove';
	
	td3.appendChild(inpssidremovebutt);
	ssidNTr.appendChild(td3);
	
	var td4 = document.createElement('td');
	ssidNTr.appendChild(td4);
	
	if(number !== 1){
		var inpssid1upbutt = document.createElement('button');
		inpssid1upbutt.setAttribute('id', 'up' + ssidNId);
		inpssid1upbutt.setAttribute('onclick', 'upssid(this);');
		inpssid1upbutt.innerHTML = 'up';
		
		td4.appendChild(inpssid1upbutt);
	}

	var td5 = document.createElement('td');
	
	if(number < g_orderSsidArr.length){
		var inpssid1downbutt = document.createElement('button');
		ssidNTr.appendChild(td5);
		
		inpssid1downbutt.setAttribute('id', 'down' + ssidNId);
		inpssid1downbutt.setAttribute('onclick', 'downssid(this);');
		inpssid1downbutt.innerHTML = 'down';
		
		td5.appendChild(inpssid1downbutt);
	}
	
	return ssidRowTr;
}

function addssidRow(){
	var ssidTr = document.createElement('tr');
	
	var td1 = document.createElement('td');
	ssidTr.appendChild(td1);
	
	var tabdata = document.createElement('table');
	td1.appendChild(tabdata);
	
	tabdata.setAttribute('class', 'ssidinp');
	
	var tr1data = document.createElement('tr');
	tabdata.appendChild(tr1data);
	
	var td0data = document.createElement('td');
	tr1data.appendChild(td0data);
	
	// add the ssid and password fields
	var td1data = document.createElement('td');
	tr1data.appendChild(td1data);

	var inpssidaddbutt = document.createElement('button');
	td1data.appendChild(inpssidaddbutt);
	
	inpssidaddbutt.setAttribute('id', 'addssidbutt');
	inpssidaddbutt.setAttribute('onclick', 'newaddssid();');
	inpssidaddbutt.setAttribute('onchange', 'addSsidChg(this);');
	inpssidaddbutt.innerHTML = 'Add';

	//
	var td1 = document.createElement('td');
	tr1data.appendChild(td1);
	//
	var inpssidname = document.createElement('input');
	tr1data.appendChild(inpssidname);
	
	inpssidname.setAttribute('type', 'text');
	inpssidname.setAttribute('id', 'nussid');
	inpssidname.setAttribute('name', 'nussid');
	inpssidname.setAttribute('value', '');
	inpssidname.setAttribute('size', '20');
	inpssidname.setAttribute('onkeyup', 'keyActionSsid(this,event);');
	
	//
	var td2 = document.createElement('td');
	tr1data.appendChild(td2);
	
	td2.innerHTML = "SSID key ";
	
	var inpssidpw = document.createElement('input');
	inpssidpw.setAttribute('type', 'password');
	inpssidpw.setAttribute('id', 'nussidpw');
	inpssidpw.setAttribute('name', 'nussidpw');
	inpssidpw.setAttribute('value', '');
	inpssidpw.setAttribute('size', '40');
	inpssidpw.setAttribute('onkeyup', 'keyActionSsidKey(this,event);');
	
	td2.appendChild(inpssidpw);
	
	// add the ssid and password fields error field
	var tr2data = document.createElement('tr');
	tabdata.appendChild(tr2data);
	
	var td1dataerr = document.createElement('td');
	tr2data.appendChild(td1dataerr);
	td1dataerr.setAttribute('id', 'ssiderr');
	td1dataerr.setAttribute('colspan', '4');
	td1dataerr.setAttribute('style', 'color: red; height: 20px;');
	td1dataerr.innerHTML = " ";
	
	return ssidTr;
}

function ssidLimit(){
	var boolAtLimit = g_orderSsidArr.length >= 5;

	document.getElementById('addssidbutt').disabled = boolAtLimit;
	document.getElementById('nussid').disabled = boolAtLimit;
	document.getElementById('nussidpw').disabled = boolAtLimit;
	
	return boolAtLimit;
}

function keyActionSsid(ele, evt){
	if(evt.code === "Tab"){
		return;
	}
	validateSsid();
}

function validateSsid(){
	if(ssidLimit()){
		return;
	}
	
	var ele = document.getElementById('nussid');
	var ssidaddbuttEle = document.getElementById('addssidbutt');
	
	var ssiderrEle = document.getElementById('ssiderr');
	ssiderrEle.innerHTML = " ";
	
	if(mcpadr === "ff"){
		setInvalidState(ele, true);
		ssidaddbuttEle.disabled = true;
		ssiderrEle.innerHTML = "MCP address needs to be set first.";
		
		ele.value = "";
		ele.style.background = 'white';
		return;
	}
	
	setInvalidState(ele, false);
	clearSsidListChkbox(true);
	
	var v = ele.value;
	if(v === ""){
		setInvalidState(ele, true);
		disableAdd(ssidaddbuttEle, true);
		clearSsidListChkbox(false);
		ele.style.background = 'white';
		return;
	}
	disableAdd(ssidaddbuttEle, false);
	
	var len = v.length;
	if(len < 2 || len > 32){
		setInvalidState(ele, true);
		ssiderrEle.innerHTML = "2 to 32 characters limit.";
		return;
	}
	
	var v1 = v.substring(0,1);
		
	if(v1 === " " || v1 === "!" || v1 === "#"  || v1 === ";"){
		setInvalidState(ele, true);
		ssiderrEle.innerHTML = "1st char may not be --- space ! # ; ---";
		return;
	}
	// go through each character and check
	for(var i = 0; i < len; i++){
		var c = v.charAt(i);

		if(c === '"' || c === '$' || c === '+' || c === '?' ||
		   c === '[' || c === "\\" || c === ']'){
			setInvalidState(ele, true);
			ssiderrEle.innerHTML = "Invalid chars --- ' ? \" $ [ \\ ] + ---";
			return;
		}
	}
	var vLast = v.substring(v.length -1);
	if(vLast === " "){
		setInvalidState(ele, true);
		ssiderrEle.innerHTML = "Last char may not be --- space ---";
		return;
	}
	// The SSID can be any alphanumeric, case-sensitive entry from 2 to 32 characters. 
	// The printable characters plus the space (ASCII 0x20) are allowed, but these 
	// six characters are not:
	// ?, ", $, [, \, ], and +.
	// The allowable characters are:
	// ASCII 0x20, 0x21, 0x23, 0x25 through 0x2A, 0x2C through 0x3E, 0x40 
	// through 0x5A, 0x5E through 0x7E.
	// In addition, these three characters cannot be the first character:
	// !, #, and ; (ASCII 0x21, 0x23, and 0x3B, respectively).
	// Trailing and leading spaces (ASCII 0x20) are not permitted. 	
	ssidduplicate(v);
}

function keyActionSsidKey(ele, evt){
	if(evt.code === "Tab"){
		return;
	}
	validateSsidKey();
}

function validateSsidKey(){
	if(ssidLimit()){
		return;
	}
	var ele = document.getElementById('nussidpw');
	
	var ssiderrEle = document.getElementById('ssiderr');
	ssiderrEle.innerHTML = " ";
	
	setInvalidState(ele, false);
	clearSsidListChkbox(true);
	
	var v = ele.value;
	if(v === ""){
		setInvalidState(ele, true);
		clearSsidListChkbox(false);
		ele.style.background = 'white';
		return;
	}
	var len = v.length;
	if(len < 8 || len > 63){
		setInvalidState(ele, true);
		ssiderrEle.innerHTML = "8 to 63 characters limit.";
		return;
	}
	
	var v1 = v.substring(0,1);
		
	if(v1 === " "){
		setInvalidState(ele, true);
		ssiderrEle.innerHTML = "1st char may not be --- space";
		return;
	}
	var vLast = v.substring(v.length -1);
	if(vLast === " "){
		setInvalidState(ele, true);
		ssiderrEle.innerHTML = "Last char may not be --- space ---";
		return;
	}
}

function clearSsidListChkbox(disabledState){
	for(var i = 1 ; i <= 5; i++){
		var nameEleId = "chkbxssid" + i;
		
		var eleN = document.getElementById(nameEleId);
		if(eleN !== null){
			eleN.checked = false;
			eleN.disabled = disabledState;
		}
	}
}

function ssidcbaction(ele){
	var key = ele.id.substring(5); // chkbxssid1
	
	var initState = (ele.checked === true);
	clearSsidListChkbox(false);
	ele.checked = initState;

	if(initState){
		// need to get the data from the server and put it in place
		ssidDataAction(ele, "query", key);
	}else{
		ssidShowdata(ele);
	}
}

function removessid(ele){
	var key = ele.id.substring(3); // remssid1
	ssidDataAction(ele, "remove", key);
	
	removessidD(ele);
}

function upssid(ele){
	var key = ele.id.substring(2); // upssid1
	ssidDataAction(ele, "up", key);
	
	upssidD(ele);
}

function downssid(ele){
	var key = ele.id.substring(4); // downssid1
	ssidDataAction(ele, "down", key);
	
	downssidD(ele);
}

function ssidDataAction(ele, action, key){
	var ssidNamEle = document.getElementById(key);
	var ssidName = ssidNamEle.value;
	
	var sendData = "";
	sendData += "ssidName=" + ssidName + "&";
	sendData += "ssidact=" + action;
	
	var xhttp = new XMLHttpRequest();
  
	xhttp.open("POST", "ssiddata", true);
	xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	xhttp.onreadystatechange = function() {
		if (this.readyState === 4 && this.status === 200) {
			//
			// var ssidch = "auto";
			// var macAddr = "xx:xx:xx:zz:zz:zz";
			// var ipAddr = "192.168.4.1";
			// var ipGateway = "192.168.4.1";
			// var ipSubnet = "255.255.255.0";
			// var httpport = "80";
			// var tcpport = "81"
		
			// this alert happens if the user (http browser) is
			// by passed to cause a submission
			if(this.responseText === "not okay"){
				alert("A data field is invalid: ssiddata.");
				return;
			}
			
			if(this.responseText === "okay"){
				return;
			}
			// is a query request response
			
			// breakdown the response and fill in the necessary
			// fields
			var splitAmpersand = this.responseText.split("&");
			
			for(var i = 0; i < splitAmpersand.length; i++){
				var itemArr = splitAmpersand[i].split("=");
				
				var key = itemArr[0];
				var data = itemArr[1];
				
				if(key === "ssidch"){
					ssidch = data;
					continue;
				}
				if(key === "mac"){
					mac = data;
					continue;
				}
				if(key === "macadr"){
					macAddr = data;
					continue;
				}
				if(key === "http"){
					httpport = data;
					continue;
				} 
				if(key === "tcp"){
					tcpport = data;
					continue;
				}
				if(key === "dhcp"){
					dhcp = data;
					continue;
				}
				if(key === "ipadr"){
					ipAddr = data;
					continue;
				}
				if(key === "ipgw"){
					ipGateway = data;
					continue;
				}
				if(key === "ipsn"){
					ipSubnet = data;
					continue;
				}
			}
			setSsidCh();
			setIpAddrFor();
			setMacAddr();
			
			ssidShowdata(ele);			
		}	
	};
	xhttp.send(sendData);
}

function ssidShowdata(eleChkbox, initState){
	if(initState){
		var ssidAddEle = document.getElementById("addssidbutt");
		disableAdd(ssidAddEle, true);
	}
	
	var eleId = eleChkbox.id;
	eleId = eleId.substring(5);
	
	var ssidNameStr = document.getElementById(eleId);
	
	var conndataEle = document.getElementById("conndatadiv");
	
	if(!eleChkbox.checked){
		conndataEle.style.display = 'none';
	}else{
		conndataEle.style.display = 'block';
	}
}

function setInvalidState(ele, inValid){
	if(inValid){	
		ele.style.backgroundColor = '#FF6666';
		ele.style.color = 'white';
		ele.inerror = '999';
	}else{
		ele.style.backgroundColor = 'white';
		ele.style.color = 'black';
		ele.inerror = '0';
	}
	// find all the input tagged items so as to set the 
	// add-ssid button based on all input fields of the 
	// page
	//
	var inputArr = document.getElementsByTagName('input');
	var subdatabuttEle = document.getElementById('addssidbutt');
	
	var ssidInp = document.getElementById('nussid');
	if(ssidInp.value === ""){
		ssidInp.inerror = '999';
	}
	
	var ssidKeyInp = document.getElementById('nussidpw');
	
	if(subdatabuttEle === null){
		return;
	}
	for(var i = 0; i < inputArr.length; i++){
		var inpEle = inputArr[i];
		
		if(inpEle.inerror === '999'){
			// would have been nice to use the color code for the above test
			// but chrome is changing values and creates inconsistency
			subdatabuttEle.disabled = true;
			subdatabuttEle.style.color = "red";
			return;
		}
	}
	subdatabuttEle.disabled = false;
	subdatabuttEle.style.color = "black";
}

function actssid(type){
	var b64Encrypted = enPassword(document.getElementById("pwdcfg").value);
	console.log(b64Encrypted);
	
	var xhttp = new XMLHttpRequest();
	var parms = "";
	parms += 'pwdacc=' + b64Encrypted + '&';
	parms += 'type=' + type;
	
	xhttp.open("POST", "actssid", true);
	xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  
	xhttp.onreadystatechange = function() {
		if (this.readyState === 4){
			if(this.status === 200) {
				// if wish to do something
				var bodyEle = document.getElementsByTagName('body')[0];
				while (bodyEle.firstChild) {
					bodyEle.removeChild(bodyEle.firstChild);
				}
				var restartLabelEle = document.createElement('label');
				restartLabelEle.style.color = 'red';
				restartLabelEle.innerHTML = "Reboot requested: need to re-post page after it completes. (A few seconds.)";	
				
				bodyEle.appendChild(restartLabelEle);
			}
		}
	};
	xhttp.send(parms);
}

function hex2a(hex) {
    var str = '';
    for (var i = 0; i < hex.length; i += 2)
        str += String.fromCharCode(parseInt(hex.substr(i, 2), 16));
    return str;
}



