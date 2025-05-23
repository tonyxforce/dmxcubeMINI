/** @format */

var currentSectionIndex = 0;
var currentURL = new URL(document.URL);
var ajaxEndpoint = `/ajax`;
if (currentURL.port != 80 && currentURL.port != "") {
    ajaxEndpoint = `http://localhost:8080/http://cubeminiw.local/ajax`;
}

function downloadTxt(text) {
    var hiddenElement = document.createElement("a");

    hiddenElement.href = "data:attachment/text," + encodeURI(text);
    hiddenElement.target = "_blank";
    hiddenElement.download = "settings.dcs";
    hiddenElement.click();
}

var saveClicked = 0;
var updateBtn = document.getElementById("doUpdate");
var targetSectionIndex = 0;
var err = 0;
var sections = document.getElementsByName("sections");
var save = document.getElementsByName("save");
var errComm = document.getElementById("errComm");
var errFetch = document.getElementById("errFetch");
var checkUpdateBtn = document.getElementById("checkUpdate");
save.forEach((e) =>
    e.addEventListener("click", function () {
        sendData();
        saveClicked = true;
    })
);

var isWPA2E = document.getElementById("wpa2Enterprise");
var wpa2e = document.getElementsByClassName("wpa2e");
var checkWPAE = () => {
    for (let a = 0; a < wpa2e.length; a++)
        wpa2e[a].style.display = isWPA2E.checked ? "" : "none";
    return isWPA2E.checked;
};
function doUpdate() {
    if (
        !confirm(
            "Are you sure you want to start updating? WiFi and other settings may be reset to factory defauls"
        )
    )
        return;
    var x = new XMLHttpRequest();
    x.onreadystatechange = function () {};
    x.open("POST", ajaxEndpoint, true);
    x.setRequestHeader("Content-Type", "application/json");
    x.send('{"page":6,"success":1,"doUpdate":1}');
}
function checkUpdate() {
    checkUpdateBtn.value = "Checking for updates...";
    menuClick(6, { checkUpdate: 1 });
}

function bodyLoad() {
    isWPA2E.addEventListener("click", checkWPAE);
    checkWPAE();
    updateBtn.addEventListener("click", doUpdate);
    checkUpdateBtn.addEventListener("click", checkUpdate);
}

var nav = document.getElementsByClassName("nav")[0];

for (let i = 1; i < nav.childElementCount; i++) {
    document
        .getElementById(`menuClick${i}`)
        .addEventListener("click", (_) => menuClick(i));
}

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
    x.open("POST", ajaxEndpoint, true);
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
            ["ipAddress", "subAddress", "gwAddress", "dmxInBroadcast"].includes(
                name
            )
        ) {
            var c = [v];
            console.log("k", name);
            console.log("v", v);

            for (let i = 0; i < 3; i++)
                c.push(
                    sections[currentSectionIndex].getElementsByTagName("INPUT")[
                        index++
                    ].value
                );
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
function menuClick(newSectionIndex, extraReq) {
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
    var data = { page: newSectionIndex, success: 1 };
    data = Object.assign(data, extraReq);
    console.log("req", data);
    x.send(JSON.stringify(data));
}
function handleAJAX(request) {
    if (request.readyState == XMLHttpRequest.DONE) {
        if (request.status == 200) {
            var response = JSON.parse(request.responseText);
            console.log("res", response);
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

            if (saveClicked && response.hasOwnProperty("message")) {
                saveClicked = 0;
                for (let i = 0; i < save.length; i++) {
                    let e = save[i];
                    e.value = response["message"];
                    e.className = "showMessage";
                }
                setTimeout(function () {
                    for (let i = 0, e; (e = save[i++]); ) {
                        e.value = "Save Changes";
                        e.className = "";
                    }
                }, 1000);
            }
            hide(currentSectionIndex);
            show(targetSectionIndex);

            currentSectionIndex = targetSectionIndex;
            for (let key in response) {
                if (response.hasOwnProperty(key)) {
                    var elements = document.getElementsByName(key);
                    if (
                        [
                            "ipAddress",
                            "subAddress",
                            "gwAddress",
                            "dmxInBroadcast",
                        ].includes(key)
                    ) {
                        for (let i = 0; i < elements.length; i++) {
                            elements[i].value = response[key][i];
                        }
                    }else{

											["A", "B"].forEach((port) => {
												if (key == `port${port}mode`) {
                            var portPix = document.getElementsByClassName(
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
                                    portPix[octet].classList.add("show");
                                    portPix[octet].classList.remove("hide");
                                }
                            } else {
                                for (
                                    let octet = 0;
                                    octet < portPix.length;
                                    octet++
                                ) {
                                    portPix[octet].classList.remove("show");
                                    portPix[octet].classList.add("hide");
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
                        } else if (key == `port${port}prot`) {
                            var prot = response[key];
                            var sacn = document.getElementsByClassName(
                                `sacn${port}`
                            );
                            switch (prot) {
                                case 0:
                                    //artnet
                                    for (let a = 0; a < sacn.length; a++) {
                                        sacn[a].classList.add("hide");
                                        sacn[a].classList.remove("show");
                                    }
                                    break;
                                case 1:
                                    //artnet with sacn
                                    for (let a = 0; a < sacn.length; a++) {
                                        sacn[a].classList.add("show");
                                        sacn[a].classList.remove("hide");
                                    }
                                    break;
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
                    if (key == "deviceSettings") {
                        downloadTxt(response.deviceSettings);
                    }
                }
							}
                switch (currentSectionIndex) {
                    case 6:
                        checkUpdateBtn.value = "Check for updates";
                        var newFirm =
                            document.getElementsByClassName("newFirm");
                        if (response.updateAvail) {
                            for (let i = 0; i < newFirm.length; i++) {
                                newFirm[i].classList.remove("hide");
                                newFirm[i].classList.add("show");
                            }
                            document.getElementById("newFirmVer").innerText =
                                response.latestVer;
                        } else {
                            for (let i = 0; i < newFirm.length; i++) {
                                newFirm[i].classList.remove("show");
                                newFirm[i].classList.add("hide");
                            }
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
