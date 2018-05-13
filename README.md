# Engine Fixes - Skyrim 64bit

WIP skse64 plugin

### Current Fixes

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

### Optional

There's a replacement for the BSReadWriteLock mutex implementation included (disabled by default). I only really included it because SSE fixes has one, also disabled by default. It may or may not have performance impacts in some situations.
### Future Todo/Things to look into

- large reference overwrite bug https://bethesda.net/community/topic/8825/editing-large-references-causes-lod-to-show-incorrectly-causing-texture-flicker
 
  have done some initial research into this and RNAM records seem to merge fine, there's something happening later on in TESObjectCELL functions that is breaking the reference table. ideally need to create a test worldspace to make this easier to track down.
- disabling TAA causes glitchy save behavior. specifically, hitting quicksave will not actually save the game until a menu is opened (or some other event triggers the update; i've heard entering combat will work). also causes save games to not include a screenshot, and possibly also a bug where the save menu opens multiple times.
- having a large number of plugins in the data folder makes save games appear corrupted. the number includes disabled plugins, and is something over 300 total. may be related to bugs that exist in other (skyrim LE, FO3/NV, etc) games where having excessive disabled plugins will break plugin loading -> https://www.nexusmods.com/skyrim/mods/19556/ presumably this bug also exists in SE and should be fixed
- there's another false save corruption bug that I don't know much about at all, see here -> https://www.nexusmods.com/skyrimspecialedition/articles/342/
- CTD when bethesda.net isn't accessible (or something like this? maybe DNS related instead? not entirely sure, since people fix it by disabling their internet), breaks game for people in eastern europe 
- there's probably more stuff. its skyrim

### Outside the scope of this fix

- broken specular skin shader

thanks:

- meh321 - sse fixes research, skyrim LE bug fixes (ported with permission)
- Nukem - form cache code, additional research for the tree LOD alpha stuff, pointing me at the waterflow timer, tree reflection fix, moral support :^)
- himika - scatter table implementation from libskyrim (LE), plus tons of research function/variable names




