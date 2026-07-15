#include <Arduino.h>
#include <SPI.h>
#include <SD.h>


const int chipSelect = 6;
bool connectedSD = false;
const char* dirName = "/zakamera1";
String nextFileName = ""; // Will be set dynamically during setup

void setUpSD() {
  if (!SD.begin(chipSelect)) {
    connectedSD = false;
    return;
  }
  connectedSD = true;

  if (!SD.exists(dirName)) {
    if (!SD.mkdir(dirName)) {
      return;
    }
  }

  nextFileName = getNextFileName(dirName);
}

// Function to scan the directory and calculate the next index
String getNextFileName(const char* dirPath) {
  File dir = SD.open(dirPath);
  int maxIndex = -1;

  if (!dir) {
    Serial.println("Error opening directory for scanning.");
    return "";
  }

  while (true) {
    File entry = dir.openNextFile();

    if (!entry) {
      break;
    }

    if (entry.isDirectory()) {
      entry.close();
      continue;
    }
    String name = entry.name();
    
    if (!name.startsWith("ZKP")) {
      entry.close();
      continue;
    }

    String numPart = name.substring(3, 6); 
    
    int fileIndex = numPart.toInt();
    if (fileIndex > maxIndex) {
      maxIndex = fileIndex; // Track the highest number found
    }
    entry.close();
  }
  dir.close();

  // Increment the highest found index by 1 (or make it 0 if folder is empty)
  int nextIndex = maxIndex + 1;

  // Format the index back into a 3-digit padded string (e.g. "/zakamera1/zkp004.txt")
  char buffer[40];
  snprintf(buffer, sizeof(buffer), "%s/zkp%03d.txt", dirPath, nextIndex);
  
  return String(buffer);
}

void saveSD(String photoDir) {
  if (!connectedSD || photoDir == "") {
    return;
  }

  File dataFile = SD.open(photoDir, FILE_WRITE);
  if (dataFile) {
    dataFile.print("Your data goes here"); // Added placeholder text to print
    dataFile.close();
  }
}