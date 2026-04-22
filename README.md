# 🌄 Interactive Village Environment Simulation

An OpenGL-based graphical simulation of a **village environment** featuring **day-night transition, animations, and interactive controls**.

---

## 📌 Features

* 🌅 **Day & Night Mode**

  * Sun, moon, stars with brightness control
* 🌄 **Natural Environment**

  * Hills with smooth cosine interpolation
  * Gradient sky
* 🌊 **River Animation**

  * Flowing water waves
  * Boat movement, fish jumping, ducks floating
* 🏡 **Village Scene**

  * Huts, well, trees, cows
* 🌾 **Field & Playground**

  * Football ground with players
  * Ball passing animation
  * Seesaw with children
* 🦋 **Extra Animations**

  * Birds flying
  * Butterflies moving
  * Clouds drifting
* 🎮 **Interactive Controls**

  * Zoom, rotate, translate scene
  * Toggle day/night

---

## 🛠️ Technologies Used

* **C++**
* **OpenGL (GLUT)**
* **Graphics Algorithms**

  * DDA Line Drawing
  * Bresenham Line Algorithm
  * Midpoint Circle Algorithm

---

## 🎮 Controls

| Key        | Action                 |
| ---------- | ---------------------- |
| Arrow Keys | Move scene (translate) |
| `+` / `-`  | Zoom in / out          |
| `R` / `r`  | Rotate around Y-axis   |
| `T` / `t`  | Tilt around X-axis     |
| `d`        | Switch to Day          |
| `n`        | Switch to Night        |
| `0`        | Reset transformations  |
| `ESC`      | Exit program           |

---

## 🚀 How to Run

### 1. Install dependencies

Make sure OpenGL and GLUT are installed.

### 2. Compile

```bash
g++ main.cpp -o village -lGL -lGLU -lglut
```

### 3. Run

```bash
./village
```

---

## 🧠 Project Concepts

This project demonstrates:

* Real-time animation using **timer functions**
* Use of **trigonometric functions (sin, cos)** for motion
* Scene organization into **zones**
* Efficient drawing using **loops and arrays**
* Interactive graphics using **event handling**

---

## 📚 Learning Outcomes

* Understanding of computer graphics basics
* Implementation of classic drawing algorithms
* Handling user interaction in OpenGL
* Creating structured and animated scenes

---

## 👨‍💻 Author

Imtiazpias

---

## 📄 License

This project is for educational purposes.
