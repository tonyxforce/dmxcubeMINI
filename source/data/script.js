/** @format */

console.log("JSfile V0.1.0");

var cl = 0;

var num = 0;
var err = 0;
var sections = document.getElementsByName("sections");
var save = document.getElementsByName("save");
save.forEach((e) =>
    e.addEventListener("click", function () {
        sendData();
    })
);

var nav = document.getElementsByClassName("nav")[0];

for(let i = 0; i<nav.childElementCount; i++){
	document.getElementById(`menuClick${i}`).addEventListener("click", _=>menuClick(i));
}

var firmupload = document.getElementById("fUp");
var um = document.getElementById("uploadMsg");
var fileSelect = document.getElementById("update");
firmupload.addEventListener("click", function () {
    uploadPrep();
});
function uploadPrep() {
    if (fileSelect.files.length === 0) return;
    firmupload.disabled = !0;
    firmupload.value = "Preparing Device…";
    var x = new XMLHttpRequest();
    x.onreadystatechange = function () {
        if (x.readyState == XMLHttpRequest.DONE) {
            try {
                var r = JSON.parse(x.response);
            } catch (e) {
                var r = { success: 0, doUpdate: 1 };
            }
            if (r.success == 1 && r.doUpdate == 1) {
                uploadWait();
            } else {
                um.value = "<b>Update failed!</b>";
                firmupload.value = "Upload Now";
                firmupload.disabled = !1;
            }
        }
    };
    x.open("POST", "/ajax", 1);
    x.setRequestHeader("Content-Type", "application/json");
    x.send('{"doUpdate":1,"success":1}');
}
function uploadWait() {
    setTimeout(function () {
        var request = new XMLHttpRequest();
        request.onreadystatechange = function () {
            if (request.readyState == XMLHttpRequest.DONE) {
                try {
                    var resp = JSON.parse(request.response);
                } catch (e) {
                    var resp = { success: 0 };
                }
                console.log("r=" + resp.success);
                if (resp.success == 1) {
                    upload();
                } else {
                    uploadWait();
                }
            }
        };
        request.open("POST", "/ajax", !0);
        request.setRequestHeader("Content-Type", "application/json");
        request.send('{"doUpdate":2,"success":1}');
    }, 1000);
}
var upload = function () {
    firmupload.value = "Uploading… 0%";
    var data = new FormData();
    data.append("update", fileSelect.files[0]);
    var x = new XMLHttpRequest();
    x.onreadystatechange = function () {
        if (x.readyState == 4) {
            try {
                var resp = JSON.parse(x.response);
            } catch (e) {
                var resp = { success: 0, message: "No response from device." };
            }
            console.log(resp.success + ": " + resp.message);
            if (resp.success == 1) {
                firmupload.value = resp.message;
                setTimeout(function () {
                    location.reload();
                }, 15000);
            } else {
                um.value = "<b>Update failed!</b> " + resp.message;
                firmupload.value = "Upload Now";
                firmupload.disabled = !1;
            }
        }
    };
    x.upload.addEventListener(
        "progress",
        function (e) {
            var p = Math.ceil((e.loaded / e.total) * 100);
            console.log("Progress: " + p + "%");
            if (p < 100) firmupload.value = "Uploading... " + p + "%";
            else firmupload.value = "Upload complete. Processing…";
        },
        !1
    );
    x.open("POST", "/upload", 1);
    x.send(data);
};
function reboot() {
    if (err == 1) return console.log("Can't reboot with an error");

    if (!confirm("Are you sure you want to reboot?")) return;

    sections[cl].className = "hide";
    sections[0].childNodes[0].innerHTML = "Rebooting";
    sections[0].childNodes[1].innerHTML =
        "Please wait while the device reboots. This page will refresh shortly unless you changed the IP or Wifi.";
    sections[0].className = "show";
    err = 0;
    var x = new XMLHttpRequest();
    x.onreadystatechange = function () {
        if (x.readyState == 4) {
            try {
                var r = JSON.parse(x.response);
            } catch (e) {
                var r = {
                    success: 0,
                    message: "Unknown error: [" + x.responseText + "]",
                };
            }
            if (r.success != 1) {
                sections[0].childNodes[0].innerHTML = "Reboot Failed";
                sections[0].childNodes[1].innerHTML =
                    "Something went wrong and the device didn't respond correctly. Please try again.";
            }
            setTimeout(function () {
                location.reload();
            }, 5000);
        }
    };
    x.open("POST", "/ajax", true);
    x.setRequestHeader("Content-Type", "application/json");
    x.send('{"reboot":1,"success":1}');
}
function sendData() {
    var d = { page: num };
    for (
        var i = 0, e;
        (e = sections[cl].getElementsByTagName("INPUT")[i++]);

    ) {
        var k = e.getAttribute("name");
        var v = e.value;
        if (k in d) continue;
        if (
            k == "ipAddress" ||
            k == "subAddress" ||
            k == "gwAddress" ||
            k == "portAuni" ||
            k == "portBuni" ||
            k == "portAsACNuni" ||
            k == "portBsACNuni" ||
            k == "dmxInBroadcast"
        ) {
            var c = [v];
            console.log("k", k);
            console.log("v", v);
            for (var z = 1; z < 4; z++) {
                c.push(sections[cl].getElementsByTagName("INPUT")[i++].value);
            }
            d[k] = c;
            continue;
        }
        if (e.type === "text") d[k] = v;
        if (e.type === "number") {
            if (v == "") v = 0;
            d[k] = v;
        }
        if (e.type === "checkbox") {
            if (e.checked) d[k] = 1;
            else d[k] = 0;
        }
    }
    for (
        var i = 0, e;
        (e = sections[cl].getElementsByTagName("SELECT")[i++]);

    ) {
        d[e.getAttribute("name")] = e.options[e.selectedIndex].value;
    }
    d["success"] = 1;
    var x = new XMLHttpRequest();
    x.onreadystatechange = function () {
        handleAJAX(x);
    };
    x.open("POST", "/ajax");
    x.setRequestHeader("Content-Type", "application/json");
    x.send(JSON.stringify(d));
    console.log(d);
}
function menuClick(n) {
    if (err == 1) return;
    num = n;
    setTimeout(function () {
        if (cl == num || err == 1) return;
        sections[cl].className = "hide";
        sections[0].className = "show";
        cl = 0;
    }, 100);
    var x = new XMLHttpRequest();
    x.onreadystatechange = function () {
        handleAJAX(x);
    };
    x.open("POST", "/ajax");
    x.setRequestHeader("Content-Type", "application/json");
    x.send(JSON.stringify({ page: num, success: 1 }));
}
function handleAJAX(request) {
    if (request.readyState == XMLHttpRequest.DONE) {
        if (request.status == 200) {
            var response = JSON.parse(request.responseText);
            console.log(response);
            if (!response.hasOwnProperty("success")) {
                err = 1;
                sections[cl].className = "hide";
                document.getElementsByName("error")[0].className = "show";
                return;
            }
            if (response["success"] != 1) {
                err = 1;
                sections[cl].className = "hide";
                document
                    .getElementsByName("error")[0]
                    .getElementsByTagName("P")[0].innerHTML =
                    response["message"];
                document.getElementsByName("error")[0].className = "show";
                return;
            }
            if (response.hasOwnProperty("message")) {
                for (var i = 0, e; (e = save[i++]); ) {
                    e.value = response["message"];
                    e.className = "showMessage";
                }
                setTimeout(function () {
                    for (var i = 0, e; (e = save[i++]); ) {
                        e.value = "Save Changes";
                        e.className = "";
                    }
                }, 5000);
            }
            sections[cl].className = "hide";
            sections[num].className = "show";
            cl = num;
            for (var key in response) {
                if (response.hasOwnProperty(key)) {
                    var elements = document.getElementsByName(key);
                    if (key == "ipAddress" || key == "subAddress") {
                        var portApix = document.getElementsByName(key + "T");
                        for (let octet = 0; octet < 4; octet++) {
                            elements[octet].value = response[key][octet];
                            if (octet == 0) portApix[0].innerHTML = "";
                            else
                                portApix[0].innerHTML =
                                    portApix[0].innerHTML + " . ";
                            portApix[0].innerHTML =
                                portApix[0].innerHTML + response[key][octet];
                        }
                        continue;
                    } else if (key == "bcAddress") {
                        for (let octet = 0; octet < 4; octet++) {
                            if (octet == 0) elements[0].innerHTML = "";
                            else
                                elements[0].innerHTML =
                                    elements[0].innerHTML + " . ";
                            elements[0].innerHTML =
                                elements[0].innerHTML + response[key][octet];
                        }
                        continue;
                    } else if (
                        key == "gwAddress" ||
                        key == "dmxInBroadcast" ||
                        key == "portAuni" ||
                        key == "portBuni" ||
                        key == "portAsACNuni" ||
                        key == "portBsACNuni"
                    ) {
                        for (var z = 0; z < elements.length; z++) {
                            elements[z].value = response[key][z];
                        }
                        continue;
                    };
										["A", "B"].forEach((port)=>{
											if(key == `port${port}mode`){
												var portPix = document.getElementsByName(`port${port}pix`);
                        var DmxInBcAddr =
                            document.getElementsByName(`DmxInBcAddr${port}`);
                        if (response[key] == 3) {
                            for (let z = 0; z < portPix.length; z++) {
                                portPix[z].style.display = "";
                            }
                        } else {
                            for (let z = 0; z < portPix.length; z++) {
                                portPix[z].style.display = "none";
                            }
                        }
                        if (response[key] == 2) {
                            DmxInBcAddr[0].style.display = "";
                        } else {
                            DmxInBcAddr[0].style.display = "none";
                        }
											}
										});
                    /* if (key == "portAmode") {
                        var portApix = document.getElementsByName("portApix");
                        var DmxInBcAddrA =
                            document.getElementsByName("DmxInBcAddrA");
                        if (response[key] == 3) {
                            for (let z = 0; z < portApix.length; z++) {
                                portApix[z].style.display = "";
                            }
                        } else {
                            for (let z = 0; z < portApix.length; z++) {
                                portApix[z].style.display = "none";
                            }
                        }
                        if (response[key] == 2) {
                            DmxInBcAddrA[0].style.display = "";
                        } else {
                            DmxInBcAddrA[0].style.display = "none";
                        }
                    } else if (key == "portBmode") {
                        var portApix = document.getElementsByName("portBpix");
                        if (response[key] == 3) {
                            portApix[0].style.display = "";
                            portApix[1].style.display = "";
                        } else {
                            portApix[0].style.display = "none";
                            portApix[1].style.display = "none";
                        }
                    } */
                    for (let i = 0; i < elements.length; i++) {
                        switch (elements[i].nodeName) {
                            case "P":
                            case "DIV":
                                elements[i].innerHTML = response[key];
                                break;
                            case "INPUT":
                                if (elements[i].type == "checkbox") {
                                    if (response[key] == 1)
                                        elements[i].checked = true;
                                    else elements[i].checked = false;
                                } else elements[i].value = response[key];
                                break;
                            case "SELECT":
                                for (
                                    let j = 0;
                                    j < elements[i].options.length;
                                    j++
                                ) {
                                    if (
                                        elements[i].options[j].value ==
                                        response[key]
                                    ) {
                                        elements[i].options.selectedIndex = j;
                                        break;
                                    }
                                }
                                break;
                        }
                    }
                }
            }
        } else {
            err = 1;
            sections[cl].className = "hide";
            document.getElementsByName("error")[0].className = "show";
        }
    }
}

var update = document.getElementById("update");
var label = update.nextElementSibling;
var labelVal = label.innerHTML;
update.addEventListener("change", function (e) {
    var fileName = e.target.value.split("\\").pop();
    if (fileName) label.querySelector("span").innerHTML = fileName;
    else label.innerHTML = labelVal;
    update.blur();
});
menuClick(1);
