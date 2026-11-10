# Synthesis

Based on our [Affinity Map](Affinity%20Map.md) and [Journey Map](Journey%20Map.md), below is the synthesis of our four interviews (`P1` Will, `P2` Liam, `P3` Chloe, and `P4` Marcus).

---

## 1. The 5 Synthesis Questions

1. **Who (Which participants experience this problem):**
   Both **indie game developers (`P2`, `P3`, `P4`)** who build small games for constrained handheld hardware and **casual or on-the-go players (`P1`, `P2`, `P3`, `P4`)** who want a dedicated pocket device for 15-to-30-minute play sessions away from their desks (on trains, couches, and porches).

2. **What they do (What do they actually do when the problem occurs):**
   - **Developers (`P2`, `P3`, `P4`):** When creating games for microcontroller hardware, their iteration loop grinds to a halt (`P2`). Instead of designing gameplay, they get stuck manually formatting sprites and audio into raw byte arrays (`P2`), fighting display flicker and raw SPI drivers (`P3`, `P4`), digging through forums when starter guides fail to compile (`P4`), and printing serial logs to debug silent memory/watchdog freezes (`P2`, `P3`).
   - **Players (`P1`, `P2`, `P3`, `P4`):** When a handheld has exposed electronics, sharp edges, stiff/mushy buttons, or a cramped vertical layout ("Game Boy claw"), players stop picking it up after 15–30 minutes (`P1`, `P2`, `P3`). Furthermore, when loading new games requires command-line tools, drivers, or manual SD formatting, casual players give up immediately (`P1`), leaving single-game devices to become "desk ornaments" after a week (`P4`).

3. **Why it happens (What is the root cause, not the surface complaint?):**
   **Root Cause:** Custom microcontroller handhelds are usually engineered as raw hardware prototypes rather than cohesive game platforms, they expose low-level embedded complexity (manual memory management, raw peripheral drivers, CLI firmware flashing, and unenclosed PCBs) directly to developers and players instead of abstracting it behind a developer-friendly SDK, working starter templates, and an ergonomic, finished physical shell.

4. **What surprised you (What contradicted your assumptions?):**
   - **Hardware limits are a feature, not a bug:** We assumed strict microcontroller limits (low RAM, small resolution, 6 buttons, basic buzzer audio) would frustrate developers. Instead, all three developers (`P2`, `P3`, `P4`) *loved* hardware constraints because they kill scope creep and inspire focused, mechanics-first design, frustration only happens when the *tooling, asset pipeline, or documentation* is broken.
   - **Nostalgic vertical layouts lose to horizontal ergonomics:** Even participants who found vertical Game Boy-style layouts charming or pocketable (`P1`, `P4`) agreed with heavy gamers (`P2`, `P3`) that a **horizontal layout** is far more comfortable for any session lasting longer than 15–20 minutes.
   - **The wide gap in aesthetic and loading tolerance:** While indie devs (`P2`, `P4`) actively enjoy visible PCBs and don't mind CLI flashing via `esptool`, a casual player (`P1`) is terrified of breaking or shocking themselves on exposed boards and will abandon game loading altogether if it isn't plug-and-play.

5. **The real problem (In one sentence, what is broken? What is stopping people from achieving their goal?):**
   High low-level technical friction, spanning slow hardware compile-flash cycles, missing engine/asset abstractions, fragile documentation, cramped or unpolished enclosures, and complex firmware loading, prevents indie developers from rapidly creating games for microcontroller handhelds and stops casual players from comfortably playing and swapping them on the go.

---

## 2. Synthesis Paragraph

Indie game developers (`P2`, `P3`, `P4`) and on-the-go players (`P1`, `P2`, `P3`, `P4`) both gravitate toward small, pocketable handhelds for short 15-to-30-minute gaming bursts away from a computer desk, and developers actively embrace strict microcontroller constraints on screen size, memory, and inputs because those limits kill scope creep and foster creative, mechanics-first design (`P2`, `P3`, `P4`). However, when developers actually sit down to build for custom hardware, their momentum grinds to a halt (`P2`): they spend hours converting sprites and audio into byte arrays by hand (`P2`), wrestling with flicker-free display rendering and raw SPI drivers (`P3`, `P4`), debugging silent SRAM crashes via serial logs (`P2`, `P3`), and waiting on slow hardware flashes for every tiny gameplay tweak (`P2`). At the same time, when players pick up DIY handhelds, cramped vertical layouts cause "Game Boy claw" hand fatigue (`P1`, `P2`, `P3`), mushy buttons or rattling parts ruin game feel (`P2`, `P4`), exposed circuit boards intimidate casual users (`P1`), and command-line flashing barriers cause non-technical players to give up on loading new games, turning a single-game device into a desk ornament within a week (`P1`, `P3`, `P4`). **Ultimately, this happens because DIY microcontroller handhelds treat the hardware prototype as the finished product, exposing raw embedded engineering bottlenecks, from display drivers and byte-array pipelines to CLI firmware flashing and bare PCBs, directly to creators and players instead of providing a cohesive abstraction layer, fast simulation tooling, and an approachable physical enclosure.**

---

## 3. Problem Statement & How Might We (HMW)

### Root Cause Sentence
> DIY microcontroller handhelds treat the hardware prototype as the finished product, exposing raw embedded engineering bottlenecks, from display drivers and byte-array pipelines to CLI firmware flashing and bare PCBs, directly to creators and players instead of providing a cohesive abstraction layer, fast simulation tooling, and an approachable physical enclosure.

### Problem Statement *(No solution named)*
> **Indie game developers and on-the-go players** need a way to **rapidly build, test, and play multiple small-scope games on constrained pocket hardware without getting blocked by embedded systems boilerplate, slow hardware iteration loops, hand-cramping ergonomics, or technical file-loading hurdles**, because **raw microcontroller friction stalls creative development and intimidates casual players from picking up the device or trying new games.**

### How Might We (HMW) Question
> **How might we** remove the low-level friction of developing, loading, and playing games on a microcontroller handheld so that indie developers can iterate and casual players can comfortably enjoy and swap games anywhere?
