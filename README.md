# Gecko

The Gecko is an open-source thermal monocular featuring the Mini2 256p thermal core, digital zoom, and video recording capability. Special thanks to LupasWorax Gremlin and [JacobOTW](https://github.com/Jacob-OTW/BCOTI/tree/V2_Beta?tab=readme-ov-file) .

The hardware platform is built around:
- HdanieeCamera Core
- ESP32 UART/GPIO Controls  
- 0.39" micro OLED display  
- NP18 lens  


---

## 🧩 Parts Checklist

| ✓ | Component              | Notes                          | Price (USD) | Link |
|--|------------------------|--------------------------------|-------------|------|
| ☐ | **Thermal Core (Mini2 256)** | Gecko Variant                  | $120 | https://shorturl.at/Mn3Rv |
| ☐ | **Thermal Core (P6 640)**   | TAC-640 Variant                | $420 | https://www.hdaniee.com/p6-uc-series.html |
| ☐ | **0.39" OLED Display**     | Main display + driver          | $110 | https://www.aliexpress.us/item/3256809119244467.html |
| ☐ | **NP18 Lens**              | Optics                        | $60 | https://www.aliexpress.us/item/3256808208343549.html |
| ☐ | **ESP32-C3**               | Main controller               | $1 | https://www.aliexpress.us/item/3256808855552297.html |
| ☐ | **5V Boost Converter**     | Steps battery → 5V            | $2 | https://www.amazon.com/dp/B0836J8LR4 |
| ☐ | **18350 Battery**          | Power source                  | $15 | https://amazon.com/dp/B07QDDS64F |
| ☐ | **Battery Contacts**       | Spring terminals              | $5 | https://amazon.com/dp/B0FSDNXKJY |
| ☐ | **Rotary Pot (20k)**       | Brightness / contrast control | $2 | https://www.aliexpress.us/item/3256803337433740.html |
| ☐ | **Buttons (x3)**           | Zoom / Mode / Record          | $2 | https://www.aliexpress.us/item/3256806708603327.html |
| ☐ | **Runcam DVR (Optional)**  | Recording                     | $30 | https://www.aliexpress.us/item/3256809185467125.html |
| ☐ | **MicroSD Extender**       | Required for DVR              | $10 | https://amazon.com/dp/B0D7YZ5PG3 |
| ☐ | **M2.5 x 6 Bolts**         | Mounting hardware             | $2 | https://amazon.com/dp/B0GCSRG1WD |
| ☐ | **Heat Set Inserts**       | Threaded inserts              | $10 | https://amazon.com/dp/B0D7YZ5PG3 |
| ☐ | **Silicone Wire**          | Internal wiring               | $6 | https://amazon.com/dp/B01KQ2JNLI |

---

### 💰 Estimated Build Cost

- **Gecko (Mini2 build):**  
  $335  

- **TAC-640 (P6 build):**  
  $635  

- **With DVR (adds DVR + MicroSD extender):**  
  $40  

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

<table>
<tr>
<td valign="top">

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

</td>
<td valign="top">

<pre>
        Button Diagram

        [ Camera Side ]
               ↑
           ┌─────────┐
           │   ◉     │   ZOOM (Front)
           │         │
           │   ◉     │   MODE / MULTI
           │         │
           │   ◉     │   RECORD (Rear)
           └─────────┘
               ↓
        [ Ocular Side ]
</pre>

</td>
</tr>
</table>

---

## 🧠 System Diagram / Reference

<img width="1419" height="1121" alt="image" src="https://github.com/user-attachments/assets/da7ef4d3-162a-4f77-a530-645bb60353bf" />

