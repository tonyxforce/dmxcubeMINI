/** @format */

console.log("JSfile V0.1.0");

var currentSectionIndex = 0;
var currentURL = new URL(document.URL);
var ajaxEndpoint = `/ajax`;
if (currentURL.port != 80 && currentURL.port != "") {
    ajaxEndpoint = `http://localhost:8080/http://cubeminiw.local/ajax`;
}

var checkUpdate = document.getElementById("checkUpdate");
var targetSectionIndex = 0;
var err = 0;
var sections = document.getElementsByName("sections");
var save = document.getElementsByName("save");
var errComm = document.getElementById("errComm");
var errFetch = document.getElementById("errFetch");
save.forEach((e) =>
    e.addEventListener("click", function () {
        sendData();
    })
);

var isWPA2E = document.getElementById("wpa2Enterprise");
var wpa2e = document.getElementsByClassName("wpa2e");
var checkWPAE = () => {
    for (let a = 0; a < wpa2e.length; a++)
        wpa2e[a].style.display = isWPA2E.checked ? "" : "none";
    return isWPA2E.checked;
};
checkWPAE();

function bodyLoad() {
    isWPA2E.addEventListener("click", checkWPAE);
}

var nav = document.getElementsByClassName("nav")[0];

for (let i = 1; i < nav.childElementCount; i++) {
    document
        .getElementById(`menuClick${i}`)
        .addEventListener("click", (_) => menuClick(i));
}

var firmupload = document.getElementById("fUp");
var um = document.getElementById("uploadMsg");
var fileSelect = document.getElementById("update");
function reboot() {
    if (err == 1) return console.log("Can't reboot with an error");

    if (!confirm("Are you sure you want to reboot?")) return;

    hide(currentSectionIndex);
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
    var data = { page: targetSectionIndex };
    for (
        var index = 0, element;
        (element =
            sections[currentSectionIndex].getElementsByTagName("INPUT")[
                index++
            ]);

    ) {
        var name = element.getAttribute("name");
        var v = element.value;
        if (name in data) continue;
        if (
            name == "ipAddress" ||
            name == "subAddress" ||
            name == "gwAddress" ||
            name == "portAuni" ||
            name == "portBuni" ||
            name == "portAsACNuni" ||
            name == "portBsACNuni" ||
            name == "dmxInBroadcast"
        ) {
            var c = [v];
            console.log("k", name);
            console.log("v", v);
            for (var z = 1; z < 4; z++) {
                c.push(
                    sections[currentSectionIndex].getElementsByTagName("INPUT")[
                        index++
                    ].value
                );
            }
            data[name] = c;
            continue;
        }
        if (element.type === "text") data[name] = v;
        if (element.type === "number") {
            if (v == "") v = 0;
            data[name] = v;
        }
        if (element.type === "checkbox") {
            if (element.checked) data[name] = 1;
            else data[name] = 0;
        }
    }
    for (
        var index = 0, element;
        (element =
            sections[currentSectionIndex].getElementsByTagName("SELECT")[
                index++
            ]);

    ) {
        data[element.getAttribute("name")] =
            element.options[element.selectedIndex].value;
    }
    data["success"] = 1;
    var x = new XMLHttpRequest();
    x.onreadystatechange = function () {
        handleAJAX(x);
    };

    x.open("POST", ajaxEndpoint);
    x.setRequestHeader("Content-Type", "application/json");
    x.send(JSON.stringify(data));
    console.log(data);
}
function menuClick(newSectionIndex) {
    if (err == 1) return console.error("errored");
    setTimeout(function () {
        if (currentSectionIndex == newSectionIndex || err == 1) return;
        hide(currentSectionIndex);
        show(0);
        currentSectionIndex = 0;
    }, 100);
    targetSectionIndex = newSectionIndex;
    var x = new XMLHttpRequest();
    x.onreadystatechange = function () {
        handleAJAX(x);
    };

    x.open("POST", ajaxEndpoint);
    x.setRequestHeader("Content-Type", "application/json");
    x.send(JSON.stringify({ page: newSectionIndex, success: 1 }));
}
function handleAJAX(request) {
    if (request.readyState == XMLHttpRequest.DONE) {
        if (request.status == 200) {
            var response = JSON.parse(request.responseText);
            console.log(response);
            if (!response.hasOwnProperty("success")) {
                console.error("no success");
                err = 1;
                hide(currentSectionIndex);
                showErr(errComm);
                return;
            }
            if (response["success"] != 1) {
                console.error("unsuccessful");
                err = 1;
                hide(currentSectionIndex);
                showErr(errComm);
                document
                    .getElementsByClassName("error")[0]
                    .getElementsByTagName("P")[0].innerHTML =
                    response["message"];
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
                }, 1000);
            }
            hide(currentSectionIndex);
            show(targetSectionIndex);

            currentSectionIndex = targetSectionIndex;
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
                                    elements[0].innerHTML + ".";
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
                        for (var octet = 0; octet < elements.length; octet++) {
                            elements[octet].value = response[key][octet];
                        }
                        continue;
                    }
                    ["A", "B"].forEach((port) => {
                        if (key == `port${port}mode`) {
                            var portPix = document.getElementsByName(
                                `port${port}pix`
                            );
                            var DmxInBcAddr = document.getElementsByClassName(
                                `DmxInBcAddr${port}`
                            );
                            if (response[key] == 3) {
                                for (
                                    let octet = 0;
                                    octet < portPix.length;
                                    octet++
                                ) {
                                    portPix[octet].style.display = "";
                                }
                            } else {
                                for (
                                    let octet = 0;
                                    octet < portPix.length;
                                    octet++
                                ) {
                                    portPix[octet].style.display = "none";
                                }
                            }
                            if (port == "A")
                                if (response[key] == 2) {
                                    for (
                                        let a = 0;
                                        a < DmxInBcAddr.length;
                                        a++
                                    ) {
                                        DmxInBcAddr[a].style.display = "";
                                    }
                                } else {
                                    for (
                                        let a = 0;
                                        a < DmxInBcAddr.length;
                                        a++
                                    ) {
                                        DmxInBcAddr[a].style.display = "none";
                                    }
                                }
                        }
                    });
                    for (let i = 0; i < elements.length; i++) {
                        switch (elements[i].nodeName) {
                            case "SPAN":
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
                switch (currentSectionIndex) {
                    case 6:
                        if (response.updateAvail) {
                        }
                        break;
                }
            }
        } else {
            console.error("not 200");
            err = 1;
            show(currentSectionIndex);
            showErr(errComm);
        }
    }
}

function show(id) {
    sections[id].classList.remove("hide");
    sections[id].classList.add("show");
}
function hide(id) {
    sections[id].classList.add("hide");
    sections[id].classList.remove("show");
}
function showErr(errName) {
    var errors = document.getElementsByClassName("error");
    for (let i = 0; i < errors.length; i++) {
        error = errors[i];
        error.classList.add("hide");
        error.classList.remove("show");
    }
    errName.classList.add("show");
    errName.classList.remove("hide");
}
menuClick(1);
