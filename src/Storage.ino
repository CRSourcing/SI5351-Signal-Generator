//##########################################################################################################################//


// Functionality to work with an external SD card

void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {

  File root = fs.open(dirname);
  if (!root) {
    tft.println("\nFailed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    tft.println("\nNot a directory");
    return;
  }

  tft.print(F("FILES ON SDCard:\n"));

  File file = root.openNextFile();
  while (file) {
    tft.printf("%s %u bytes\n", file.name(), file.size());
    file = root.openNextFile();
  }
}

//##########################################################################################################################//











void readSDCard(bool close) {  // 0 = read and close,  1 = leave open

  tRel();

  int ctr = 0;

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 20);
  delay(20);

  digitalWrite(SD_CS, OUTPUT);

  while (!SD.begin(SD_CS, spiSD, 2000000)) {

    ctr++;
    tft.print(F("Mounting SD card...\n"));
    if (ctr == 10) {
      tft.setTextColor(TFT_RED);
      tft.print(F("-SD Card Mount failed!\n"));
      tft.setTextColor(TFT_GREEN);
      delay(1000);
      return;
    }
    delay(100);
  }

  tft.print(F("\nSD Card Mounted\n"));
  uint64_t cardSize = SD.cardSize() / (1048567);
  tft.printf("\nSD Card Size: %lluMB\n", cardSize);

  listDir(SD, "/", 0);

  digitalWrite(SD_CS, INPUT_PULLUP);


  if (close)
    return;


  while (true) {
    uint16_t z = tft.getTouchRawZ();
    if (z > 300)  // touch, encoder moved or pressed
      break;
  }

  SD.end();
  preferences.putBool("fB", true);
  ESP.restart();
}
//##########################################################################################################################//

void listLittleFSFiles() {
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 15);

  while (file) {
    tft.print(file.name());
    tft.print(F(" - "));
    tft.print(file.size());
    tft.println(" bytes");
    file = root.openNextFile();
  }

  uint64_t totalBytes = LittleFS.totalBytes();
  uint64_t usedBytes = LittleFS.usedBytes();
  uint64_t remainingBytes = totalBytes - usedBytes;

  tft.println("----------------");
  tft.print(F("Total space: "));
  tft.print(totalBytes);
  tft.println(" bytes");
  tft.print(F("Used space: "));
  tft.print(usedBytes);
  tft.println(" bytes");
  tft.print(F("Remaining space: "));
  tft.print(remainingBytes);
  tft.println(" bytes");
}

//##########################################################################################################################//


// WIFI upload/download files to LittleFS or SDCard
WebServer server(80);

bool useLittleFS = true;
fs::FS* storage;
File uploadFile;

// ------------------ Handlers ------------------

void handleRoot() {
  size_t total = useLittleFS ? LittleFS.totalBytes() : 0;
  size_t used = useLittleFS ? LittleFS.usedBytes() : 0;
  size_t free = total - used;

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>SigGen upload/download utility</title>
  <style>
    .progress-container { margin: 5px 0; }
    .progress-bar {
      width: 0%; height: 20px; background: blue; text-align: center; color: white;
    }
  </style>
</head>
<body>
  <h2>Receiver upload/download utility. Copies files to/from LittleFS or SD Card.</h2>
)rawliteral";

  html += "<p>Backend: " + String(useLittleFS ? "LittleFS" : "SD") + "</p>";
  html += "<form action='/switch' method='get'>";
  html += "<button type='submit'>Switch to " + String(useLittleFS ? "SDCard" : "LittleFS") + "</button>";
  html += "</form>";

  if (useLittleFS) {
    html += "<p>Total: " + String(total) + " bytes</p>";
    html += "<p>Used: " + String(used) + " bytes</p>";
    html += "<p>Free: " + String(free) + " bytes</p>";
  }

  html += R"rawliteral(
  <hr>
  <h3>Upload Files:</h3>
  <input type="file" id="files" multiple>
  <button onclick="uploadFiles()">Upload</button>
  <div id="status"></div>
  <hr>
  <h3>Stored Files:</h3>
  <ul id='fileList'>
)rawliteral";

  // File listing
  File root = storage->open("/");
  if (root) {
    File file = root.openNextFile();
    while (file) {
      String name = file.name();
      if (name.startsWith("/")) name = name.substring(1);
      size_t size = file.size();
      html += "<li>" + name + " (" + String(size) + " bytes) ";
      html += "<a href='/download?file=" + name + "'>Download</a> ";
      html += "<a href='/delete?file=" + name + "'>Delete</a></li>";
      file = root.openNextFile();
    }
    root.close();
  }

  html += R"rawliteral(
  </ul>
<script>
function refreshFileList() {
  fetch("/")
    .then(res => res.text())
    .then(html => {
      const parser = new DOMParser();
      const doc = parser.parseFromString(html, "text/html");
      const newList = doc.querySelector("#fileList");
      document.querySelector("#fileList").innerHTML = newList.innerHTML;
    });
}

function uploadFiles() {
  const files = document.getElementById('files').files;
  const statusDiv = document.getElementById('status');
  statusDiv.innerHTML = '';

  for (let i = 0; i < files.length; i++) {
    const file = files[i];
    const container = document.createElement('div');
    container.className = 'progress-container';
    const label = document.createElement('div');
    label.textContent = 'Uploading ' + file.name;
    const bar = document.createElement('div');
    bar.className = 'progress-bar';
    container.appendChild(label);
    container.appendChild(bar);
    statusDiv.appendChild(container);

    const xhr = new XMLHttpRequest();
    xhr.open("POST", "/upload", true);
    xhr.upload.onprogress = function(e) {
      if (e.lengthComputable) {
        const percent = (e.loaded / e.total) * 100;
        bar.style.width = percent + "%";
        bar.textContent = Math.round(percent) + "%";
      }
    };
    xhr.onload = function() {
      if (xhr.status == 200) {
        bar.style.background = "blue";
        label.textContent = "Uploaded " + file.name;
        refreshFileList();
      } else {
        bar.style.background = "red";
        label.textContent = "Error uploading " + file.name;
      }
    };
    const formData = new FormData();
    formData.append("file", file);
    xhr.send(formData);
  }
}
</script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}
//##########################################################################################################################//

void handleUpload() {
  HTTPUpload& upload = server.upload();
  String filename = upload.filename;
  if (!filename.startsWith("/")) filename = "/" + filename;

  if (upload.status == UPLOAD_FILE_START) {
    uploadFile = storage->open(filename, FILE_WRITE);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
      server.send(200, "text/plain", "Upload successful: " + filename);
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    if (uploadFile) uploadFile.close();
    server.send(400, "text/plain", "Upload aborted");
  }
}
//##########################################################################################################################//
void handleDownload() {
  if (!server.hasArg("file")) {
    server.send(400, "text/plain", "Missing file parameter");
    return;
  }
  String filename = server.arg("file");
  if (!filename.startsWith("/")) filename = "/" + filename;

  File f = storage->open(filename, FILE_READ);
  if (!f) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  // Tell browser to download with original filename
  server.sendHeader("Content-Disposition", "attachment; filename=" + filename.substring(1));
  server.streamFile(f, "application/octet-stream");
  f.close();
}

//##########################################################################################################################//
void handleDelete() {
  if (!server.hasArg("file")) {
    server.send(400, "text/plain", "Missing file parameter");
    return;
  }
  String filename = server.arg("file");
  if (!filename.startsWith("/")) filename = "/" + filename;
  if (storage->remove(filename)) {
    server.sendHeader("Location", "/");
    server.send(303);
  } else {
    server.send(500, "text/plain", "Failed to delete file");
  }
}
//##########################################################################################################################//
void handleSwitch() {
  useLittleFS = !useLittleFS;
  if (useLittleFS) {
    LittleFS.begin(true);
    storage = &LittleFS;
    listLittleFSFiles();
  } else {
    readSDCard(true);
    storage = &SD;
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

//##########################################################################################################################//

void startUploader() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);

  if (useLittleFS) {
    if (!LittleFS.begin(true)) {
      tft.println("LittleFS mount failed");
      delay(1000);
      return;
    }
    storage = &LittleFS;
  } else {


    readSDCard(true);
    storage = &SD;
  }

  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    tft.print(F("."));
  }
  tft.println("\n\nUpload/download files via WiFi.\n\nWiFi is now connected.\n\nOpen IP in browser: " + WiFi.localIP().toString());
  tft.println("\n\n Touch when finished.");

  server.on("/", HTTP_GET, handleRoot);
  server.on(
    "/upload", HTTP_POST, []() {}, handleUpload);
  server.on("/download", HTTP_GET, handleDownload);
  server.on("/delete", HTTP_GET, handleDelete);
  server.on("/switch", HTTP_GET, handleSwitch);
  server.begin();
}


//##########################################################################################################################//
void runUpLoader() {
  while (true) {
    server.handleClient();
    uint16_t z = tft.getTouchRawZ();
    if (z > 300)  // touch, encoder moved or pressed
      ESP.restart();
  }
}

//##########################################################################################################################//

// WAV file header structure
typedef struct {
  char chunkID[4];         // "RIFF"
  uint32_t chunkSize;      // File size - 8
  char format[4];          // "WAVE"
  char subchunk1ID[4];     // "fmt "
  uint32_t subchunk1Size;  // 16 for PCM
  uint16_t audioFormat;    // 1 = PCM
  uint16_t numChannels;    // 1 = mono
  uint32_t sampleRate;
  uint32_t byteRate;       // sampleRate * numChannels * bitsPerSample/8
  uint16_t blockAlign;     // numChannels * bitsPerSample/8
  uint16_t bitsPerSample;  // 8
  char subchunk2ID[4];     // "data"
  uint32_t subchunk2Size;  // data size
} WavHeader_8bit;


//##########################################################################################################################//


// basic .wav player

void playWavFile() {

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(1, 1);
  tft.print("Modulates audio from file play.wav on LittleFS.\nMax. file size 1.5MB.\nFormat: 8 bit, SR 8000, mono.\nEndless loop. Touch to leave.");

  LittleFS.begin(false);

  File f = LittleFS.open("/play.wav", "r");


  if (!f) {
    tft.println("\n\nFailed to open /play.wav");
    delay(1000);
    return;
  }

  WavHeader_8bit header;



  if (f.read((uint8_t*)&header, sizeof(header)) != sizeof(header)) {
    Serial.println(F("Failed to read WAV header"));
    f.close();
    return;
  }

  if (header.audioFormat != 1 || header.numChannels != 1 || header.bitsPerSample != 8) {
    Serial.println(F("Unsupported WAV format"));
    f.close();
    return;
  }



  const size_t dataStart = sizeof(header);
  uint32_t bytesAvailable = 9999;
  bool read = false;



  while (true) {

    bufferPlaying = false;
    playIndex = 0;
    bytesAvailable = 9999;
    read = false;

    f.seek(dataStart, SeekSet);

    while (bytesAvailable > 0) {
      if (!read) {
        bytesAvailable = f.read(playBuffer, 512);

        if (bytesAvailable <= 0) {
          break;
        }

        playIndex = 0;
        bufferPlaying = true;  // ISR plays the buffer
        read = true;
      }

      while (bufferPlaying) {
        uint16_t z = tft.getTouchRawZ();
        if (z > 300)  // touch, encoder moved or pressed
          return;
      }

      read = false;
    }
  }
}

//##########################################################################################################################//
