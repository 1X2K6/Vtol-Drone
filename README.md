# Vtol-Drone

# No-Control-Surface Vtol Drone

A custom VTOL search-and-rescue (SAR) drone built **without control surfaces** Pitch, roll, and yaw are achieved strictly through **differential motor thrust and motorized motor/pod movement**.

For SAR operations, the drone uses a **4mm front thermal camera**, **dual RunCam FPV cameras**, a **3-way video switcher** over a **1.2 GHz** link, and under the drone near the front is a **24 GHz mmWave human-detection radar** sweeping **180° on a servo**—with radar visuals displayed on a **2.4" TFT screen** and relayed live to the ground station by the second FPV camera.

---

## Table of Contents
* Features
* Hardware & Bill of Materials
* System Architecture
* Sensor & Camera Layout
* Build & Setup Notes
* Repository Structure
* Safety Notice

---

## Features
* **Control-Surface-Free Flight:** Eliminates linkages and hinged aerodynamic surfaces; relies purely on thrust allocation and motor/pod articulation.
* **180° Sweeping Human-Detection Radar:** A **24 GHz mmWave radar** mounted on a **180° servo** beneath the fuselage scans wide sectors for human presence/micro-motion.
* **Optical Radar Relay:** An **ESP32** renders radar detections onto a **2.4" TFT screen**, which is captured by **RunCam FPV #2** and transmitted to the ground via the **3-way video switcher**—requiring zero extra telemetry downlink hardware.
* **Tri-Feed 1.2 GHz Video Link:** Switch mid-flight between frontal thermal search, top-nose navigation FPV, and the live TFT radar display.

---

## 🛠 Hardware & Bill of Materials

### Flight, Propulsion & Control
* **Flight Controller:** Matek H743 (Qty: 1) — Main autopilot & motor/actuator mixing
* **Motors:** EMAX ECO II 3115 900KV (Qty: 4) — Hover & forward-flight propulsion
* **ESCs:** Hobbywing Skywalker V2 80A (Qty: 4) — Brushless motor speed control
* **Battery:** 10,000 mAh (Qty: 1) — High-capacity flight pack
* **RC Receiver:** RadioMaster XR3 Nano (Qty: 1) — Control & telemetry link
* **RC Transmitter:** RadioMaster TX16S MkIII (Qty: 1) — Ground pilot controller
* **Radar Sweep Servo:** 180° Servo (Qty: 1) — Sweeps the 24 GHz mmWave radar under the drone

### Sensors, Display & Video
* **Thermal Camera:** 4mm Lens Thermal Camera (Qty: 1) — Front-mounted heat-signature search
* **FPV Camera #1:** RunCam FPV Camera (Qty: 1) — Top-nose forward flight view
* **FPV Camera #2:** RunCam FPV Camera (Qty: 1) — Aimed at 2.4" TFT screen to relay radar data
* **Video Switcher:** 3-Way Video Switcher (Qty: 1) — Selects between Thermal, FPV #1, and FPV #2
* **Video Link:** 1.2 GHz VTX & VRX (Qty: 1 set) — Long-range analog video transmission
* **mmWave Radar:** 24 GHz Human Detection Module (Qty: 1) — Sweeps 180° underneath for human detection
* **Microcontroller:** ESP32 (Qty: 1) — Reads radar UART/IO & drives TFT UI
* **Display:** 2.4" TFT Screen (Qty: 1) — Displays radar sweep & human-detection alerts

### Airframe Materials
* **Carbon Fiber Tubes:** Primary structural booms and spars
* **PolyLite LW-PLA:** Lightweight aerodynamic body and wing shells
* **Creality PETG-CF:** High-rigidity motor mounts, tilting pods, and servo brackets
* **Generic TPU:** the wing tips (also can be used for camera mounts and the radar pod)

## 📡 Sensor & Camera Layout
* **Front Nose: 4mm thermal camera with an unobstructed forward/downward search view.
* **Top of Nose: RunCam FPV #1 for standard pilot situational awareness and landing.
* **Underbelly: 24 GHz mmWave radar mounted on a 180° sweep servo for wide-arc human detection.
* **Internal Bay: ESP32 + 2.4" TFT screen enclosed with RunCam FPV #2 in a light-shielded shroud to prevent screen glare on the video feed.


## Build & Setup Notes
* **Bench Verification (Props Off): Confirm motor spin directions, motor/pod movement endpoints, and failsafe behavior before attaching propellers.
* **TFT Shroud: Enclose the 2.4" TFT screen and RunCam FPV #2 in a dark matte printed shroud (PETG-CF or TPU) so ambient sunlight does not wash out the radar display.
* **RF Separation: Keep the 1.2 GHz VTX antenna separated from the RadioMaster XR3 Nano and 24 GHz radar to minimize RF interference.

## ⚠️ NOTE: The 3d model files are not mine it is bought i just modified some of the models and uploaded them to build the drone you would need to buy the stl files.
