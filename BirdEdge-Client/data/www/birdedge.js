function loadStatus() {
    var req = new XMLHttpRequest();
    req.open('GET', 'status.json', true);
    req.onreadystatechange = function () {
        if (req.readyState == 4 && req.status == "200") {
            document.getElementById("status").innerHTML = req.responseText;
        }
    };
    req.send(null);
}

// schedule function to run every five seconds
loadStatus();
const interval = setInterval(loadStatus, 5000);
