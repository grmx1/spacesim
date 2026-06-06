# 🚀: SpaceSim

> On development Solar System sized physics simulator to predict and visualize orbits and planetary interaction.

<img width="1273" height="898" alt="spacesim1" src="https://github.com/user-attachments/assets/d007e7e1-045d-43ca-8b30-b600a3ca7bbe" />

## 🌟 Highlights

- Gravity based interaction between space objects.
- Custom 3D rendering and optimizations based on SDL2
  
<img width="1023" height="779" alt="spacesim2" src="https://github.com/user-attachments/assets/df85b20e-f313-46d8-a2e7-40f1938e62b6" />

## ℹ️ Overview

This is a pet project inspired on Universe Sandbox in order for me to learn more about graphics, optimizations, and how to make real life physics be accurate on a computer.

## 🚀 Usage

The most basic flags are:
- *--labels*: shows the names of the space objects.
- *--traces*: shows the trail of the planets.
- *--forward*: traces fall behind the planets.

Also by pressing "m" a menu will pop up to modify each space object's mass

<img width="927" height="792" alt="spacesim3" src="https://github.com/user-attachments/assets/82de0a0b-6f0c-4af5-a777-1a2a1da321fd" />

## ⬇️ Installation

Debian / Ubuntu
```bash
git clone https://github.com/grmx1/spacesim.git
sudo apt install cmake libsdl2-dev libsdl2-ttf-dev build-essential
```

Arch
```bash
git clone https://github.com/grmx1/spacesim.git
sudo pacman -Syu --needed cmake sdl2 sdl2_ttf base-devel
```

In order to build
```bash
cd spacesim && mkdir build && cd build && cmake .. && make
```
