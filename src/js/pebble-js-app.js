var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationError(err) {
  console.log("Error requesting location!");
}

function getRadiationStatus() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

function locationSuccess(pos) {
  // Construct URL
  var url = "http://api.bqm.jp/v1/radiations?lat=" +
      pos.coords.latitude + "&lng=" + pos.coords.longitude + "&type=nearest";

  // Send request
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with radiation info
      var json = JSON.parse(responseText);
      console.log(json.results[0].distance);
      // Assemble dictionary using our keys
      var dictionary = {
        "KEY_NAME": json.results[0].location_name,
        "KEY_DISTANCE": '' + json.results[0].distance.toFixed(2),
        "KEY_DATE": json.results[0].data_date,
        "KEY_CURRENT": json.results[0].current_dosage,
        "KEY_AVERAGE": json.results[0].average_dosage,
      };

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Radiation info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending radiation info to Pebble!");
        }
      );
    }
  );
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready',
  function(e) {
    console.log("PebbleKit JS ready!");

    // Get the initial radiation
    getRadiationStatus();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("AppMessage received!");
    getRadiationStatus();
  }
);
