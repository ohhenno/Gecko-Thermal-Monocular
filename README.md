# TAC-640 (Upgraded Gecko)

The TAC-640 is an open-source thermal monocular featuring a P6 thermal core, digital zoom, and video recording capability.

The hardware platform is built around:
- Hdaniee P6 thermal camera  
- 0.39" micro OLED display  
- NP18 lens  

---

## 📷 Device Overview

<img width="300" height="400" src="https://github.com/user-attachments/assets/21ff8a33-6ccc-4398-af90-d936a225ea72" />
<img width="300" height="400" src="https://github.com/user-attachments/assets/87d28182-701c-433f-8b85-3effc9024394" />
<img width="300" height="400" src="https://github.com/user-attachments/assets/e37db837-70ba-407e-aedf-c7d0c0c3d4d8" />
<img width="300" height="400" src="https://github.com/user-attachments/assets/2c80c966-5193-4ee8-84d6-4e7e728f41e4" />

---

## ⚡ Power-On Behavior

Turn the potentiometer fully clockwise to power on the device.

- **System initialization time:** ~6 seconds  

### Default Image Settings
- Brightness: 70  
- Contrast: 64  
- Enhancement: 80  
- Denoise: 40  
- Zoom: 1.0×  
- Palette: White Hot  
- Automatic calibration (NUC): every 60 seconds  

---

## 🎮 Controls Overview

### Buttons
- **Zoom Button** — front-most button  
- **Multi Button** — center button  
- **Record Button** — rear button  

### Analog Control
- **Rotary Knob (Potentiometer)** — controls brightness and contrast  

---

## 🔍 Zoom & OLED Brightness

### Zoom Control
- **Single Press:**  
  Cycles zoom levels  
  `1.0× → 2.0× → 4.0× → repeat`

### OLED Brightness
- **Hold Zoom (~1 second):**  
  Increase brightness (continues stepping every ~1s)

- **Double Tap Zoom:**  
  Decrease brightness  

---

## 🎨 Palette Control

- **Multi Button (Short Press):**  
  Cycles through palettes:
  - White Hot  
  - Black Hot  
  - Rainbow  
  - Green Hot  
  - Red Hot  
  - Iron Red  

---

## 🎛️ Image Adjustment

- **Knob (Normal):**  
  Adjust brightness  

- **Hold Multi + Turn Knob:**  
  Adjust contrast  

---

## 🔴 Recording Feedback

- **Record Button:**  
  Toggles external DVR recording  

### Visual Feedback (Palette Flash)
- Start recording → **3 slow flashes**  
- Stop recording → **5 fast flashes**  

---

## 🧊 Calibration (NUC)

### Manual Calibration
- Hold **Multi button for 3 seconds**
  - Image briefly freezes  
  - Sensor recalibrates  
  - Fixed-pattern noise reduced  

### Automatic Calibration
- Runs every **60 seconds**  

---

## ⚡ Quick Reference

| Action              | Result                         |
|--------------------|--------------------------------|
| Zoom press         | Change zoom                    |
| Zoom hold          | Increase OLED brightness       |
| Zoom double tap    | Decrease OLED brightness       |
| Multi press        | Cycle palettes                 |
| Multi hold         | Trigger NUC calibration        |
| Knob               | Adjust brightness              |
| Multi + knob       | Adjust contrast                |
| Record press       | Toggle recording               |

---

## 🧠 System Diagram / Reference

<img width="1429" height="1128" src="https://github.com/user-attachments/assets/b0eabb19-590a-4c99-99f2-658ee891185a" />
