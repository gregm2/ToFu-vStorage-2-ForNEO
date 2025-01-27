# ToFu for NEOFarmers server-side-only

This is a derivative of the original ToFu vStorage 2.0 mod changed to be server-side-only for NEOFarmers with additions to combat raiding. This will be maintained in the NEOFarmers admintools repository:
https://github.com/NEO-Farmers/AdminTools
with the original fork in:
https://github.com/gregm2/ToFu-vStorage-2-ForNEO

The original ToFu vStorage readme is included below where changes are encouraged to use their GitHub repository rather than unpacking their PBO files.

Configuration for auto-close and admins is the same as the original ToFu vStorage v2 mod, however, no items will be blocked from storage to 
avoid synchronization. Sadly, never-spoiling food is a thing here. Putting a barrel in a barrel will be tested, this will likely 'delete' the
nested barrel and recreate it when the containing barrel is opened again. Notes are normally blocked, but not on vanilla anyways.

What it does:
- When a barrel is closed its contents are serialized to a file and deleted on the server
  - Note: If it has offline items, it'll technically have a rag in it when closed so it can't be moved or poked holes in. When loading contents the rag will be deleted. This is the easiest way for it to show up as non-empty on the client.
- When a barrel is opened, its contents are re-loaded from the file
- When a barrel is closed, it becomes claimed by the closer
- An empty barrel can not be claimed. A user should place it and put an item in it (even a rag) and close it to claim it.
- A claimed barrel may only be opened or damaged by the owner or an admin configured in profiles/ToFuVStorage/VST_Config.json (using SteamIDs)
- Virtual contents stored in separate files should still prevent a barrel from being moved.

CHANGES:
- Blacklist in VST_Config.json is currently ignored. Not sure what would happen if a client thinks an item can go into a barrel and the server does not
- No types XML, for server-side only there can be no non-vanilla objects.
- No missiongameplay code, that's client based and RPCs removed.
- Removed any community framework references as they may not be safe for server side only code
- ActionCloseVStorage and ActionOpenVStorage have been removed to simply change the behavior for vanilla barrels
- No more added actions vstorage open/close/claim, claim is now part of close
- Auto close feature uses native avoid player function rather than get objects at position

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
