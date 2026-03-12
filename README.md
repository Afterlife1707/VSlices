# ParkourBase

A modular parkour system for Unreal Engine 5, part of a vertical slices project. This open-source system provides smooth, responsive parkour mechanics through independent, reusable components. This system is part of a larger vertical slices project. Each slice focuses on a specific game mechanic, with the parkour system being the first implementation.

## Overview

ParkourBase implements a comprehensive parkour system by breaking down complex movement mechanics into modular components that can be easily mixed, matched, and extended. Each component handles a specific parkour action while maintaining independence from others, allowing for flexible gameplay design. The project contains a small parkour map ready for playtesting, using engine assets.

## Features

### Core Movement Components

<img width="569" height="619" alt="image" src="https://github.com/user-attachments/assets/307a3f3f-b17b-4c25-9fa6-121220284d9d" />
                                                 
- **Sprint Component** - Custom sprint with cooldown, stamina system, and breathing audio that reacts to exhaustion level
- **Slide Component** - Ground sliding with momentum preservation based on timer and slope(used in accordance with the Slope Component to detect slopes)
- **Landing Component** - Fall detection and landing animations based on height
- **Vault/Mantle Component** - Obstacle traversal for low and high obstacles with arc-based vaulting and climb/mantle support. Uses anim notify (AN_VaultFinish) to drive vault completion
- **Wall Run Component** - Vertical wall running with camera tilting and its own jump function
- **Grappling Hook Component** - Simple grappling gun with a cable component
- **Ledge Component** — Ledge detection and hanging, with clamped freelook and camera offset during hang

<img width="1089" height="481" alt="image" src="https://github.com/user-attachments/assets/65833f94-d9c2-46eb-9c8d-627e9531ea55" />

### Enhanced Jump System
- **Coyote Time** - Grace period for jumping after leaving platforms
- **Sprint Boost** - Forward momentum boost when jumping while sprinting
- **Context-Aware Jumping** - Jump behaviour adapts based on current state (ledge hang, wall run, vault attempt, standard jump)
- 
### Audio System

- **Footstep System** — Anim notify driven (AN_Footstep) with physical material surface detection. Supports separate crouch sounds and per-surface audio via custom UFootstepData data asset
- **Breathing Audio** — Driven by stamina level on SprintComponent. Volume and pitch increase as stamina depletes, fades out on recovery

### Camera System

- First-person embodied camera attached to neck_02 bone for natural head movement
- Camera lag via USpringArmComponent for smooth motion
- Clamped freelook during vault and ledge hang states
- Dynamic camera offset during ledge hang, smoothly interpolated in CalcCamera
- Wall run camera tilt via CalcCamera override
  
## Architecture

### Component-Based Design
All parkour mechanics inherit from UParkourComponentBase, which provides standardised access to the owning character and movement component. Components have no dependencies on each other, they are all driven through the character class and do not need to know about each other.
A custom logging macro system (LoggingMacros.h) is used throughout for consistent log output.

### Input Flow
**Controller Input → Character Class → Individual Components → Movement Execution**

### Key Design Patterns

- WarnMissingComponent on the character — logs a once-per-session warning if a component is missing
- WarnMissingAsset on ParkourComponentBase — same pattern for missing data assets
- Components use bAutoActivate / SetActive() for enable/disable rather than custom booleans
- Duplicate component prevention via OnComponentCreated override in ParkourComponentBase

### Installation
1. Clone this repository into your UE5 project's Source folder
2. Regenerate project files
3. Compile the project
4. AVSlicesCharacter is the character class being used, which is the base type of the "OwnerCharacter" variable used in the components
5. AVSlicesController is the controller class handling the enhanced action inputs

## Usage

### Adding Components
Components are added freely via Blueprint in the character's Components panel. The character caches them in BeginPlay via FindComponentByClass. Any component can be omitted — the character will warn once if a component is missing and degrade gracefully.

### Component States
Each component exposes getter functions for current state:
- Sprinting, exhaustion, and stamina level
- Sliding active state
- Slope info for movement restriction
- Vault type and vaulting state
- Wall run status and camera tilt data
- Ledge grab state
  
### Blueprint-Configurable Parameters
Every component exposes its tuning values as EditDefaultsOnly properties, including sprint speed, stamina drain/regen rates, vault heights, trace distances, wall run parameters, and audio thresholds.

### Animations and Audio Setup 
- All the animations are from Mixamo. Some of them are reused and combined using animation composite.
- The Animation Blueprint is ABP_Character, extended from ABP_Manny with custom parkour state logic
- Footstep audio is driven by AN_Footstep anim notify with physical material surface detection — configure surfaces via the DA_Footstep data asset
- Vault completion is driven by AN_VaultFinish anim notify
- Some animations use a custom notify to drive control rotation, syncing the camera with the animation (e.g. landing)
- There is audio on the animations available, which are reused quite a lot, so they may not sound great. They're all implemented using notifiers.

<img width="720" height="500" alt="image" src="https://github.com/user-attachments/assets/3c2898db-db25-4282-a90e-80c25196117e" />

<img width="500" height="500" alt="image" src="https://github.com/user-attachments/assets/647c9fd4-2b3b-495b-8cd6-b7cf9002d487" />


## Acknowledgments

- Mixamo for character animations
- Pixabay and Youtube for Royalty free sound effects
- https://www.youtube.com/watch?v=z4LdRAMaifY&ab_channel=UnrealAxis
- https://www.youtube.com/watch?v=zF7z-xrFABY&t=1126s&ab_channel=LocoDev
- https://www.youtube.com/watch?v=z4LdRAMaifY&t=516s&ab_channel=UnrealAxis
- https://trello.com/c/SLe2bi6F/59-https-wwwyoutubecom-watchvwmzkj1gsexmt44s
- Cartoon Grappling Hook" (https://skfb.ly/6UNYo) by Matt LeMoine is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/)
