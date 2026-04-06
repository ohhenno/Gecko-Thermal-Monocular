# Gecko-Thermal-Monocular
The Gecko is an opensource thermal monocular. It features a 256 Mini2 Thermal Core (V2 is P6 640), digital zoom, and video recording. The hardware is loosely based around the Lupus Gremlin >> https://shorturl.at/PMXlo , Hdaniee thermal camera (Mini2 256, soon to be P6), 0.39" micro OLED, and the NP18 lens. The software and controls are based around the JacobOTW BCOTI >> https://github.com/Jacob-OTW/BCOTI , but used in a standalone monocular form factor.

<img width="300" height="400" alt="image" src="https://github.com/user-attachments/assets/21ff8a33-6ccc-4398-af90-d936a225ea72" />
<img width="300" height="400" alt="image" src="https://github.com/user-attachments/assets/87d28182-701c-433f-8b85-3effc9024394" />
<img width="300" height="400" alt="image" src="https://github.com/user-attachments/assets/e37db837-70ba-407e-aedf-c7d0c0c3d4d8" />
<img width="300" height="400" alt="image" src="https://github.com/user-attachments/assets/2c80c966-5193-4ee8-84d6-4e7e728f41e4" />


---

## 🧩 Parts Checklist

| ✓ | Component              | Notes                          | Link |
|--|------------------------|--------------------------------|------|
| ☐ | **Thermal Core (Mini2 256)** | Gecko Variant                  | https://shorturl.at/Mn3Rv |
| ☐ | **Thermal Core (P6 640)**   | TAC-640 Variant | https://www.hdaniee.com/p6-uc-series.html |
| ☐ | **0.39" OLED Display**     | Main display + Driver Board                 | https://www.aliexpress.us/item/3256809119244467.html |
| ☐ | **NP18 Lens**              | Optics                        | https://www.aliexpress.us/item/3256808208343549.html |
| ☐ | **ESP32-C3**               | Main controller               | https://www.aliexpress.us/item/3256808855552297.html |
| ☐ | **5V Boost Converter**     | Steps battery → 5V            | https://www.amazon.com/dp/B0836J8LR4 |
| ☐ | **18350 Battery**          | Power source                  | https://amazon.com/dp/B07QDDS64F |
| ☐ | **Battery Contacts**       | Spring terminals              | https://amazon.com/dp/B0FSDNXKJY |
| ☐ | **Rotary Pot (20k)**       | Brightness / contrast control | https://www.aliexpress.us/item/3256803337433740.html |
| ☐ | **Buttons (x3)**           | Zoom / Mode / Record          | https://www.aliexpress.us/item/3256806708603327.html |
| ☐ | **Runcam DVR (Optional)**  | Recording                     | https://www.aliexpress.us/item/3256809185467125.html |
| ☐ | **MicroSD Extender**       | Required for DVR              | https://amazon.com/dp/B0D7YZ5PG3 |
| ☐ | **M2.5 x 6 Bolts**         | Mounting hardware             | https://amazon.com/dp/B0GCSRG1WD |
| ☐ | **Heat Set Inserts**       | Threaded inserts              | https://amazon.com/dp/B0D7YZ5PG3 |
| ☐ | **Silicone Wire**          | Internal wiring               | https://amazon.com/dp/B01KQ2JNLI |

---

### 🧠 Notes
- Choose **one thermal core** (Mini2 *or* P6 depending on your build)
- DVR + MicroSD extender are **optional**, the code funcitons the same, the record button can be used for pallete switching, etc..
- Everything else is required for a full build

---

<img width="1429" height="1128" alt="image" src="https://github.com/user-attachments/assets/cf64fd9d-ef84-4fcf-8cc9-e876ee20d669" />


