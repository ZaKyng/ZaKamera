#include <SPI.h>
#include <SD.h>

const int chipSelect = 11;

void setup() {
  // Start communication with the computer (Serial Monitor)
  Serial.begin(9600);
  
  Serial.print("Starting SD card...");

  // Try to start the SD card module
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card failed or not present!");
    return; // Stop the program here if it fails
  }
  Serial.println("SD card connected.");

  // --- PART 1: WRITE DATA ---
  
  // Open the file 'data.txt'. FILE_WRITE allows us to save new data.
  File dataFile = SD.open("data.txt", FILE_WRITE);

  if (dataFile) {
    Serial.println("Writing text to data.txt...");
    dataFile.println("This message is now saved on the card.");
    
    // IMPORTANT: You MUST close the file to save the data.
    dataFile.close();
    Serial.println("Writing complete.");
  } else {
    Serial.println("ERROR: Could not open data.txt for writing.");
  }

  // --- PART 2: READ DATA ---
  
  Serial.println("--- Reading saved file ---");
  
  // Open the same file again, but just for reading
  File readFile = SD.open("data.txt");

  if (readFile) {
    // Read the file byte by byte until the end
    while (readFile.available()) {
      Serial.write(readFile.read()); // Print the data to the Serial Monitor
    }
    // Close the file after you finish reading
    readFile.close();
    Serial.println("\n--- Reading complete ---");
  } else {
    Serial.println("ERROR: Could not open data.txt for reading.");
  }
}

void loop() {
  // Nothing to do here, the program runs only once in setup()
}