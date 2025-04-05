#ifndef SOURCE_H
#define SOURCE_H

#include <ESP8266WiFi.h>
#include <U8g2lib.h>
#include "espDMX_RDM.h"
#include "espArtNetRDM.h"
#include "wsFX.h"
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#define CLK 12
#define DT 13
#define ENCBTN 14

extern int frameCounter;
extern float factor;
extern unsigned long lastFpsCalc;
extern float fps;
extern int remainingFrames;
extern unsigned long lastFrame;
extern IPAddress lastPacketSource;
extern unsigned long lastPacketTime;

extern int16_t encoderPos; // Counts up or down depending which way the encoder is turned

void ICACHE_RAM_ATTR readEncoder();
void ICACHE_RAM_ATTR encBtnChange();
String IPToString(IPAddress ip);

extern uint8_t encstate;
extern bool CLKstate;
extern bool DTstate;

extern "C"
{
#include "user_interface.h"
	extern struct rst_info resetInfo;
	 #include <wpa2_enterprise.h>
}

#define FIRMWARE_VERSION "v3.0.1"
#define ART_FIRM_VERSION 0x0200 // Firmware given over Artnet (2 bytes)

// #define ESP_01              // Un comment for ESP_01 board settings
// #define NO_RESET            // Un comment to disable the reset button

// Wemos boards use 4M (3M SPIFFS) compiler option

#define ARTNET_OEM 0x0123		// Artnet OEM Code
#define ESTA_MAN 0x08DD			// ESTA Manufacturer Code
#define ESTA_DEV 0xEE000000 // RDM Device ID (used with Man Code to make 48bit UID)

#define DMX_DIR_A 15
#define DMX_DIR_B 16
#define DMX_TX_A 1
#define DMX_TX_B 2

#define STATUS_LED_PIN 0
#define STATUS_LED_MODE_WS2812
// #define STATUS_LED_MODE_APA106
#define STATUS_LED_A 0 // Physical wiring order for status LEDs
#define STATUS_LED_B 1
#define STATUS_LED_S 2

#define WS2812_ALLOW_INT_SINGLE false
#define WS2812_ALLOW_INT_DOUBLE false

// #define NO_RESET // Comment to enable the reset button

#ifndef NO_RESET
#define SETTINGS_RESET 14
extern bool factoryResetSelection;
extern bool startedFactoryReset;
#endif

// Definitions for status leds  xxBBRRGG
#define BLACK 0x00000000
#define WHITE 0x00FFFFFF
#define RED 0x0000FF00
#define GREEN 0x000000FF
#define BLUE 0x00FF0000
#define CYAN 0x00FF00FF
#define PINK 0x0066FF22
#define MAGENTA 0x00FFFF00
#define YELLOW 0x0000FFFF
#define ORANGE 0x0000FF33
#define STATUS_DIM 0x0F

extern uint8_t portA[5], portB[5];
extern uint8_t MAC_array[6];
extern uint8_t dmxInSeqID;
extern uint8_t statusLedData[9];
extern uint32_t statusTimer;

extern esp8266ArtNetRDM artRDM;
extern ESP8266WebServer webServer;
extern DynamicJsonBuffer jsonBuffer;
extern ws2812Driver pixDriver;
extern File fsUploadFile;
extern bool statusLedsDim;
extern bool statusLedsOff;

extern pixPatterns pixFXA;
extern pixPatterns pixFXB;

enum WiFiState_e{
	WIFISTATE_DEFAULT,
	WIFISTATE_STARTUP,
	WIFISTATE_CONNECTED,
	WIFISTATE_TIMEOUT
};

extern WiFiState_e WifiState;

/*const char PROGMEM mainPage[] = "<!DOCTYPE html><meta content='text/html; charset=utf-8' http-equiv=Content-Type /><title>DMXCube Config</title><meta content='Matthew Tong - http://github.com/mtongnz/' name=DC.creator /><meta content='Dominik Hörich - http://github.com/tonyxforce/' name=DC.creator /><meta content=en name=DC.language /><meta content='width=device-width,initial-scale=1' name=viewport /><link href=style.css rel=stylesheet /><div id=page><div class=inner><div class=mast><div class=title><br><h1>DMXCube</h1></div><ul class=nav><li class=first><a href='javascript: menuClick(1)'>Device Status</a><li><a href='javascript: menuClick(2)'>WiFi</a><li><a href='javascript: menuClick(3)'>IP settings</a><li><a href='javascript: menuClick(4)'>Port A</a>"
#ifndef ONE_PORT
																"<li><a href='javascript: menuClick(5)'>Port B</a>"
#endif
																"<li><a href='javascript: menuClick(7)'>Firmware</a><li class=last><a href='javascript: reboot()'>Reboot</a></ul><div class=author><i>Design by</i> Matthew Tong and Dominik Hörich</div></div><div class='main section'><div class=hide name=error><h2>Error</h2><p class=center>There was an error communicating with the device. Refresh the page and try again.</div><div class=show name=sections><h2>Fetching Data</h2><p class=center>Fetching data from device. If this message is still here in 15 seconds, try refreshing the page or clicking the menu option again.</div><div class=hide name=sections><h2>Device Status</h2><p class=left>MAC Address:<p class=right name=macAddress><p class=left>Wifi Status:<p class=right name=wifiStatus><p class=left>IP Address:<p class=right name=ipAddressT><p class=left>Subnet Address:<p class=right name=subAddressT><p class=left>Port A:<p class=right name=portAStatus>"
#ifndef ONE_PORT
																"<p class=left>Port B:<p class=right name=portBStatus>"
#endif
																"<p class=left>Firmware:<p class=right name=firmwareStatus></div><div class=hide name=sections><input name=save type=button value='Save Changes'/><h2>WiFi Settings</h2><p class=left>MAC Address:<p class=right name=macAddress><p class=spacer><p class=left>Wifi SSID:<p class=right><input type=text name=wifiSSID /><p class=left>Password:<p class=right><input type=text name=wifiPass /><p class=spacer><p class=left>Hotspot SSID:<p class=right><input type='text' name='hotspotSSID' /><p class=left>Password:<p class=right><input type=text name=hotspotPass /><p class=left>Connection timeout:<p class=right><input name=hotspotDelay type=number min=0 max=180 class=number /> (seconds)<p class=spacer><p class=left>Stand Alone:<p class=right><input name=standAloneEnable type=checkbox value=true /><p class=right>In normal mode, the hotspot will start after <i>delay</i> seconds if the main WiFi won't connect. If no users connect, the device will reset and attempt the main WiFi again. This feature is purely for changing settings and ArtNet data is ignored.<p class=right>Stand alone mode disables the primary WiFi connection and allows ArtNet data to be received via the hotspot connection.</div><div class=hide name=sections><input name=save type=button value='Save Changes'/><h2>IP settings</h2><p class=left>Enable DHCP:<p class=right><input name=dhcpEnable type=checkbox value=true /><p class=left>IP Address:<p class=right><input name=ipAddress type=number min=0 max=255 class=number /> <input name=ipAddress type=number min=0 max=255 class=number /> <input name=ipAddress type=number min=0 max=255 class=number /> <input name=ipAddress type=number min=0 max=255 class=number /><p class=left>Subnet Address:<p class=right><input name=subAddress type=number min=0 max=255 class=number /> <input name=subAddress type=number min=0 max=255 class=number /> <input name=subAddress type=number min=0 max=255 class=number /> <input name=subAddress type=number min=0 max=255 class=number /><p class=left>Gateway Address:<p class=right><input name=gwAddress type=number min=0 max=255 class=number /> <input name=gwAddress type=number min=0 max=255 class=number /> <input name=gwAddress type=number min=0 max=255 class=number /> <input name=gwAddress type=number min=0 max=255 class=number /><p class=left>Broadcast Address:<p class=right name=bcAddress><p class=left>Enable ArduinoOTA:<p class=right><input name=allowOTA type=checkbox value=false /><p class=left>Enable Automatic display refresh:<p class=right><input name=autoRefresh type=checkbox value=false /><p class=center>These settings only affect the main WiFi connection. The hotspot will always have DHCP enabled and an IP of <b>2.0.0.1</b></div><div class=hide name=sections><input name=save type=button value='Save Changes'/><h2>Port A Settings</h2><p class=left>Port Type:<p class=right><select class=select name=portAmode><option value=0>DMX Output<option value=1>DMX Output with RDM<option value=2>DMX Input<option value=3>LED Pixels - WS2812</select><p class=left>Protocol:<p class=right><select class=select name=portAprot><option value=0>Artnet v4<option value=1>Artnet v4 with sACN DMX</select><p class=left>Merge Mode:<p class=right><select class=select name=portAmerge><option value=0>Merge LTP<option value=1>Merge HTP</select><p class=left>Net:<p class=right><input name=portAnet type=number min=0 max=127 class=number /><p class=left>Subnet:<p class=right><input name=portAsub type=number min=0 max=15 class=number /><p class=left>Universe:<p class=right><input type=number min=0 max=15 name=portAuni class=number /><span name=portApix> <input type=number min=0 max=15 name=portAuni class=number /> <input type=number min=0 max=15 name=portAuni class=number /> <input type=number min=0 max=15 name=portAuni class=number /></span></p><p class=left>sACN Universe:<p class=right><input type=number min=0 max=63999 name=portAsACNuni class=number /> <input type=number min=1 max=63999 name=portAsACNuni class=number /> <input type=number min=1 max=63999 name=portAsACNuni class=number /> <input type=number min=1 max=63999 name=portAsACNuni class=number /></p><span name=DmxInBcAddrA><p class=left>Broadcast Address:<p class=right><input type=number min=0 max=255 name=dmxInBroadcast class=number /> <input type=number min=0 max=255 name=dmxInBroadcast class=number /> <input type=number min=0 max=255 name=dmxInBroadcast class=number /> <input type=number min=0 max=255 name=dmxInBroadcast class=number /></p></span><span name=portApix><p class=left>Number of Pixels:<p class=right><input type=number min=0 max=680 name=portAnumPix class=number /> 680 max - 170 per universe</p><p class=left>Mode:</p><p class=right><select class=select name=portApixMode><option value=0>Pixel Mapping</option><option value=1>12 Channel FX</option></select><p class=left>Start Channel:</p><p class=right><input type=number name=portApixFXstart class=number min=1 max=501 /> FX modes only</p></span></div><div class=hide name=sections><input name=save type=button value='Save Changes'/><h2>Port B Settings</h2><p class=left>Port Type:<p class=right><select class=select name=portBmode><option value=0>DMX Output<option value=1>DMX Output with RDM<option value=3>LED Pixels - WS2812</select><p class=left>Protocol:<p class=right><select class=select name=portBprot><option value=0>Artnet v4<option value=1>Artnet v4 with sACN DMX</select><p class=left>Merge Mode:<p class=right><select class=select name=portBmerge><option value=0>Merge LTP<option value=1>Merge HTP</select><p class=left>Net:<p class=right><input name=portBnet type=number min=0 max=127 class=number /><p class=left>Subnet:<p class=right><input name=portBsub type=number min=0 max=15 class=number /><p class=left>Universe:<p class=right><input type=number min=0 max=15 name=portBuni class=number /><span name=portBpix> <input type=number min=0 max=15 name=portBuni class=number /> <input type=number min=0 max=15 name=portBuni class=number /> <input type=number min=0 max=15 name=portBuni class=number /></span></p><p class=left>sACN Universe:<p class=right><input type=number min=1 max=63999 name=portBsACNuni class=number /> <input type=number min=1 max=63999 name=portBsACNuni class=number /> <input type=number min=1 max=63999 name=portBsACNuni class=number /> <input type=number min=1 max=63999 name=portBsACNuni class=number /></p><span name=portBpix><p class=left>Number of Pixels:<p class=right><input type=number min=0 max=680 name=portBnumPix class=number /> 680 max - 170 per universe</p><p class=left>Mode:</p><p class=right><select class=select name=portBpixMode><option value=0>Pixel Mapping</option><option value=1>12 Channel FX</option></select><p class=left>Start Channel:</p><p class=right><input type=number name=portBpixFXstart class=number min=1 max=501 /> FX modes only</p></span></div><div class=hide name=sections><h2>Stored Scenes</h2><p class=center>Not yet implemented</div><div class=hide name=sections><form action=/update enctype=multipart/form-data method=POST id=firmForm><h2>Update Firmware</h2><p class=left>Firmware:<p class=right name=firmwareStatus><p class=right><input name=update type=file id=update><label for=update><svg height=17 viewBox='0 0 20 17' width=20 xmlns=http://www.w3.org/2000/svg><path d='M10 0l-5.2 4.9h3.3v5.1h3.8v-5.1h3.3l-5.2-4.9zm9.3 11.5l-3.2-2.1h-2l3.4 2.6h-3.5c-.1 0-.2.1-.2.1l-.8 2.3h-6l-.8-2.2c-.1-.1-.1-.2-.2-.2h-3.6l3.4-2.6h-2l-3.2 2.1c-.4.3-.7 1-.6 1.5l.6 3.1c.1.5.7.9 1.2.9h16.3c.6 0 1.1-.4 1.3-.9l.6-3.1c.1-.5-.2-1.2-.7-1.5z'/></svg> <span>Choose Firmware</span></label><p class=right id=uploadMsg></p><p class=right><input type=button class=submit value='Upload Now' id=fUp></div></div><div class=footer><p>Coding and hardware © 2016-2017 <a href=http://github.com/mtongnz/ >Matthew Tong</a>.<p>Released under <a href=http://www.gnu.org/licenses/ >GNU General Public License V3</a>.</div></div></div><script src='/script.js'></script></body></html>";
*/
//const char PROGMEM scriptJs[] = "var cl=0;var num=0;var err=0;var o=document.getElementsByName('sections');var s=document.getElementsByName('save');for (var i=0, e; e=s[i++];)e.addEventListener( 'click', function(){sendData();}); var u=document.getElementById('fUp');var um=document.getElementById('uploadMsg');var fileSelect=document.getElementById('update');u.addEventListener('click',function(){uploadPrep()});function uploadPrep(){if(fileSelect.files.length===0) return;u.disabled=!0;u.value='Preparing Device…';var x=new XMLHttpRequest();x.onreadystatechange=function(){if(x.readyState==XMLHttpRequest.DONE){try{var r=JSON.parse(x.response)}catch(e){var r={success:0,doUpdate:1}} if(r.success==1&&r.doUpdate==1){uploadWait()}else{um.value='<b>Update failed!</b>';u.value='Upload Now';u.disabled=!1}}};x.open('POST','/ajax',!0);x.setRequestHeader('Content-Type','application/json');x.send('{\"doUpdate\":1,\"success\":1}')} function uploadWait(){setTimeout(function(){var z=new XMLHttpRequest();z.onreadystatechange=function(){if(z.readyState==XMLHttpRequest.DONE){try{var r=JSON.parse(z.response)}catch(e){var r={success:0}} console.log('r=' + r.success); if(r.success==1){upload()}else{uploadWait()}}};z.open('POST','/ajax',!0);z.setRequestHeader('Content-Type','application/json');z.send('{\"doUpdate\":2,\"success\":1}')},1000)} var upload=function(){u.value='Uploading… 0%';var data=new FormData();data.append('update',fileSelect.files[0]);var x=new XMLHttpRequest();x.onreadystatechange=function(){if(x.readyState==4){try{var r=JSON.parse(x.response)}catch(e){var r={success:0,message:'No response from device.'}} console.log(r.success+': '+r.message);if(r.success==1){u.value=r.message;setTimeout(function(){location.reload()},15000)}else{um.value='<b>Update failed!</b> '+r.message;u.value='Upload Now';u.disabled=!1}}};x.upload.addEventListener('progress',function(e){var p=Math.ceil((e.loaded/e.total)*100);console.log('Progress: '+p+'%');if(p<100) u.value='Uploading... '+p+'%';else u.value='Upload complete. Processing…'},!1);x.open('POST','/upload',!0);x.send(data)}; function reboot() { if (err == 1) return; var r = confirm('Are you sure you want to reboot?'); if (r != true) return; o[cl].className = 'hide'; o[0].childNodes[0].innerHTML = 'Rebooting'; o[0].childNodes[1].innerHTML = 'Please wait while the device reboots. This page will refresh shortly unless you changed the IP or Wifi.'; o[0].className = 'show'; err = 0; var x = new XMLHttpRequest(); x.onreadystatechange = function(){ if(x.readyState == 4){ try { var r = JSON.parse(x.response); } catch (e){ var r = {success: 0, message: 'Unknown error: [' + x.responseText + ']'}; } if (r.success != 1) { o[0].childNodes[0].innerHTML = 'Reboot Failed'; o[0].childNodes[1].innerHTML = 'Something went wrong and the device didn\\'t respond correctly. Please try again.'; } setTimeout(function() { location.reload(); }, 5000); } }; x.open('POST', '/ajax', true); x.setRequestHeader('Content-Type', 'application/json'); x.send('{\"reboot\":1,\"success\":1}'); } function sendData(){var d={'page':num};for (var i=0, e; e=o[cl].getElementsByTagName('INPUT')[i++];){var k=e.getAttribute('name');var v=e.value;if (k in d) continue; if (k=='ipAddress' || k=='subAddress' || k=='gwAddress' || k=='portAuni' || k=='portBuni' || k=='portAsACNuni' || k=='portBsACNuni' || k=='dmxInBroadcast'){var c=[v];for (var z=1; z < 4; z++){c.push(o[cl].getElementsByTagName('INPUT')[i++].value);}d[k]=c; continue;}if (e.type==='text')d[k]=v;if (e.type==='number'){if (v=='')v=0;d[k]=v;}if (e.type==='checkbox'){if (e.checked)d[k]=1;else d[k]=0;}}for (var i=0, e; e=o[cl].getElementsByTagName('SELECT')[i++];){d[e.getAttribute('name')]=e.options[e.selectedIndex].value;}d['success']=1;var x=new XMLHttpRequest();x.onreadystatechange=function(){handleAJAX(x);};x.open('POST', '/ajax');x.setRequestHeader('Content-Type', 'application/json');x.send(JSON.stringify(d));console.log(d);} function menuClick(n){if (err==1) return; num=n; setTimeout(function(){if (cl==num || err==1) return; o[cl].className='hide'; o[0].className='show'; cl=0;}, 100); var x=new XMLHttpRequest(); x.onreadystatechange=function(){handleAJAX(x);}; x.open('POST', '/ajax'); x.setRequestHeader('Content-Type', 'application/json'); x.send(JSON.stringify({\"page\":num,\"success\":1}));}function handleAJAX(x){if (x.readyState==XMLHttpRequest.DONE ){if (x.status==200){var response=JSON.parse(x.responseText);console.log(response);if (!response.hasOwnProperty('success')){err=1; o[cl].className='hide'; document.getElementsByName('error')[0].className='show';return;}if (response['success'] !=1){err=1; o[cl].className='hide';document.getElementsByName('error')[0].getElementsByTagName('P')[0].innerHTML=response['message']; document.getElementsByName('error')[0].className='show';return;}if (response.hasOwnProperty('message')) { for (var i = 0, e; e = s[i++];) { e.value = response['message']; e.className = 'showMessage' } setTimeout(function() { for (var i = 0, e; e = s[i++];) { e.value = 'Save Changes'; e.className = '' } }, 5000); } o[cl].className='hide'; o[num].className='show'; cl=num; for (var key in response){if (response.hasOwnProperty(key)){var a=document.getElementsByName(key); if (key=='ipAddress' || key=='subAddress'){var b=document.getElementsByName(key + 'T'); for (var z=0; z < 4; z++){a[z].value=response[key][z]; if (z==0) b[0].innerHTML=''; else b[0].innerHTML=b[0].innerHTML + ' . '; b[0].innerHTML=b[0].innerHTML + response[key][z];}continue;}else if (key=='bcAddress'){for (var z=0; z < 4; z++){if (z==0) a[0].innerHTML=''; else a[0].innerHTML=a[0].innerHTML + ' . '; a[0].innerHTML=a[0].innerHTML + response[key][z];}continue;} else if (key=='gwAddress' || key=='dmxInBroadcast' || key=='portAuni' || key=='portBuni' || key=='portAsACNuni' || key=='portBsACNuni'){for(var z=0;z<4;z++){a[z].value = response[key][z];}continue}if(key=='portAmode'){var b = document.getElementsByName('portApix');var c = document.getElementsByName('DmxInBcAddrA');if(response[key] == 3) {b[0].style.display = '';b[1].style.display = '';} else {b[0].style.display = 'none';b[1].style.display = 'none';}if (response[key] == 2){c[0].style.display = '';}else{c[0].style.display = 'none';}} else if (key == 'portBmode') {var b = document.getElementsByName('portBpix');if(response[key] == 3) {b[0].style.display = '';b[1].style.display = '';} else {b[0].style.display = 'none';b[1].style.display = 'none';}}for (var z=0; z < a.length; z++){switch (a[z].nodeName){case 'P': case 'DIV': a[z].innerHTML=response[key]; break; case 'INPUT': if (a[z].type=='checkbox'){if (response[key]==1) a[z].checked=true; else a[z].checked=false;}else a[z].value=response[key]; break; case 'SELECT': for (var y=0; y < a[z].options.length; y++){if (a[z].options[y].value==response[key]){a[z].options.selectedIndex=y; break;}}break;}}}}}else{err=1; o[cl].className='hide'; document.getElementsByName('error')[0].className='show';}}}var update=document.getElementById('update');var label=update.nextElementSibling;var labelVal=label.innerHTML;update.addEventListener( 'change', function( e ){var fileName=e.target.value.split( '\\\\' ).pop(); if( fileName ) label.querySelector( 'span' ).innerHTML=fileName; else label.innerHTML=labelVal; update.blur();}); document.onkeydown=function(e){if(cl < 2 || cl > 6)return; var e = e||window.event; if (e.keyCode == 13)sendData();}; menuClick(1);";
#include "media/index.html.h"
#include "media/style.css.h"
#include "media/script.js.h"
#include "media/favicon.svg.h"
const char PROGMEM cssUploadPage[] = "<html><head><title>DMXCube CSS Upload</title></head><body>Select and upload your CSS file.  This will overwrite any previous uploads but you can restore the default below.<br /><br /><form method='POST' action='/style_upload' enctype='multipart/form-data'><input type='file' name='css' accept='style.css'><input type='submit' value='Upload New CSS'></form><br /><a href='/style_delete'>Restore default CSS</a></body></html>";

//const char PROGMEM css[] = ".author,.title,ul.nav a{text-align:center}.author i,.show,.title h1,ul.nav a{display:block}input,ul.nav a:hover{background-color:#DADADA}a,abbr,acronym,address,applet,b,big,blockquote,body,caption,center,cite,code,dd,del,dfn,div,dl,dt,em,fieldset,font,form,h1,h2,h3,h4,h5,h6,html,i,iframe,img,ins,kbd,label,legend,li,object,ol,p,pre,q,s,samp,small,span,strike,strong,sub,sup,table,tbody,td,tfoot,th,thead,tr,tt,u,ul,var{margin:0;padding:0;border:0;outline:0;font-size:100%;vertical-align:baseline;background:0 0}.main h2,li.last{border-bottom:1px solid #888583}body{line-height:1;background:#E4E4E4;color:#292929;color:rgba(0,0,0,.82);font:400 100% Cambria,Georgia,serif;-moz-text-shadow:0 1px 0 rgba(255,255,255,.8);}ol,ul{list-style:none}a{color:#890101;text-decoration:none;-moz-transition:.2s color linear;-webkit-transition:.2s color linear;transition:.2s color linear}a:hover{color:#DF3030}#page{padding:0}.inner{margin:0 auto;width:91%}.amp{font-family:Baskerville,Garamond,Palatino,'Palatino Linotype','Hoefler Text','Times New Roman',serif;font-style:italic;font-weight:400}.mast{float:left;width:31.875%}.title{font:semi 700 16px/1.2 Baskerville,Garamond,Palatino,'Palatino Linotype','Hoefler Text','Times New Roman',serif;padding-top:0}.title h1{font:700 20px/1.2 'Book Antiqua','Palatino Linotype',Georgia,serif;padding-top:0}.author{font:400 100% Cambria,Georgia,serif}.author i{font:400 12px Baskerville,Garamond,Palatino,'Palatino Linotype','Hoefler Text','Times New Roman',serif;letter-spacing:.05em;padding-top:.7em}.footer,.main{float:right;width:65.9375%}ul.nav{margin:1em auto 0;width:11em}ul.nav a{font:700 14px/1.2 'Book Antiqua','Palatino Linotype',Georgia,serif;letter-spacing:.1em;padding:.7em .5em;margin-bottom:0;text-transform:uppercase}input[type=button],input[type=button]:focus{background-color:#E4E4E4;color:#890101}li{border-top:1px solid #888583}.hide{display:none}.main h2{font-size:1.4em;text-align:left;margin:0 0 1em;padding:0 0 .3em}.main{position:relative}p.left{clear:left;float:left;width:20%;min-width:120px;max-width:300px;margin:0 0 .6em;padding:0;text-align:right}p.right,select{min-width:200px}p.right{overflow:auto;margin:0 0 .6em .4em;padding-left:.6em;text-align:left}p.center,p.spacer{padding:0;display:block}.footer,p.center{text-align:center}p.center{float:left;clear:both;margin:3em 0 3em 15%;width:70%}p.spacer{float:left;clear:both;margin:0;width:100%;height:20px}input{margin:0;border:0;color:#890101;outline:0;font:400 100% Cambria,Georgia,serif}input[type=text]{width:70%;min-width:200px;padding:0 5px}input[type=number]{min-width:50px;width:50px}input:focus{background-color:silver;color:#000}input[type=checkbox]{-webkit-appearance:none;background-color:#fafafa;border:1px solid #cacece;box-shadow:0 1px 2px rgba(0,0,0,.05),inset 0 -15px 10px -12px rgba(0,0,0,.05);padding:9px;border-radius:5px;display:inline-block;position:relative}input[type=checkbox]:active,input[type=checkbox]:checked:active{box-shadow:0 1px 2px rgba(0,0,0,.05),inset 0 1px 3px rgba(0,0,0,.1)}input[type=checkbox]:checked{background-color:#fafafa;border:1px solid #adb8c0;box-shadow:0 1px 2px rgba(0,0,0,.05),inset 0 -15px 10px -12px rgba(0,0,0,.05),inset 15px 10px -12px rgba(255,255,255,.1);color:#99a1a7}input[type=checkbox]:checked:after{content:'\\2714';font-size:14px;position:absolute;top:0;left:3px;color:#890101}input[type=button],input[type=file]+label{font:700 16px/1.2 'Book Antiqua','Palatino Linotype',Georgia,serif;margin:17px 0 0}input[type=button]{position:absolute;right:0;display:block;border:1px solid #adb8c0;float:right;border-radius:12px;padding:5px 20px 2px 23px;-webkit-transition-duration:.3s;transition-duration:.3s}input[type=button]:hover{background-color:#909090;color:#fff;padding:5px 62px 2px 65px}input.submit{float:left;position: relative}input.showMessage,input.showMessage:focus,input.showMessage:hover{background-color:#6F0;color:#000;padding:5px 62px 2px 65px}input[type=file]{width:.1px;height:.1px;opacity:0;overflow:hidden;position:absolute;z-index:-1}input[type=file]+label{float:left;clear:both;cursor:pointer;border:1px solid #adb8c0;border-radius:12px;padding:5px 20px 2px 23px;display:inline-block;background-color:#E4E4E4;color:#890101;overflow:hidden;-webkit-transition-duration:.3s;transition-duration:.3s}input[type=file]+label:hover,input[type=file]:focus+label{background-color:#909090;color:#fff;padding:5px 40px 2px 43px}input[type=file]+label svg{width:1em;height:1em;vertical-align:middle;fill:currentColor;margin-top:-.25em;margin-right:.25em}select{margin:0;border:0;background-color:#DADADA;color:#890101;outline:0;font:400 100% Cambria,Georgia,serif;width:50%;padding:0 5px}.footer{border-top:1px solid #888583;display:block;font-size:12px;margin-top:20px;padding:.7em 0 20px}.footer p{margin-bottom:.5em}@media (min-width:600px){.inner{min-width:600px}}@media (max-width:600px){.inner,.page{min-width:300px;width:100%;overflow-x:hidden}.footer,.main,.mast{float:left;width:100%}.mast{border-top:1px solid #888583;border-bottom:1px solid #888583}.main{margin-top:4px;width:98%}ul.nav{margin:0 auto;width:100%}ul.nav li{float:left;min-width:100px;width:33%}ul.nav a{font:12px Helvetica,Arial,sans-serif;letter-spacing:0;padding:.8em}.title,.title h1{padding:0;text-align:center}ul.nav a:focus,ul.nav a:hover{background-position:0 100%}.author{display:none}.title{border-bottom:1px solid #888583;width:100%;display:block;font:400 15px Baskerville,Garamond,Palatino,'Palatino Linotype','Hoefler Text','Times New Roman',serif}.title h1{font:600 15px Baskerville,Garamond,Palatino,'Palatino Linotype','Hoefler Text','Times New Roman',serif;display:inline}p.left,p.right{clear:both;float:left;margin-right:1em}li,li.first,li.last{border:0}p.left{width:100%;text-align:left;margin-left:.4em;font-weight:600}p.right{margin-left:1em;width:100%}p.center{margin:1em 0;width:100%}p.spacer{display:none}input[type=text],select{width:85%;}@media (min-width:1300px){.page{width:1300px}}";
const char PROGMEM typeHTML[] = "text/html";
const char PROGMEM typeCSS[] = "text/css";

extern char wifiStatus[60];
extern bool isHotspot;
extern uint32_t nextNodeReport;
extern char nodeError[ARTNET_NODE_REPORT_LENGTH];
extern bool nodeErrorShowing;
extern uint32_t nodeErrorTimeout;
extern bool pixDone;
extern bool newDmxIn;
extern bool doReboot;
extern byte *dataIn;

void dmxHandle(uint8_t group, uint8_t port, uint16_t numChans, bool syncEnabled);
void syncHandle();
void ipHandle();
void addressHandle();
void rdmHandle(uint8_t group, uint8_t port, rdm_data *c);
void rdmReceivedA(rdm_data *c);
void sendTodA();

#ifndef ONE_PORT
void rdmReceivedB(rdm_data *c);
void sendTodB();
#endif
void todRequest(uint8_t group, uint8_t port);
void todFlush(uint8_t group, uint8_t port);
void dmxIn(uint16_t num);
void doStatusLedOutput();
void setStatusLed(uint8_t num, uint32_t col);

#endif