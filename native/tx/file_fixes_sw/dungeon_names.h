#pragma once

// There are giant consistency issues with some dungeon names where the same dungeon is called
// multiple things depending on where you look. This is a complete mess, I picked my choices here
// depending on what sounds good and what matches the original JP better but it's kinda arbitrary.
// There are defines so we can easily swap them if we figure out what the actual name for any of
// these is supposed to be...

// this one is consistently Scarlet Battlefield in Switch v1.0.1 now
#define DUNGEON_NAME_m3800 "Scarlet Battlefield"

// the title card calls it "Temple at the Farthest End" but the title card is unused, so go with the
// name that's already used in the text...
#define DUNGEON_NAME_m7500 "Temple of the Farthest Ends"

// the final dungeon maps, this is all a complete mess, completely inconsistent across files. this
// is my best guess. note that m8330 is the 4th area and m8340 is the 3rd area
#define DUNGEON_NAME_m8310 "Pandora - Path of Interstices"
#define DUNGEON_NAME_m8320 "Pandora - Path of Boundaries"
#define DUNGEON_NAME_m8330 "Pandora - Path of Reincarnation"
#define DUNGEON_NAME_m8340 "Pandora - Path of the Afterlife"
// title cards say this in switch 1.0.1 but I think that's still wrong
// #define DUNGEON_NAME_m8310 "Pandora - Path of Interstice"
// #define DUNGEON_NAME_m8320 "Pandora - Path of Division"
// #define DUNGEON_NAME_m8330 "Pandora - Path of Infinity"
// #define DUNGEON_NAME_m8340 "Pandora - Path of Rebirth"

// this was Viridian Path in some versions of the game, it should now consistently be Emerald
// Waterway everywhere
#define DUNGEON_NAME_m9810 "Emerald Waterway"

// this is still Amber Labyrinth in some instances
#define DUNGEON_NAME_m9820 "Amber Waterway"

// this is Frost Demon Path in the title card only, otherwise consistent
#define DUNGEON_NAME_m9830 "Ice Demon Road"
