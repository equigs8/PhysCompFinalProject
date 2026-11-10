# Affinity Map: Project 2 Interviews

Following the guidelines in `Synthesis review.md`, each observation or quote below is recorded as an individual note marked with its participant code (`P1`–`P4`), grouped by similarity from the bottom up, and labeled with an **"I" statement** in the participant's voice.

## Participant Key
- **P1:** Will 
- **P2:** Liam 
- **P3:** Chloe 
- **P4:** Marcus 
---

## Affinity Wall

```mermaid
flowchart TD
    classDef p1 fill:#fef3c7,stroke:#d97706,stroke-width:1.5px,color:#000
    classDef p2 fill:#dbeafe,stroke:#2563eb,stroke-width:1.5px,color:#000
    classDef p3 fill:#fce7f3,stroke:#db2777,stroke-width:1.5px,color:#000
    classDef p4 fill:#dcfce7,stroke:#16a34a,stroke-width:1.5px,color:#000

    subgraph G1["I play handheld games in short bursts away from a desk"]
        direction TB
        N1["P1: Plays on train commute and on the couch; almost never plays docked or at a desk"]:::p1
        N2["P2: Carries a pocket device for 15-minute sessions at a coffee shop or subway"]:::p2
        N3["P3: Plays on the couch or traveling; Steam Deck is too bulky for daily errands so a pocket handheld appeals"]:::p3
        N4["P4: Plays 60% on the go or away from desk (porch, train, bed) after staring at a monitor all day"]:::p4
    end

    subgraph G2["I need a comfortable horizontal grip and responsive tactile buttons to enjoy playing"]
        direction TB
        N5["P1: Vertical is charming, but over 15-20 mins horizontal feels more natural so hands aren't squished"]:::p1
        N6["P1: Won't pick it up if it has sharp edges, gets hot, creaks, or has loud, stiff buttons"]:::p1
        N7["P2: Vertical handhelds cause 'Game Boy claw' cramping after 30 mins; horizontal gives palms a rest"]:::p2
        N8["P2: False diagonals on a D-pad or mushy buttons make action games frustrating"]:::p2
        N9["P3: Horizontal keeps thumbs from bumping or blocking the screen; snappy buttons give 'juice'"]:::p3
        N10["P4: Thick devices need a horizontal layout for two hands; rattling batteries ruin immersion"]:::p4
    end

    subgraph G3["I want the enclosure to look intentional and safe to carry, though my style preference varies"]
        direction TB
        N11["P1: Drawn to cute/clean aesthetics; terrified of breaking or shocking himself on exposed circuit boards/wires"]:::p1
        N12["P2: Loves the maker/DIY aesthetic (translucent cases, visible PCBs, clean 3D prints) if ergonomics are clean"]:::p2
        N13["P3: Cohesive colors and a screen bezel elevate it from a 'class project' to a gadget to show friends"]:::p3
        N14["P4: Digs DIY silkscreen/cassette-futurism cases as long as the screen isn't recessed too deeply"]:::p4
    end

    subgraph G4["I want to load multiple games easily so the device doesn't become a desk ornament"]
        direction TB
        N15["P1: Will give up immediately if downloading requires drivers, CLI, or SD formatting; wants USB drag-and-drop or Wi-Fi"]:::p1
        N16["P2: Fine flashing via USB/esptool or microSD if steps are clearly in a README"]:::p2
        N17["P3: Fine with USB/SD herself, but wants an SD menu or Chrome web flasher so non-technical friends can play her games"]:::p3
        N18["P4: A single-game handheld becomes a desk ornament after a week; web flasher or SD loader makes swapping 10x likelier"]:::p4
    end

    subgraph G5["I design better, more focused games when working within clear hardware constraints"]
        direction TB
        N19["P2: Strict limits on resolution, palette, RAM, and buttons kill scope creep and focus 100% on core mechanics"]:::p2
        N20["P3: Fixed memory and resolution budgets force architecture-first thinking toward arcade, turn-based, or puzzle games"]:::p3
        N21["P4: 6 buttons, a tiny screen, and a piezo buzzer inspire single-screen arcade or grid-based tactics designs"]:::p4
    end

    subgraph G6["I need engine abstractions, asset converters, and an emulator so I don't get stuck on low-level boilerplate"]
        direction TB
        N22["P2: Needs a desktop simulator with hot-reloading so he doesn't have to compile and flash over serial for every tweak"]:::p2
        N23["P2: Converting sprites and audio into custom byte arrays is a huge friction point without a converter script"]:::p2
        N24["P3: Needs a clean HAL (update, draw, delta time, non-blocking audio) instead of writing raw SPI drivers"]:::p3
        N25["P3: Debugging embedded freezes (SRAM exhaustion, watchdog timers) via serial logs is a major hurdle"]:::p3
        N26["P4: Needs flicker-free rendering, bitmap font helpers, collision helpers, and a tone sequencer for piezo audio"]:::p4
    end

    subgraph G7["I rely on working starter templates and copy-pasteable examples to get started quickly"]
        direction TB
        N27["P2: Wants a 30-line 'Hello World' game loop (screen init, sprite draw, beep, input) and documented hardware quirks"]:::p2
        N28["P3: Needs a Quickstart under 30 minutes, architecture/pinout diagrams, and warnings on gotchas like draw-loop allocation"]:::p3
        N29["P4: Loses trust when 'Getting Started' fails on step 2 from library updates; wants an out-of-the-box sample game repo"]:::p4
    end
```

---

## Cluster Breakdown & Traceability

### 1. "I play handheld games in short bursts away from a desk"
- **P1:** "On my commute on the train, I'm pretty much always playing small puzzle games on my phone. When I'm at home on the couch, I'll use my Switch in handheld mode... I almost never play docked on a big TV or sit at a desk."
- **P2:** "On the go, I always carry a small retro handheld... in my jacket pocket. I love having a dedicated pocket device for 15-minute sessions at a coffee shop or on the subway because touchscreens are terrible for action games."
- **P3:** "Mostly at home, either curled up on the couch with my Steam Deck... for daily errands it's a bit too bulky to carry around, so a smaller pocket handheld is actually really appealing."
- **P4:** "Honestly, 60% on the go or away from my desk, 40% at home. Since I stare at my computer monitor all day for work and game dev, the last thing I want to do at night is sit in the same chair."

### 2. "I need a comfortable horizontal grip and responsive tactile buttons to enjoy playing"
- **P1:** "If I'm playing for more than 15 or 20 minutes, a horizontal layout like the Switch Lite feels a bit more natural so my hands aren't squished together."
- **P1:** "If a device has sharp plastic edges, gets hot, or has really loud, stiff buttons that click too hard, I honestly just won't pick it up."
- **P2:** "Vertical handhelds look nostalgic, but after 30 minutes of playing an action game, my hands cramp up, what people call the 'Game Boy claw.' A wider horizontal layout gives your palms something to rest against..."
- **P2:** "If the D-pad has false diagonals or the buttons feel mushy and don't rebound quickly, action games become frustrating to play."
- **P3:** "I'm a big fan of horizontal layouts... it keeps your thumbs from blocking the bottom corners of the display or bumping into each other." / "When I press a button, I want a distinct, snappy actuation so I know my input registered without looking down."
- **P4:** "If the device is thick (because of AA batteries or a dev board), a horizontal layout usually feels way better to hold with two hands." / "Making sure the battery pack or back cover doesn't rattle when you're mashing buttons is a big deal for immersion."

### 3. "I want the enclosure to look intentional and safe to carry, though my style preference varies"
- **P1:** "If it looks like an exposed circuit board with wires sticking out, I'd be terrified of breaking it or shocking myself. It needs to look like a finished, friendly object I can toss in my tote bag."
- **P2:** "I actually love the maker/industrial aesthetic, translucent cases, visible PCBs, or clean 3D-printed enclosures look super cool to me."
- **P3:** "Choosing a cohesive color scheme for the buttons and shell, or having a nice bezel around the screen, elevates it from 'class project' to 'a gadget I want to show my friends.'"
- **P4:** "I really dig when DIY handhelds embrace what they are, like having a cool silkscreen on the PCB or a chunky retro cassette-futurism case. As long as the screen isn't recessed too deeply where viewing angles get cut off, I'm happy."

### 4. "I want to load multiple games easily so the device doesn't become a desk ornament"
- **P1:** "If I have to install special drivers, use a command line, or format an SD card with weird files, I'm going to give up immediately. Unless it's as simple as plugging in a USB cord and dragging a file over, or tapping a button over Wi-Fi, I'll just stick to whatever comes pre-installed."
- **P2:** "Flashing firmware via USB, using `esptool`, or dropping `.bin` files onto a microSD card doesn't bother me at all, as long as the steps are clearly written down in a README."
- **P3:** "If I'm making a game *for* this device and want non-technical friends to play it, the easier the loading process is (like selecting a game from a menu on an SD card or a web flasher in Chrome), the better."
- **P4:** "A handheld with only one hardcoded game becomes a desk ornament after a week. Being able to swap games or have a multi-game launcher menu gives it real longevity... having a web-based USB flasher or SD card loader makes it 10x more likely I'll swap games frequently."

### 5. "I design better, more focused games when working within clear hardware constraints"
- **P2:** "When you have strict limits on resolution, color palette, button count, or RAM, like on an ESP32, it kills scope creep. You stop worrying about fancy shaders or massive asset pipelines and focus 100% on making a single core mechanic feel good."
- **P3:** "On a constrained microcontroller, I have to plan around fixed memory budgets, lower screen resolutions, and limited inputs right from the design phase. It definitely steers me toward arcade-style, turn-based, or puzzle mechanics..."
- **P4:** "If you tell me I only have 6 buttons, a small screen, and a simple piezo buzzer for audio, I immediately start designing around those constraints... Constraints breed creativity, provided the core loop runs smoothly."

### 6. "I need engine abstractions, asset converters, and an emulator so I don't get stuck on low-level boilerplate"
- **P2:** "A fast iteration loop, specifically a desktop simulator or emulator with hot-reloading and proper debug output. If I have to compile and flash hardware over serial every single time... development grinds to a halt."
- **P2:** "Converting sprites and audio into whatever byte array or custom format the engine expects is usually a huge friction point if there isn't a simple converter script."
- **P3:** "A clean hardware abstraction layer (HAL) for graphics, timing, and audio. I don't want to write raw SPI display drivers or wrestle with hardware timers just to get a stable 30 or 60 FPS game loop."
- **P3:** "When a game freezes on an embedded device, sometimes the screen just locks up and you're stuck printing serial logs to figure out if you ran out of SRAM or hit a watchdog timer."
- **P4:** "Dealing with flicker-free screen refreshes (like double buffering or dirty rectangles) on small displays can be a nightmare if the platform SDK doesn't handle it for you. Having built-in helpers for drawing bitmap fonts and checking bounding-box collisions saves so much boilerplate."

### 7. "I rely on working starter templates and copy-pasteable examples to get started quickly"
- **P2:** "Working, copy-pasteable minimal code examples make it; walls of auto-generated Doxygen function signatures with zero context break it. Tell me *how* to initialize the screen, draw a 8x8 sprite at `(x, y)`, play a beep, and read button state in a 30-line 'Hello World' game loop."
- **P3:** "If it takes me more than 30 minutes to set up the toolchain, compile the template project, and get it running on the device, that's a major red flag. Good documentation needs a clear diagram of the system architecture, pinouts/button mappings, and explicit warnings about known gotchas..."
- **P4:** "Nothing breaks trust in documentation faster than following the 'Getting Started' guide, hitting a compiler error on step 2 because a library version changed... A working template repo with a sample game (like Pong or Tic-Tac-Toe) that compiles out-of-the-box is worth a thousand pages of docs."
