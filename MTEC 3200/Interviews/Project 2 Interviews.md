# Project 2 Interviews

## Interviewee 1: Will (25, Casual Player / Average Person)
*Background: Works in marketing. Plays games occasionally to unwind, mostly mobile puzzle games, cozy Switch games, and nostalgia picks, but wouldn't consider himself a "gamer."*

### General
**1. What's your favorite game and why?**
"Probably *Animal Crossing: New Horizons* or *Mario Kart 8*. I also play *NYT Connections* and *Block Blast* on my phone almost every day. For *Animal Crossing*, I just loved having a low-stakes routine where I could decorate and check in for 20 minutes without feeling stressed or needing fast reflexes. With *Mario Kart*, it's just something fun and social I can play when friends are over."

**2. Where do you like to play games? At home or on the go?**
"Both, depending on what it is! On my commute on the train, I'm pretty much always playing small puzzle games on my phone. When I'm at home on the couch, I'll use my Switch in handheld mode while watching a show in the background. I almost never play docked on a big TV or sit at a desk."

### Game Dev Platform
*(Game dev platform questions skipped as interviewee is not a game developer.)*

### Physical Product Player Perspective
**1. Do you prefer a horizontal or vertical layout?**

"I actually think a vertical layout is really charming, it reminds me of the old Game Boy Color my older brother had, or just holding a phone with one or two hands. That said, if I'm playing for more than 15 or 20 minutes, a horizontal layout like the Switch Lite feels a bit more natural so my hands aren't squished together."

**2. How important is the feel of the product?**
"Super important. If a device has sharp plastic edges, gets hot, or has really loud, stiff buttons that click too hard, I honestly just won't pick it up. I like buttons that have a soft, tactile press and a casing that feels smooth and solid in my hands, not hollow or creaky."

**3. How important is the look of the product?**
"Really important! I'm definitely drawn to cute or clean aesthetics, like pastel colors, retro translucent shells, or something minimalist like the Playdate. If it looks like an exposed circuit board with wires sticking out, I'd be terrified of breaking it or shocking myself. It needs to look like a finished, friendly object I can toss in my tote bag."

**4. How important is the ability to download new games? Would you jump through hurdles to do so?**
"Having a few extra games to try would be nice, as long as the ones already on it are fun and replayable. Honestly, though, I would *not* jump through hurdles to download new ones. If I have to install special drivers, use a command line, or format an SD card with weird files, I'm going to give up immediately. Unless it's as simple as plugging in a USB cord and dragging a file over, or tapping a button over Wi-Fi, I'll just stick to whatever comes pre-installed."

---

## Interviewee 2: Liam (26, Indie Game Dev & Heavy Gamer)
*Background: Solo indie developer working primarily in Godot and PICO-8. Plays a lot of roguelikes, precision platformers, and retro handheld games.*

### General
**1. What's your favorite game and why?**
"*Celeste* and *Spelunky 2* are tied for me. *Celeste* has the most fine-tuned, responsive movement feel of any platformer ever made. *Spelunky 2* is just a masterclass in systemic design where simple rules interact to create endless emergent chaos. Both games respect the player's time and skill."

**2. Where do you like to play games? At home or on the go?**
"I do both pretty heavily. At home, I'm at my PC desk playing Steam indies or testing builds. On the go, I always carry a small retro handheld, like my Miyoo Mini or Playdate, in my jacket pocket. I love having a dedicated pocket device for 15-minute sessions at a coffee shop or on the subway because touchscreens are terrible for action games."

### Game Dev Platform
**1. What is one feature that you couldn't live without?**
"A fast iteration loop, specifically a desktop simulator or emulator with hot-reloading and proper debug output. If I have to compile and flash hardware over serial every single time I want to tweak a jump arc or check a sprite alignment, development grinds to a halt. I also need a straightforward sprite/tilemap rendering API and easy button input polling."

**2. Does working with limits on a device that might have hardware restrictions affect your development?**
"In a great way, honestly. That's why I love fantasy consoles like PICO-8. When you have strict limits on resolution, color palette, button count, or RAM, like on an ESP32, it kills scope creep. You stop worrying about fancy shaders or massive asset pipelines and focus 100% on making a single core mechanic feel good. The only time hardware limits become frustrating is when the bottleneck is poorly documented, like mystery memory fragmentation crashing the device."

**3. What makes or breaks your ability to use documentation?**
"Working, copy-pasteable minimal code examples make it; walls of auto-generated Doxygen function signatures with zero context break it. Tell me *how* to initialize the screen, draw a 8x8 sprite at `(x, y)`, play a beep, and read button state in a 30-line 'Hello World' game loop. Also, explicitly documenting hardware quirks, like frame buffer memory limits or pin mappings, saves hours of headache."

**4. What are some of your hurdles when creating games?**
"Finishing the last 20%, UI menus, audio integration, and asset formatting. On custom or microcontroller-based hardware specifically, converting sprites and audio into whatever byte array or custom format the engine expects is usually a huge friction point if there isn't a simple converter script."

### Physical Product Player Perspective
**1. Do you prefer a horizontal or vertical layout?**
"Horizontal for sure. Vertical handhelds look nostalgic, but after 30 minutes of playing an action game, my hands cramp up, what people call the 'Game Boy claw.' A wider horizontal layout gives your palms something to rest against and spaces out the D-pad and action buttons much more ergonomically."

**2. How important is the feel of the product?**
"It's everything for a handheld. If the D-pad has false diagonals or the buttons feel mushy and don't rebound quickly, action games become frustrating to play. Even if the case is 3D-printed, good button travel, a comfortable grip, and balanced weight make it feel like a real console rather than a fragile prototype."

**3. How important is the look of the product?**
"Moderately important, though I'm way more forgiving than a casual user. I actually love the maker/industrial aesthetic, translucent cases, visible PCBs, or clean 3D-printed enclosures look super cool to me. As long as the screen is centered and the ergonomics are clean, a DIY aesthetic is a plus in my book."

**4. How important is the ability to download new games? Would you jump through hurdles to do so?**
"Very important, that's the whole appeal of an open indie handheld! And yeah, I'm totally fine jumping through some technical hurdles. Flashing firmware via USB, using `esptool`, or dropping `.bin` files onto a microSD card doesn't bother me at all, as long as the steps are clearly written down in a README."

---

## Interviewee 3: Chloe (24, Indie Game Dev & Heavy Gamer)
*Background: Gameplay programmer and technical artist at a small indie collective. Uses Unity and C++; loves narrative RPGs, rhythm games, and hardware game jams.*

### General
**1. What's your favorite game and why?**
"*Outer Wilds* and *Hades*. *Outer Wilds* is my favorite because progression is entirely driven by player knowledge and curiosity rather than arbitrary stat upgrades, it's something only video games can do as a medium. *Hades* is on the flip side: incredible combat flow, reactive storytelling that rewards dying, and immaculate visual and audio feedback."

**2. Where do you like to play games? At home or on the go?**
"Mostly at home, either curled up on the couch with my Steam Deck or at my desktop setup. I do bring my Steam Deck or 3DS when traveling on flights or visiting family, though for daily errands it's a bit too bulky to carry around, so a smaller pocket handheld is actually really appealing."

### Game Dev Platform
**1. What is one feature that you couldn't live without?**
"A clean hardware abstraction layer (HAL) for graphics, timing, and audio. I don't want to write raw SPI display drivers or wrestle with hardware timers just to get a stable 30 or 60 FPS game loop. Give me `update()`, `draw()`, delta time, and a simple way to trigger sound effects without blocking the main thread."

**2. Does working with limits on a device that might have hardware restrictions affect your development?**
"Yes, it forces me to think architecture-first. On a PC, you can be lazy with garbage collection or unoptimized textures. On a constrained microcontroller, I have to plan around fixed memory budgets, lower screen resolutions, and limited inputs right from the design phase. It definitely steers me toward arcade-style, turn-based, or puzzle mechanics rather than heavy physics or complex UI."

**3. What makes or breaks your ability to use documentation?**
"Searchability and an onboarding 'Quickstart' guide. If it takes me more than 30 minutes to set up the toolchain, compile the template project, and get it running on the device, that's a major red flag. Good documentation needs a clear diagram of the system architecture, pinouts/button mappings, and explicit warnings about known gotchas (like 'don't allocate memory inside the draw loop')."

**4. What are some of your hurdles when creating games?**
"Scope management and debugging hardware-specific bugs. When a game crashes on PC, you get a stack trace and a debugger. When a game freezes on an embedded device, sometimes the screen just locks up and you're stuck printing serial logs to figure out if you ran out of SRAM or hit a watchdog timer."

### Physical Product Player Perspective
**1. Do you prefer a horizontal or vertical layout?**
"I'm a big fan of horizontal layouts, like the Game Boy Advance or Switch. Having your hands flanking the screen feels much more balanced, and it keeps your thumbs from blocking the bottom corners of the display or bumping into each other."

**2. How important is the feel of the product?**
"Very important. Tactile feedback is half the "juice" of playing on dedicated hardware instead of a phone screen. When I press a button, I want a distinct, snappy actuation so I know my input registered without looking down. Also, weight distribution matters, if it's too top-heavy, it gets tiring to hold."

**3. How important is the look of the product?**
"Pretty important! Part of the magic of custom handhelds is having a distinct personality. Even with off-the-shelf components, choosing a cohesive color scheme for the buttons and shell, or having a nice bezel around the screen, elevates it from 'class project' to 'a gadget I want to show my friends.'"

**4. How important is the ability to download new games? Would you jump through hurdles to do so?**
"Super important, especially if I want to share my own games with friends or play games from a jam! Personally, I don't mind flashing over USB or using an SD card bootloader. However, if I'm making a game *for* this device and want non-technical friends to play it, the easier the loading process is (like selecting a game from a menu on an SD card or a web flasher in Chrome), the better."

---

## Interviewee 4: Marcus (27, Indie Game Dev & Heavy Gamer)
*Background: Freelance web/game developer who makes retro-inspired arcade games, shmups, and dungeon crawlers in JS/p5.js and C. Big collector of handhelds.*

### General
**1. What's your favorite game and why?**
"*Chrono Trigger* and *Downwell*. *Chrono Trigger* is the gold standard of pacing, sprite work, and music on 16-bit hardware, there's zero filler. *Downwell* is the ultimate minimalist design: it only uses left, right, and one action button (jump/shoot), yet it has more depth and replayability than most AAA games. It proves how much mileage you can get out of three buttons."

**2. Where do you like to play games? At home or on the go?**
"Honestly, 60% on the go or away from my desk, 40% at home. Since I stare at my computer monitor all day for work and game dev, the last thing I want to do at night is sit in the same chair. I love lounging on the porch, waiting for a train, or lying in bed with a small handheld."

### Game Dev Platform
**1. What is one feature that you couldn't live without?**
"Collision detection helpers and a reliable frame-rate limiter/tile renderer. Dealing with flicker-free screen refreshes (like double buffering or dirty rectangles) on small displays can be a nightmare if the platform SDK doesn't handle it for you. Having built-in helpers for drawing bitmap fonts and checking bounding-box collisions saves so much boilerplate."

**2. Does working with limits on a device that might have hardware restrictions affect your development?**
"100%, and it's what makes hardware dev fun. If you tell me I only have 6 buttons, a small screen, and a simple piezo buzzer for audio, I immediately start designing around those constraints, like making single-screen arcade games, grid-based tactics, or high-contrast pixel art that reads well on a tiny display. Constraints breed creativity, provided the core loop runs smoothly."

**3. What makes or breaks your ability to use documentation?**
"Up-to-date starter templates and accurate API references. Nothing breaks trust in documentation faster than following the 'Getting Started' guide, hitting a compiler error on step 2 because a library version changed (like LVGL or Adafruit_GFX updates), and having to dig through forums. A working template repo with a sample game (like Pong or Tic-Tac-Toe) that compiles out-of-the-box is worth a thousand pages of docs."

**4. What are some of your hurdles when creating games?**
"Sound design on limited hardware and UI scaling. Making a piezo buzzer or basic DAC sound pleasing instead of harsh and grating is surprisingly tricky without a little melody/tone sequencing tool. Also, getting text and UI to be readable on a small low-res screen takes a lot of trial and error."

### Physical Product Player Perspective
**1. Do you prefer a horizontal or vertical layout?**
"I actually love vertical layouts for quick, pick-up-and-play arcade games, like the Arduboy or Game Boy Pocket, because they slip into a pocket so easily. But if the device is thick (because of AA batteries or a dev board), a horizontal layout usually feels way better to hold with two hands."

**2. How important is the feel of the product?**
"Crucial. The placement of the buttons relative to where your thumbs naturally pivot makes a huge difference. Also, making sure the battery pack or back cover doesn't rattle when you're mashing buttons is a big deal for immersion."

**3. How important is the look of the product?**
"It's a nice bonus, though functionality comes first for me. I really dig when DIY handhelds embrace what they are, like having a cool silkscreen on the PCB or a chunky retro cassette-futurism case. As long as the screen isn't recessed too deeply where viewing angles get cut off, I'm happy."

**4. How important is the ability to download new games? Would you jump through hurdles to do so?**
"Essential! A handheld with only one hardcoded game becomes a desk ornament after a week. Being able to swap games or have a multi-game launcher menu gives it real longevity. As a dev, I have zero problem jumping through hoops, building from source in PlatformIO/Arduino IDE or flashing partitions, though having a web-based USB flasher or SD card loader makes it 10x more likely I'll swap games frequently."
