# ToFu for NEOFarmers server-side-only

This is a derivative of the original ToFu vStorage 2.0 mod changed to be server-side-only for NEOFarmers with additions to combat raiding. This will be maintained in the NEOFarmers admintools repository:
https://github.com/NEO-Farmers/AdminTools
with the original fork in:
https://github.com/gregm2/ToFu-vStorage-2-ForNEO

The original ToFu vStorage readme is included below where changes are encouraged to use their GitHub repository rather than unpacking their PBO files.

Configuration for auto-close and admins is the same as the original ToFu vStorage v2 mod. 
Blacklist items can't be blocked from entering storage due to lack of client sync, so for this version, 
they get kicked out immediately upon detection. 


What it does:
- When a barrel is closed its contents are serialized to a file and deleted from the server
  - Note: If it has offline items, the barrel will actually have a rag in it when closed so it can't be moved or turned into a firebarrel. When loading contents the rag will be deleted. This is the easiest way for it to show up as non-empty on the client.
- When a barrel is opened, its contents are re-loaded from the file
- Claiming/Unclaiming a barrel:
  - Closing an unclaimed barrel with contents, claims it for the closer
  - The owner may close the barrel while empty to unclaim it.
- An empty barrel can not be claimed. A user should place it and put an item in it (even just a rag) and close it to claim it.
- A claimed barrel may only be opened or damaged by the owner or an admin configured in profiles/ToFuVStorage/VST_Config.json (using SteamIDs)
- If a barrel becomes ruined, the contents are loaded so they may be dropped as in normal behavior
- Deletion of barrel objects (i.e. lifetime expiration, destruction, held by player during logoff) will delete virtual storage contents file and metadata file
- Items configured in the config as blacklist items will be immediatey ejected from the barrel upon detection. (often food items so they spoil correctly, blocking barrels in other barrels, or things like written notes that the mod doesn't serialize contents of)
- Blacklist items already in barrels upon introduction of this mod to a running server won't be disturbed.
- If a player wants a rain barrel, they will need to leave it empty and open

CHANGES:
- Blacklist in VST_Config.json doesn't block items from ever entering the barrels, it causes them to be immediately dropped and player notified. (This is due to the lack of the client having knowledge of what is not allowed)
- No types XML, for server-side only there can be no non-vanilla objects, this changes the behavior of normal barrels taking advantage of the fact that open/close happen on the server and get synced to the client.
- No missiongameplay code, RPCs removed as there is nothing to sync with the client beyond base-game behavior
- Removed any community framework references as they may not be safe for server side only code (was in original required_addons)
- ActionCloseVStorage and ActionOpenVStorage have been removed to simply change the behavior for vanilla barrels
- Closing an unclaimed barrel with contents claims it.
- ActionClaimVStorage is replaced by detecting the closing of an empty barrel being interpretted as 'unclaiming'
- No more added actions vstorage open/close/claim, claim is now part of close
- Auto close feature uses native avoid player function rather than get objects at position
- Added configurable user notifications to config file since we have no control of client display
- Added optional 'cool down' to prevent a user from openining and closing a barrel rapidly many times and triggering excessive disk operations.
- Autoclose now longer closes empty barrels that are open so players may have a rain barrel at the cost of storage
- Added ability to block written paper in case we get admin notes working with server-side only changes. (Note: This will not protected any existing items at the time this mod is originally loaded)


# ToFu vStorage 2.0
This mod adds some new barrels to DayZ which store items outside the DayZ database to increase server performance.

This Mod is the open source version of my original mod:
https://steamcommunity.com/sharedfiles/filedetails/?id=2810820431 with some new features like autoclose (thanks inkihh) and a new personal quantum barrel (stores items player based).

You need to create your own textures for the barrels cause the polygon camo texture in the workshop mod is licensed to me.

I will provide no support for the this repro.

DO NOT REPACK THE ORIGINAL MOD, BUILD YOUR OWN ONLY FROM THIS GITHUB

[![CC BY-NC-SA 4.0][cc-by-nc-sa-shield]][cc-by-nc-sa]

This work is licensed under a
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License][cc-by-nc-sa].

[![CC BY-NC-SA 4.0][cc-by-nc-sa-image]][cc-by-nc-sa]

[cc-by-nc-sa]: http://creativecommons.org/licenses/by-nc-sa/4.0/
[cc-by-nc-sa-image]: https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png
[cc-by-nc-sa-shield]: https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg
