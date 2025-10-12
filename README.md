# LemonEngine

---

<img src="repo/card.svg"/>

## Documentation

API documentation is available for each branch:

- [Main Branch Docs](https://onirau.github.io/LemonEngine/main/)
- [Develop Branch Docs](https://onirau.github.io/LemonEngine/develop/)

## Supported Platforms

- Linux
- Windows

## Dependencies

- [Luau](https://github.com/luau-lang/luau)
- [BGFX](https://github.com/bkaradzic/bgfx)
- [BX](https://github.com/bkaradzic/bx)
- [BIMG](https://github.com/bkaradzic/bimg)
- [GLFW](https://github.com/glfw/glfw)

## How to Build

### Linux

1. **Clone the repository**

```bash
   git clone https://github.com/Onirau/LemonEngine.git
   cd LemonEngine
```

2. **Install required system libraries**

```bash
   sudo apt update
   sudo apt install -y build-essential cmake libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxxf86vm-dev
```

3. **Navigate to the build directory**

```bash
   cd build
```

4. **Configure the project**

```bash
   cmake ..
```

5. **Build the project**
   make -j$(nproc)

---

### Windows

1. **Clone the repository**
   git clone https://github.com/Onirau/LemonEngine.git
   cd LemonEngine

2. **Install CMake and Visual Studio**

````bash
   - Install [CMake](https://cmake.org/download/)
   - Install Visual Studio with "Desktop development with C++" workload

3. **Navigate to build folder**

```bash
   cd build
````

4. **Configure the project**

```bash
   cmake ..
```

5. **Build the project**

```bash
   cmake --build .
```

## How to Use

LemonEngine supports executing a Luau Script by passing it as command-line argument, and with a familiar scripting API, you can create parts and edit them in real time!

```luau
local part1 = Instance.new("Part")
part1.Color = Color3.new(1,0,0)
part1.Position = Vector3.new(0, 2.5, 0)
part1.Size = Vector3.one * 2
part1.Shape = "CornerWedge"

local part2 = Instance.new("Part")
part2.Color = Color3.new(1,0,0)
part2.Position = Vector3.new(3, 2.5, 0)
part2.Size = Vector3.one * 2
part2.Shape = "Wedge"

local t = 0
while true do
    t += task.wait()
    part1.Rotation = Vector3.new(0, t * 100, 0)
    part1.Color = Color3.fromHSV((t*0.2)%1, 1, 1)

    part2.Rotation = Vector3.new(0, t * 100, 0)
    part2.Color = Color3.fromHSV((t*0.2)%1, 1, 1)
end
```

# Checklists

Below is what you can expect for the future in LemonEngine's development! Expect this big list to expand as time goes on!

## Core Checklist

- [ ] Rendering
- [ ] File System
- [ ] Audio

## Other Checklist

- [ ] Signals in Lua
- [x] Instance System
- [ ] Physics
- [ ] Data types
  - [ ] CFrame
  - [ ] Random
  - [x] game
  - [x] workspace
- [ ] Services
  - [ ] RunService
  - [ ] Lighting
  - [ ] StarterGui
  - [x] Workspace
- [ ] Lighting system
  - [ ] Sun lighting
  - [ ] Shadows
  - [ ] Lights from Instance Lights

# How to Help

You can help out by using LemonEngine and reporting any bugs you find!

If you know how to code, help out by creating pull requests to add and change code!
