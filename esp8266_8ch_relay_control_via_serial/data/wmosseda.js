var mcpadr = "-1";
var macAddr = "xx:xx:xx:zz:zz:zz";


var mac = "true";
var ssidch = "auto";
var dhcp = "true";
var ipAddr = "nnn.nnn.nnn.nnn";
var ipGateway = "nnn.nnn.nnn.nnn";
var ipSubnet = "255.255.255.m";
var httpport = "80";
var tcpport = "81";

function onloadSEDA(){
	var ipinpdivEle = document.getElementById('ipinputdiv');
	if(ipinpdivEle === null){
		return;
	}
	
	var ipinpdivoutrEle = ipinpdivEle.parentNode;

	// remove and rebuild
	ipinpdivoutrEle.removeChild(ipinpdivEle);
	
	ipinpdivoutrEle.appendChild( ipinputdiv() );
	
	setMcpAddr();
	setMacAddr();
	validMcpAdrs();
	validDataIpArr();
	validateSsid();
	validateSsidKey();
}

function validMcpAdrs(){
	var ele = document.getElementById('mcpadr');
	mcpValid(ele);
}

function validDataIpArr(){
	var ipIdStrArr = ['ip','ipgw', 'ipmsk'];
	
	for(var i = 0; i < ipIdStrArr.length; i++){
		for(var j = 1; j < 5; j++){
			var eleIdStr= ipIdStrArr[i] + j;		
			var ipEle = document.getElementById(eleIdStr);
			ipValid(ipEle);
		}
	}
	portValid(document.getElementById('httpport'));
	portValid(document.getElementById('tcpport'));
	
	ssidchValid(document.getElementById('ssidch'));
}

function setIpAddrFor(){
	// depending on the DHCP setting need to set how things are displayed
	// and what is set
	var useDhcpEle = document.getElementById("dhcpchk");
	useDhcpEle.checked = dhcp === "true";
	
	// controls how IP fields are displayed
	useDhcp(useDhcpEle);
	
	if(!useDhcpEle.checked){
		var ipIdStrArr = ['ip','ipgw', 'ipmsk'];
		var ipIdValueArr = [ipAddr, ipGateway, ipSubnet];
		
		for(var i = 0; i < ipIdStrArr.length; i++){
			
			var ipAdrStrArr = ipIdValueArr[i].split(".");
			
			for(var j = 1; j < 5; j++){
				var eleIdStr = ipIdStrArr[i] + j;
				var ipEle = document.getElementById(eleIdStr);
				ipEle.value = ipAdrStrArr[j -1];
				ipValid(ipEle);	
			}
		}
	}
	document.getElementById('httpport').value = httpport;
	document.getElementById('tcpport').value = tcpport;
}

function setMcpAddr(){
	var mcpEle = document.getElementById('mcpadr');
	mcpEle.value =  "" + mcpadr;
	 
	mcpValid(mcpEle);
}
function setMacAddr(){
	var useDevMacEle = document.getElementById("macchk");
	useDevMacEle.checked = mac === "true";
	
	useDevMAC(useDevMacEle);

	if(!useDevMacEle.checked){
		// split the MAC address
		var macAddrArr = macAddr.split(":");
		
		for(var i = 0; i < 6; i++){
			var macNEle = document.getElementById('mac' + (i+1));
			macNEle.value =  "" + macAddrArr[i];
			
			macNEle.classList.remove("usedevmac");
			macNEle.classList.add("defmac");
		 
			macValid(macNEle);
		}
	}
}
function setSsidCh(){
	// split the MAC address
	var ssidChEle = document.getElementById('ssidch');
	
	ssidChEle.value = (ssidch === -1) ? 'auto' : ssidch;
}

function ipinputdiv(){
	var ipinputdivEle = document.createElement('div');
	
	var tabelEle = document.createElement('table');
	tabelEle.setAttribute('class', 'tabdata');
	ipinputdivEle.appendChild(tabelEle);

	var tr1Ele = document.createElement('tr');
	tabelEle.appendChild(tr1Ele);
	
	var td1Ele = document.createElement('td');
	tr1Ele.appendChild(td1Ele);

	td1Ele.appendChild(allConfig());
	
	return ipinputdivEle;
}

function allConfig(){
	
	var tabelEle = document.createElement('table');
	tabelEle.setAttribute('class', 'tabdata');
	
	var tr1Ele = document.createElement('tr');
	tabelEle.appendChild(tr1Ele);
	
	// SSID channel row
	var tr0Ele = document.createElement('tr');
	tabelEle.appendChild(tr0Ele);
	
	var td01Ele = document.createElement('td');
	tr0Ele.appendChild(td01Ele);
	td01Ele.innerHTML = 'SSID channel';
	
	var td02Ele = document.createElement('td'); //ssidch
	tr0Ele.appendChild(td02Ele);
	
	var inpSsidChEle = document.createElement('input');
	td02Ele.appendChild(inpSsidChEle);
	
	inpSsidChEle.setAttribute('type', 'text');
	inpSsidChEle.setAttribute('id', 'ssidch');
	inpSsidChEle.setAttribute('name', 'ssidch');
	inpSsidChEle.setAttribute('size', '4');
	inpSsidChEle.setAttribute('maxlength', '4');
	inpSsidChEle.setAttribute('onkeyup', 'ssidchValid(this);');
		
	inpSsidChEle.setAttribute('value', ssidch);
	
	
	var td03Ele = document.createElement('td');
	tr0Ele.appendChild(td03Ele);
	td03Ele.innerHTML = 'Connection via this channel or "auto".[<span class="chan">Channels 1-11 all regions, 12-14 region dependent.</span>]';
	
	// MAC row
	var tr1Ele = document.createElement('tr');
	tabelEle.appendChild(tr1Ele);
	
	var td1Ele = document.createElement('td');
	tr1Ele.appendChild(td1Ele);
	td1Ele.innerHTML = 'Config MAC addr';
	
	var td2Ele = document.createElement('td');
	tr1Ele.appendChild(td2Ele);
	
	td2Ele.appendChild(macAddrDom());
	
	var td3Ele = document.createElement('td');
	tr1Ele.appendChild(td3Ele);
	
	td3Ele.innerHTML = 'Device MAC address local-router will assign static IP address to. (Fields required 2 Hex digit format.)';
	
	// - - - - -  config IP   DHCP
	var dhcpEleTr = document.createElement('tr');
	var dhcpEleTd0 = document.createElement('td');
	dhcpEleTd0.appendChild(document.createTextNode('Config IP'));
	dhcpEleTr.appendChild(dhcpEleTd0);
	
	var dhcpEleTd = document.createElement('td');
	dhcpEleTd.setAttribute('colspan', '2');
	
	dhcpEleTr.appendChild(dhcpEleTd);
	
	var dhcpLabel = document.createElement('label');
	var dhcpChkbox = document.createElement('input');
	dhcpChkbox.setAttribute('id', 'dhcpchk');
	dhcpChkbox.setAttribute('type', 'checkbox');
	dhcpChkbox.setAttribute('onclick','useDhcp(this);');
	// dhcpChkbox.setAttribute('checked','');
	dhcpChkbox.checked = dhcp === "true";

	dhcpLabel.appendChild(dhcpChkbox);
	dhcpLabel.appendChild(document.createTextNode('Use DHCP'));
	
	dhcpEleTd.appendChild(dhcpLabel);
	
	tabelEle.appendChild(dhcpEleTr);
	
	// - - - - -  ip address
	ipRow(
		tabelEle, 
		'Set IP addr',
		'ip', 
		'ip', 
		ipAddr,
		'The IP address this device is to have when connected to your local network.'); 
		
	ipRow(
		tabelEle, 
		'Set IP gateway addr',
		'ip', 
		'ipgw', 
		ipGateway,
		'The gateway IP address when connected to your local network.'); 
		
	ipRow(
		tabelEle, 
		'Set IP subnet mask',
		'ip', 
		'ipmsk', 
		ipSubnet,
		'The IP subnet mask for your local network.');
		
	// - - - - - http port
	var tr2Ele = document.createElement('tr');
	tabelEle.appendChild(tr2Ele);
	
	tr2Ele.setAttribute('class', 'port');
	
	td1Ele = document.createElement('td');
	tr2Ele.appendChild(td1Ele);
	td1Ele.innerHTML = 'HTTP port';
	
	td2Ele = document.createElement('td');
	tr2Ele.appendChild(td2Ele);
	
	var inpEle = document.createElement('input');
	td2Ele.appendChild(inpEle);
		
	inpEle.setAttribute('type', 'text');
	inpEle.setAttribute('id', 'httpport');
	inpEle.setAttribute('name', 'httpport');
	inpEle.setAttribute('size', '5');
	inpEle.setAttribute('maxlength', '5');
	inpEle.setAttribute('onkeyup', 'portValid(this);');
		
	inpEle.setAttribute('value', httpport);
	
	td3Ele = document.createElement('td');
	tr2Ele.appendChild(td3Ele);
	
	td3Ele.innerHTML = 'The port number to be used for HTTP access (default is 80)';
	tr2Ele.appendChild(td3Ele);
	
		
	// - - - - tcp port
	var tr6Ele = document.createElement('tr');
	tabelEle.appendChild(tr6Ele);
	
	tr6Ele.setAttribute('class', 'port');
	
	td1Ele = document.createElement('td');
	tr6Ele.appendChild(td1Ele);
	td1Ele.innerHTML = 'TCP port access';
	
	td2Ele = document.createElement('td');
	tr6Ele.appendChild(td2Ele);
	
	var inpEle = document.createElement('input');
	td2Ele.appendChild(inpEle);
		
	inpEle.setAttribute('type', 'text');
	inpEle.setAttribute('id', 'tcpport');
	inpEle.setAttribute('name', 'tcpport');
	inpEle.setAttribute('size', '5');
	inpEle.setAttribute('maxlength', '5');
	inpEle.setAttribute('onkeyup', 'portValid(this);');
		
	inpEle.setAttribute('value', tcpport);
	
	td3Ele = document.createElement('td');
	tr6Ele.appendChild(td3Ele);
	
	td3Ele.innerHTML = 'The device may be controlled directly and this is the port (within local network)';
	
	// ------------
	return tabelEle;
}

function ipRow(appendTo, td1Str, iptrClaz, idText, ipSrc, td3Str){
	
	var tr3Ele = document.createElement('tr');
	appendTo.appendChild(tr3Ele);
	tr3Ele.setAttribute('class', 'usedhcptr');
	
	td1Ele = document.createElement('td');
	
	tr3Ele.appendChild(td1Ele);
	td1Ele.innerHTML = td1Str;
	
	td2Ele = document.createElement('td');
	tr3Ele.appendChild(td2Ele);
	
	ipAddrDom(td2Ele, iptrClaz, idText, ipSrc); 
	
	td3Ele = document.createElement('td');
	tr3Ele.appendChild(td3Ele);
	
	td3Ele.innerHTML = td3Str;
}

function ipAddrDom(tdEle, trClazz, idText, ipSrc){
	
	var tabIp = document.createElement('table');
	tdEle.appendChild(tabIp);
	
	var trIp = document.createElement('tr');
	tabIp.appendChild(trIp);
	
	trIp.setAttribute('class', trClazz); 
	
	// 4 ip values
	var splitIpAddrArr = ipSrc.split(".");
	
	var i;
	for(i = 0; i < 4; i++){
		j = i + 1;
		var ipStr = idText + j;
		
		var tdInp = document.createElement('td');
		trIp.appendChild(tdInp);
		tdInp.setAttribute('class', 'usedhcp');

		var inpEle = document.createElement('input');
		tdInp.appendChild(inpEle);
		
		inpEle.setAttribute('type', 'text');
		inpEle.setAttribute('id', ipStr);
		inpEle.setAttribute('name', ipStr);
		inpEle.setAttribute('size', '3');
		inpEle.setAttribute('maxlength', '3');
		inpEle.setAttribute('onkeyup', 'ipValid(this);');
		
		inpEle.setAttribute('value', splitIpAddrArr[i]);
	}
}

function macAddrDom(){
	var i = 1;
	
	var tabmac = document.createElement('table');
	tabmac.setAttribute('class', 'tabmac');
	
	
	var tr0Ele = document.createElement('tr');
	tabmac.appendChild(tr0Ele);
	
	var tdEle = document.createElement('td');
	tdEle.setAttribute('colspan', 6);
	tr0Ele.appendChild(tdEle);
	
	var labelChkbox = document.createElement('label');
	labelChkbox.setAttribute('for', 'chk1');
	
	var useDevMACChkbox = document.createElement('input');
	useDevMACChkbox.setAttribute('type', 'checkbox');
	// useDevMACChkbox.setAttribute('checked', '');
	useDevMACChkbox.setAttribute('name', 'chk1');
	useDevMACChkbox.setAttribute('id', 'macchk');
	useDevMACChkbox.setAttribute('onclick', "useDevMAC(this);");
	useDevMACChkbox.checked = mac === "true";
	

	labelChkbox.appendChild(useDevMACChkbox);
	labelChkbox.appendChild(document.createTextNode("Use device MAC"));
	
	tdEle.appendChild(labelChkbox);
	
	var tr1Ele = document.createElement('tr');
	tabmac.appendChild(tr1Ele);
	
	tr1Ele.setAttribute('class', 'mac');
	
	for(i = 1; i <= 6; i++){
		var tdNEle = document.createElement('td');
		tr1Ele.appendChild(tdNEle);
		// tr1Ele.setAttribute('class', 'ddd usedevmac');
		
		var inpEle = document.createElement('input');
		tdNEle.appendChild(inpEle);
		
		inpEle.setAttribute('type', 'text');
		inpEle.setAttribute('id', 'mac' + i);
		inpEle.setAttribute('name', 'mac' + i);
		inpEle.setAttribute('size', '2');
		inpEle.setAttribute('maxlength', '2');
		inpEle.setAttribute('onkeyup', 'macValid(this);');
		inpEle.setAttribute('class', 'ddd usedevmac');
		
		//. . . . . . . . 01234567890123456789
		// var macAddr = "xx:xx:xx:zz:zz:zz";
		var lower = (i -1) * 3;   // i=1 => 0
		var upper = (i * 3) - 1;  //     => 2
		
		inpEle.setAttribute('value', macAddr.substring(lower, upper));
	}
	return tabmac;
}
function useDevMAC(ele){
	var from;
	var to;
	
	if(!ele.checked){
		from = 'usedevmac';
		to = 'defmac';
	}else{
		from = 'defmac';
		to = 'usedevmac';
	}
	var useMACArr = document.getElementsByClassName(from);
	
	while(useMACArr.length > 0){
		var eleI = useMACArr[0];
		
		eleI.classList.remove(from);
		eleI.classList.add(to);	
	}
}

function useDhcp(ele){
	var from;
	var to;
	
	if(!ele.checked){
		from = 'usedhcp';
		to = 'defip';
	}else{
		from = 'defip';
		to = 'usedhcp';
	}
	var useArr = document.getElementsByClassName(from);
	
	while(useArr.length > 0){
		var eleI = useArr[0];
		
		eleI.classList.remove(from);
		eleI.classList.add(to);	
	}
	
	if(!ele.checked){
		from = 'usedhcptr';
		to = 'defiptr';
	}else{
		from = 'defiptr';
		to = 'usedhcptr';
	}
	var useArrTr = document.getElementsByClassName(from);
	
	while(useArrTr.length > 0){
		var eleI = useArrTr[0];
		
		eleI.classList.remove(from);
		eleI.classList.add(to);	
	}
}
