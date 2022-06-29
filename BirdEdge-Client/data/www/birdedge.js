function loadStatus() {
    var req = new XMLHttpRequest();
    req.responseType = 'json';
    req.open('GET', 'status.json', true);
    req.onload = function () {
        for (const key in req.response) {
            document.getElementById(key).innerHTML = req.response[key];
        }
    };
    req.send(null);
}

// schedule function to run every five seconds
loadStatus();
const interval = setInterval(loadStatus, 5000);
