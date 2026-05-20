An attempt at making a map_name.txt level merger for Fallout 2's mapper system.<br>
When finished this can be used with .txt map files exported from the mapper (either using ALT+P or the "Create ALL MAP TEXTS" menu option in mapper2.exe).<br><br>
This will allow the user to 
> copy levels from one map_name1.txt file to another map_name2.txt file,<br>
> extract map levels,<br>
> insert map levels,<br>
<br>
and maybe other stuff, dunno yet.

This tool should do the job of 
> extracting a map level from a Fallout 2 "map.txt" file, 
> then generating a new "Q-map.txt" file from the extracted levels.

This can also be used to merge two maps together, making map merges muuuuch easier and less prone to error.
Also, this fixes issues with scripts having overlapping script ID's, causing the mapper to reject them until the ID's
are manually fixed so they no longer overlap.

Use is simple, drag and drop only.
Drag and drop a Fallout 2 "map_name.txt" file onto one side or the other.
Select the level you want to extract from on either side, and the level to extract to in the middle.
Click the arrow for the appropriate side to set the indicator to show which level is extracted where.
Select which map header you want to use (there's a bunch of information in the header that might be necessary depending on the map variables used in the scripts attached to any objects/spatials being exported on a level.
Type in a filename and click "Export".

The Export will automatically add a "Q.txt" to the filename so files aren't over-written.
You should be able to rename this however you want, but be careful not to over-write an old map until you know the new one works.