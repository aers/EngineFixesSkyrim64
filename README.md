# Engine Fixes - Skyrim 64bit

WIP skse64 plugin

### Current Patches

1. Tree LOD Alpha & Generic Form Lookup Caching

   Every frame, a function is called to update the alpha of tree LODs, in order to create a stipple fade effect when moving towards or away from trees. For unknown reasons (haven't fully researched this yet), the structure containing the list of tree object reference formids to update on every frame uses the local formids of their plugin source, that is formids that don't account for the current game load order. In an attempt to find the correct tree object reference, the function loops through every possible plugin formid (example: it wants to find local formid 000010, so it searches 00000010, 01000010, etc through your entire load order). While it does break the loop after finding a valid form and the majority of tree references are in the main game plugins, there is a second problem that causes a major performance hit in some areas.

   In the vanilla game, standing in Riften will cause ~1000 tree object references to be looked up every frame to update their LOD alphas. Every single one of these lookups fails (the form is unloaded). This failure condition means the function will look through your entire load order for every single one of these local formids, every single frame. With the vanilla game's 5 plugins, this is 5000 formid lookups per frame. If you have 100 plugins, the number quickly balloons to 100,000. This is why your FPS will tank in Riften (and some other problem areas) as you add more plugins.

   Its unknown specifically why the game is attempting to lookup so many unloaded object references in certain areas; we haven't done much further research into it. Speculation is that these are object references of the parent worldspace used to render tree LODs outside the bounds of the current worldspace - that is if you look out over the walls of Riften there's a world out there. The fact that this performance hit doesn't exist in Skyrim LE suggests it is related to changes in how SSE handles LODs (large reference grid, etc) but no idea at this point.

   This fix hooks the tree LOD alpha update function to cache the lookups so that after the first frame there will only be one lookup per formid. For more details check the implementation. There's also a hook to cache form id lookups in general, because looking through all the forms loaded into the global form scatter table can be slow.

   [SSE Fixes](https://www.nexusmods.com/skyrimspecialedition/mods/10547/) also fixes this problem. Don't use them together.

2. Double Perk Apply

   When loading a save game perk effects are applied to NPCs in the current cell twice. This fix, and the next two, are ported from [bug fixes](https://www.nexusmods.com/skyrim/mods/76747). 

   The fix simply marks the start of the perk apply loop and calls a function to clear perk effects at the start of the loop, so no matter how many times the game attempts to apply perk effects, only the last one truly "sticks".

3. Slow Time Camera Movement

   Camera movement when standing still is tied to a global timer that is roughly pegged to your framerate, so the game knows how far to move the camera on each frame. When a slow time effect occurs, this frame timer is artifically slowed, so game functions dependent on it move at a slower rate than your framerate. The fix pegs camera movement to the secondary frame timer, which doesn't slow down with slow time effects.

4. Stationary Vertical Sensitivity

   Similar to above, camera movement is tied to a frame timer; the fix sets it to a constant instead.

5. Waterflow Timer

   SSE added shader-based waterflow effects. The shader needs to know how far to advance waterflow on each frame, and uses a global timer to do so. The global timer is based on the current in-game GameHour, so if you adjust the in-game timescale, causing GameHour to move faster or slower, waterflow also moves faster or slower. The fix makes waterflow use a custom independent timer that is not affected by the game's timescale. You can configure what emulated timescale to use if you don't like the default waterflow speed.

6. Tree Reflections

	Tree LOD reflections don't properly use the alpha channel and show as large black boxes. While ENB fixes this as well, its included here for people who don't use ENB. I don't know the exact specifics on this fix, it was given to me by Nukem. Ideally this would be fixed in the shader itself, not the shader class.

7. Snow Sparkles

    There are objects whose material shaders are treated as snow shaders even though they aren't flagged as such in the appropriate .nifs. Either the loader or some other part of the game doesn't properly account for this (not fully researched), so you end up with BSLightingShaderMaterial objects where the game expects there to be BSLightingShaderMaterialSnow objects. This causes the BSLightingShader::SetupMaterial case for snow objects to read out of bounds memory. Due to vanilla Skyrim's memory allocation patterns, this won't usually cause a crash, but instead causes the sparkle parameters to be populated with invalid/unknown data. When using the memory manager patch, it can and will cause CTDs due to out of bounds memory reads instead. The fix checks the objects to verify they are the appropriate type; if they are incorrect, it sets the sparkle params to the default values that are present in the BSLightingShaderMaterialSnow constructor (1.0f for all four) instead of trying to read them.    

8. Sound Category Volume Saving

    The game only saves the volume for 8 non-Master sound categories that are displayed in the Settings Menu; patch will save all to a separate ini, allowing mods to add new sound categories.

9. Max Stdio

    Per https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setmaxstdio the max number of file handles the C stdio level can hold open by default is 512. Since SSE keeps an open file handle for all your plugins (including ones that aren't part of your load order) as well as some other important files, its possible to hit this limit on a heavy load order. Once you hit the limit all calls to fopen_s will start to fail, which means the game can no longer read new files. This causes, among other things, save game loads to report corrupted as the game is no longer capable of reading the save game files, a bug known as "false save corruption". The patch replaces calls to crt stdio with a custom implementation that calls Win32 IO directly, raising the handle limit to that of those APIs.

10. RaceSex Menu Precache Disabler

    Same as LE Racemenu Precache Killer. I'm not convinced this is necessary on SE but some people who must have thousands of hairs report crashes and its easy to include, so here it is. Patch just disables the cache populate/clear functions. Disabled by default.

11. Saves

    When you queue a save event the game will request a screenshot in order to bake it into the save. With TAA or DOF (or both) disabled, this request won't process until a menu is opened. In addition, with TAA disabled the render target the screenshot is copied from will be blank, so all saves end up with blank screenshots. This fixes both bugs. Has two methods with differnt side effects, see ini for details. Auto disables if TAA+game DOF are enabled. Credit to Nukem for researching the renderer part as always.

    Also includes a patch that turns quicksaves into regular saves, like https://www.nexusmods.com/skyrim/mods/82951

12. SKSE64 Cosave Cleaner

    SKSE64 cosaves aren't deleted for quicksaves; this is just a tool that runs on launch and cleans them. This is mostly relevant due to a SkyUI(?) bug that causes some glitches in the save/load menu if you have a lot of files in your save folder. Thx Kole6738 for contrib.

### Optional

This stuff is marked optional because performance may vary on different hardware. If you're having stutter/freezing issues, try one or both. The memory manager fix requires the use of a skse64 preloader and I put the mutex one in there too Just Because.

1. Memory Manager

   This disables Skyrim's built-in memory manager for its two largest heaps. This is the commonly named "OS allocators" fix. Also replaces system memory calls with jenalloc. REQUIRES Snow Sparkle fix.

   Currently temporarily clamps BGSShaderParticleGeometryData's ParticleDensity to 10.0 as values above that cause an out-of-bounds memory read error. Expanding the particle shader to accept higher particle density is possible but it may be intended for this value to clamp at 10 even though some weathers use values far exceeding it.

2. BSReadWriteLock

   Replaces BSReadWriteLock mutex implementation with a custom one designed to be faster for reads.


### Future Todo/Things to look into

- large reference overwrite bug https://bethesda.net/community/topic/8825/editing-large-references-causes-lod-to-show-incorrectly-causing-texture-flicker
 
  have done some initial research into this and RNAM records seem to merge fine, there's something happening later on in TESObjectCELL functions that is breaking the reference table. ideally need to create a test worldspace to make this easier to track down.
- terrain LOD breaks sometimes. don't have high hopes for tracking this one down because even though I've personally seen it happen its not possible to replicate it 100% of the time
- CTD when bethesda.net isn't accessible (or something like this? maybe DNS related instead? not entirely sure, since people fix it by disabling their internet), breaks game for people in eastern europe 
- enchantment reload fix from LE. LE fix saves to cosave, maybe research exactly what is wrong with enchantment saving or just be lazy and copy LE version
- add crash handler
- training costs too high cause training to fail
- there's probably more stuff. its skyrim

### Outside the scope of this fix

- ~~broken specular skin shader~~ **Coming Soon™**

thanks:

- meh321 - sse fixes research, skyrim LE bug fixes (ported with permission)
- Nukem9 - form cache code, additional research for the tree LOD alpha stuff, pointing me at the waterflow timer, tree reflection fix, actually just every fix, moral support :^)
- himika - scatter table implementation from libskyrim (LE), plus tons of research function/variable names
- Kole6738 - cosave cleaner 



