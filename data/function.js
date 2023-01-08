var xmlHttp = createXmlHttpObject();
function createXmlHttpObject() {
    if (window.XMLHttpRequest) {
        xmlHttp = new XMLHttpRequest();
    } else {
        xmlHttp = new ActiveXObject('Microsoft.XMLHTTP');
    }
    return xmlHttp;
}

function load() {
    if (xmlHttp.readyState == 0 || xmlHttp.readyState == 4) {
        xmlHttp.open('PUT', '/config.json', true);
        xmlHttp.send(null);
        xmlHttp.onload = function () {
            jsonResponse = JSON.parse(xmlHttp.responseText);
            loadBlock();
        }
    }
}

function loadBlock() {
    newData = JSON.parse(xmlHttp.responseText);
    data = document.getElementsByTagName('body')[0].innerHTML;
    var newString;
    for (var key in newData) {
        newString = data.replace(new RegExp('{{' + key + '}}', 'g'), newData[key]);
        data = newString;
    }
    document.getElementsByTagName('body')[0].innerHTML = newString;
    setFirmvareValue('version', 'firmware');
    handleServerResponse();
}

function getValue(id) {
    var value = document.getElementById(id).value;
    return value;
}

function sendRequest(submit, server) {
    request = new XMLHttpRequest();
    request.open("GET", server, true);
    request.send();
}

function saveSetting(submit) {
    server = "/setting?deviceName=" + getValue('deviceName')
        + "&espnowNetName=" + getValue('espnowNetName');
    sendRequest(submit, server);
    alert("Please restart device for changes apply.");
}

function restart(submit) {
    server = "/restart";
    sendRequest(submit, server);
}

function setFirmvareValue(id, value) {
    document.getElementById(id).innerHTML = document.getElementById(value).value;
}