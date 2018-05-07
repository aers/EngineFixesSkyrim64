# Engine Fixes - Skyrim 64bit

WIP skse64 plugin. not release state.

current:

Tree LOD alpha update caching
Form lookup caching
Together these constitute the same fix SSE fixes provides

Double perk apply bug fix
Same as the individual one

Waterflow timescale fix
SE waterflow is tied to a global timer, this global timer updates with the in-game hour of day; timescale causes this to move slower or faster, which makes water flow slower or faster. Fix makes waterflow use a different timer.

todo:

- cleanup
- document fixes

look into:

- disabling TAA causes glitchy save behavior. specifically, hitting quicksave will not actually save the game until a menu is opened (or some other event triggers the update; i've heard entering combat will work). also causes save games to not include a screenshot, and possibly also a bug where the save menu opens multiple times.
- having a large number of plugins in the data folder makes save games appear corrupted ("false save corruption"). the number includes disabled plugins, and is something over 300 total. may be related to bugs that exist in other (skyrim LE, FO3/NV, etc) games where having excessive disabled plugins will break plugin loading -> https://www.nexusmods.com/skyrim/mods/19556/ presumably this bug also exists in SE and should be fixed
- meh's bug fixes - vertical look sensitivity + slow time camera. apparently the lip sync bug is already fixed in SE. need to check if these other two are as well.
- bugged tree reflections; this is fixed by ENB but maybe include it as an optional fix for people who dont run ENB?
- CTD when bethesda.net isn't accessible (or something like this? maybe DNS related instead? not entirely sure, since people fix it by disabling their internet), breaks game for people in eastern europe 
- there's probably more stuff. its skyrim

cant fix dont ask:

- broken character lighting when facing directions other than north (outdoors), some other set of directions (indoors). probably requires patching the shaders and i'm not touching that.

thanks:

meh321 - sse fixes research, skyrim LE bug fixes
Nukem - form cache code, additional research for the tree LOD alpha stuff, pointing me at the waterflow timer, moral support :^)
himika - scatter table implementation from libskyrim (LE), plus tons of research function/variable names




