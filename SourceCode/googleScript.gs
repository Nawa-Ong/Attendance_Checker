function doPost(e) {
  var sheet = SpreadsheetApp.openById('1vswoTEcI7ATnEq5YR8ZoMMFPJLkMfkUnwlexRScgDRY').getActiveSheet();
  var mappingSheet = SpreadsheetApp.openById('1vswoTEcI7ATnEq5YR8ZoMMFPJLkMfkUnwlexRScgDRY').getSheetByName('CardMapping');

  var cutoffTimeStr = mappingSheet.getRange("D1").getValue().replace(/"/g, "").trim();
  var cutoffTime = parseTime(cutoffTimeStr);

  var data = JSON.parse(e.postData.contents);
  var cardID = data.cardID;

  var name = lookupName(mappingSheet, cardID);
  var stdid = lookupID(mappingSheet, cardID);
  
  if (!name) {
    name = "Unknown (" + cardID + ")";
  }

  var timestamp = new Date();
  var currentTimeInMinutes = timestamp.getHours() * 60 + timestamp.getMinutes();
  var cutoffTimeInMinutes = cutoffTime.hours * 60 + cutoffTime.minutes;

  var status = (currentTimeInMinutes <= cutoffTimeInMinutes) ? "Present" : "Late";
  
  sheet.appendRow([timestamp, stdid, name, status]);

  return ContentService.createTextOutput('Data received and added to sheet.');
}

function parseTime(timeStr) {
  var parts = timeStr.split(":");
  return { hours: parseInt(parts[0], 10), minutes: parseInt(parts[1], 10) };
}

function lookupName(mappingSheet, cardID) {
  var dataRange = mappingSheet.getDataRange();
  var values = dataRange.getValues();
  for (var i = 0; i < values.length; i++) {
    if (values[i][0] == cardID) {
      return values[i][2];
    }
  }
  return null;
}

function lookupID(mappingSheet, cardID) {
  var dataRange = mappingSheet.getDataRange();
  var values = dataRange.getValues();
  for (var i = 0; i < values.length; i++) {
    if (values[i][0] == cardID) {
      return values[i][1];
    }
  }
  return null;
}
