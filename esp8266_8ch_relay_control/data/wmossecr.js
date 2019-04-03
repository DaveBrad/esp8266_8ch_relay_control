var nopwd = "d";

function onloadFunc(){
	// if the password value is filled in then the Change password message
	if(nopwd === ""){
		var chgpwdEnt = document.getElementById('chgpwd');
		chgpwdEnt.parentNode.removeChild(chgpwdEnt);

		var cfgfieldsEle = document.getElementById("configfields");
		cfgfieldsEle.parentNode.removeChild(cfgfieldsEle);
	}else{
		var entpwdEle = document.getElementById('entpwd');
		entpwdEle.parentNode.removeChild(entpwdEle);
		
		var chgpwddivEnt = document.getElementById('pwddiv');
		chgpwddivEnt.style.display = 'none';
		
		addSsidField();
	}
	onloadSEDA();
}

function togglechgpwd(){
	var chgpwddivEnt = document.getElementById('pwddiv');
	
	var toggle;
	if(chgpwddivEnt.style.display === 'none'){
		toggle = 'block';
	}else{
		toggle = 'none';
	}
	chgpwddivEnt.style.display = toggle;
}

function subpwd(){
	var currpwd = document.getElementById("pwdnow");
	var pwd = document.getElementById("pwd1");
	var pwdconfirm = document.getElementById("pwd1c");
	
	if(pwd.value !== pwdconfirm.value){
		alert("Mis-match password entry.");
		return;
	}
	var xhttp = new XMLHttpRequest();
  
	xhttp.open("POST", "pwd", true);
	xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	
	var cryptO = "setpwd=" + enPassword(pwd.value) + "&";
	cryptO += "currpwd=" + enPassword(currpwd.value);
	
	xhttp.onreadystatechange = function() {
		// remain on this page
		document.location.reload();	
	};
	xhttp.send(cryptO);
}

function keyAction(ele, evt){
	var pwd1Ele = document.getElementById("pwd1");
	var pwd1cEle = document.getElementById("pwd1c");
	
	var subpwdbuttEle = document.getElementById("pwdbutt");
	subpwdbuttEle.disabled = true;
	
	var pwd1Text = pwd1Ele.value;
	var pwd1Len = pwd1Text.length;
	var pwd1cText = pwd1cEle.value;
	var pwd1cLen = pwd1cText.length;
	
	if(pwd1Len > 16){
		alert("'Enter password' must be 8-16 characters.");
		pwd1cEle.value = "";
		return;
	}
	if(pwd1cEle === ele){
		// 
		if(pwd1Len < 8 || pwd1Len > 16){
			alert("'Enter password' must be 8-16 characters.");
			pwd1cEle.value = "";
			return;
		}
		// check that they match
		if(	pwd1cLen > pwd1Len){
			alert("Password length mismatch.");
			pwd1cEle.value = pwd1cText.substring(0, pwd1Len);
			return;
		}		
		if(	pwd1cLen === pwd1Len){
				
			if(pwd1Ele.value !== pwd1cEle.value){
				alert("Password character mismatch.");
				return;
			}
			subpwdbuttEle.disabled = false;
		}
		return;
	}
	// get the field id name for processing
	var fieldName = ele.id;
	if (evt !== undefined) {
		var c = evt.which;
	
		if (c === 13 || c === 10) {
			// disallow cr lf
			return;
		}
	}
}

function enCryptData(rawdata){
	return rawdata;
    // return CryptoJS.AES.encrypt(rawdata, "Secret Passphras");
 }

function encrypt(txt, cryptkey) {
    var cipher = crypto.createCipher('aes-256-cbc', cryptkey);
    var crypted = cipher.update(txt, 'utf8', 'hex');
    crypted += cipher.final('hex');
    return crypted;
}

// encrypt('1', 'key'); // 83684beb6c8cf063caf45cb7fad04a50



/** Not a valid IP 0 to 255 range. */
function not0To255(value){
	return (value < 0 ||  value > 255 || isNaN(value) || value === "");
}

/** h ASCII hex representation is 2 digits and HEX characters. 
*
* return true if HEX and 2 digit representation
*/
function isHex16DigMax(h, digits) {
	var a = parseInt(h,16);
	//             123456789012345
	var aPadded = "000000000000000" + a.toString(16);
	aPadded = aPadded.slice(-1 * digits);
	
	return (aPadded === h.toLowerCase());
}

/**
* Validate SSID channel range.
*/

function ssidchValid(ele){
	var valText = ele.value;
	
	var value = "";
	
	if(valText.toLowerCase() === 'auto'){
		setInvalidState(ele, false);
		return;
	}else{
		// expecting a number 1-11 North America, 1-14 elsewhere
		// as this is a recieve channel (not transmit) it operates
		// as designed legally in station mode.
		value = parseInt(valText);
	}
	var channelState = (value < 1 ||  value > 14 || isNaN(value) || value === "");

	setInvalidState(ele, channelState);
}
/**
* Validate port range.
*/
function portValid(ele){
	var value = parseInt(ele.value);
	
	var portState = (value < 0 ||  value > 65355 || isNaN(value) || value === "");
		
	setInvalidState(ele, portState);
}
	
/**
 * Validate MAC address fields for a 2 digit (1 digit is an error) and
 * indicate if in error.
*/
function macValid(ele){
	if(ele.classList.contains('usedevmac')){
		setInvalidState(ele, false);
		return;
	}
	var valStr = ele.value;
	var value = parseInt(valStr, 16);
	
	var stateInval = (!isHex16DigMax(valStr, 2) || not0To255(value));
					
	setInvalidState(ele, stateInval);
}
function mcpValid(ele){
	var valStr = ele.value;
	
	// convert to value
	var value = parseInt(valStr, 16);
	
	var stateInval = (!isHex16DigMax(valStr, 2) || notX20ToX27(value));
	
	setInvalidState(ele, stateInval);
	
	var setMcpEle = document.getElementById('mcpadrset');
	
	setMcpEle.disabled = stateInval;
	
}

function notX20ToX27(val){
	return (val < 0x20 || val > 0x27);
}

function ipValid(eleIpFld){
	if(eleIpFld.parentNode.classList.contains('usedhcp')){
		setInvalidState(eleIpFld, false);
		return;
	}
	setInvalidState(eleIpFld, not0To255(eleIpFld.value));
}

function restartboard(){
	var xhttp = new XMLHttpRequest();
	var params = 'inp=rstart';
	xhttp.open("POST", "RESTART", true);
  
	xhttp.onreadystatechange = function() {
		if (this.readyState === 4 && this.status === 200) {
			// document.getElementById("relaystateid").innerHTML =
			// this.responseText;
			// relayLabelA = this.responseText;
			// setButtonNames();
		}	
	};
	xhttp.send(params);
}

