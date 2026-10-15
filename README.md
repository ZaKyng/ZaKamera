# ZaKamera 📸

**ZaKamera** is a homemade **mechanical single-pixel camera** built with an Arduino, 2 servo motors, and a light sensor. 

Instead of taking a photo all at once like a smartphone, ZaKamera uses two servos to physically sweep a narrow tube (housing a single photoresistor) across a 2D grid. It reads light intensity one spot at a time to slowly build up a digital image.

---

## How It Works

1. **Scanning:** 
   * Two servo motors aim the sensor tube step-by-step across an X/Y grid (60 x 60 grid).
   * At each point, the light sensor records a raw analog brightness value (`0` = dark, `1023` = bright).
   * A potentiometer knob on the side lets you trim light sensitivity (acting like a manual ISO dial) depending on lighting.
   * The grid of values is saved to a file on an microSD card.

2. **Visualization:**
   * The SD card is transferred to a Windows PC.
   * A custom **Python desktop app** reads the raw text grid, maps the `0–1023` intensity values into visual pixels, and lets you apply basic color palettes before exporting to a `.PNG` image.

---

## Photos & Documentation

Because this was a personal experimental project, full schematics were not documented during development. 
Check the `photos/` directory to see the mechanical build, wiring setup, physical assembly and desktop app.

---

## Current Project State & Limitations

* **Project Paused:** Development was paused in August 2026 due to school priorities.
* **Arduino Onboard Processing:** Originally intended to let the user set his own resolution, delays, FOV, but limited space made me use AI assisted sript, that worked in the end
* **Color Processing:** The Python GUI currently supports 6 basic fixed color presets + 1 randomized color palette for creative rendering. Full custom RGB mapping remains incomplete.
* **Models:** Models for 3D printing was