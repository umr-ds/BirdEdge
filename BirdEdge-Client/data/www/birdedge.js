function loadStatus() {
    var req = new XMLHttpRequest();
    req.responseType = 'json';
    req.open('GET', 'status.json', true);
    req.onload = function () {
        for (const key in req.response) {
            element = document.getElementById(key);
            if (element.nodeName == "INPUT") {
                // write value field, if no value is present
                if (element.value == "") {
                    element.value = req.response[key];
                }
            } else {
                // write inner HTML else
                element.innerHTML = req.response[key];
            }
        }
    };
    req.send(null);
}

// schedule function to run every five seconds
loadStatus();
const interval = setInterval(loadStatus, 5000);
