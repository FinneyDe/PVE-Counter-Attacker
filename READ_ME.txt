PVE Counter Attacker - By FinKone

This is a modular system that makes use of naming of generic entities to select pre-defined spawn, 
and objective locations for AI to be created, attack, and retreat from.  An objective can be setup
and complete for counter-attacking waves in less than a few minutes once familiar with the systems workings.
---------------------------------------------------------------------------------------
To install the system and use it follow the steps below :
---------------------------------------------------------------------------------------
Open your unlocked map project with Arma Tools.
Use the PVE Counter Attacker mod (this) as a dependancy or simply open with.  
	If using a pre-existing, unlocked(locked map objectives WONT SAVE because they are... locked) map Workbench -> Options -> Unsorted -> Dependencies (+ Button) -> 
	Locate PVE Counter Attacker (possibly located in your C:\Users\<YOUR USER NAME>\Documents\My Games\ArmaReforger\addons)
	NOTE - ARMA TOOLS WILL REQUIRE A RESTART TO DISPLAY THE CHANGES.  You should see "PVE Counter Attacker" folder in resource browser on restart of tools.
	
	If using a new project from scratch, make sure to ADD an EXISITING project (PVE Counter Attacker), then click create new project and
	the PVE Counter Attacker mod should be listed under Dependencies for building the game.  Located most likely from above.

	NOTE : Right clicking and "Open with Addon" vs Declaring PVE Counter Attacker a Dependency-Open with addon will let you use it as an 
	addon for that working session but will not automatically open it on next time.  Its better to declare it as listed UNLESS you understand
	what you are doing.

---------------------------------------------------------------------------------------
You should have a working CONFLICT map (TEST BEFORE INSTALLING PVE Counter Attacker) prior to confirm.
---------------------------------------------------------------------------------------
Steps to install PVE Counter Attacker in on your map once added to your project
---------------------------------------------------------------------------------------


Open your world in the World Editor.

With the World Editor open, navigate to PVE Counter Attacker -> Prefabs -> FIN_SPAWNER_SYSTEM
Drag and drop FIN_ATTACK_WAVE_LOGIC, FIN_COUNTER_WAVE_LOGIC, FIN_Counter_Attack_Manager into your game world.
They should not be located as child (placed inside) of anything beyond a layer (The yellw/blue/red bars 'layers').

RENAME FIN_ATTACK_WAVE_LOGIC if its named differently (Example - When you placed it into the world editor it became
"FIN_ATTACK_WAVE_LOGIC_1".  Do the same for FIN_COUNTER_WAVE_LOGIC, and FIN_Counter_Attack_Manager.  If they are named incorrectly they
might not be found.  If they are not named sometimes (rarely) they struggle to be found.  I use naming to find certain entities in the world - 
the naming IN THE TOP RIGHT SHOULD BE FILLED IN AND MATCH EXACTLY.


If you are adding your own bases - do that now. Once complete with base placing, place FIN_BLANK_REGISTER_PLACE_LAST in the World Editor world.
I USED THIS ENTITY TO MARK THE END OF BASES AND TELL THE SCRIPT TO STAND BY - Its not always needed but I included it for broad situations.
---------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------

Select your first objective. In this example we'll call it "BASE1" in the world editor (A ENEMY BASE THAT IS CAPTURE POINT! NOT STARTING POINTS)
Drag and drop "OBJECTIVE", and "SPAWN_" and place it as a child (inside of the "BASE1") entity.
***************************
IMPORTANT : NAME THE OBJECTIVE "OBJECTIVE" IN THE WORLD EDITOR!(IN THE TOP RIGHT) FOR REASONS PREFAB NAMING DOESNT SAVE/MIGHT NOT REGISTER!!!
IMPORTANT : NAME THE SPAWN_ "SPAWN_" IN THE WORLD EDITOR!(IN THE TOP RIGHT) FOR REASONS PREFAB NAMING DOESNT SAVE/MIGHT NOT REGISTER!!!
***************************
You can now duplicate (Control + D) the SPAWN_ you NAMED instead of dragging new ones in and the naming will update automatically.
Same thing for OBJECTIVES for the next base.  It does not matter if it becomes SPAWN_2 or OBJECTIVE2 at this point.  
ONLY ATTACH ONE OBJECTIVE PER BASE, DUPLICATE AT LEAST 10 SPAWN LOCATIONS (So they dont always attack in the same direciton)

OBJECTIVE is the location they will try to attack (move to).  Place it within capture distance of of BASE1.

When doing other objectives, simply select OBJECTIVE and SPAWN_xx (1-10) from your first objective, and duplicate them (Control + D) for the next.
You can drag and drop them as a child (Inside of bases, think of them as folders) from the first, and simply zero their transforms out (so they are at the next base)
and quickly place them from there.

***************************
With the spawns placed around the objective, and objective placed close you can now test the spawner.
Start your build, F10 -> double click enemy FIA base you altered -> toggle to US flag -> watch.
***************************

You now understand the work flow that is needed for all bases - you can complete a base in less then 1 minute or so using snap to ground functions in editor.

It is recommended to currently place 10 spawns for regular objectives and 15 spawns for hard objectives.
The attack waves scale based on players in the server - they can also be adjusted by a GM.  
NOTE : GM ADJUSTING OF WAVES MAY BREAK THE AUTO SCALER.  GM OPTIONS WERE ADDED AS A DEMAND BY SMALLER PLAYER BASED SERVERS (TO MANY ENEMY)

You can place the infantry as far as you like - keeping in mind that they need time to attack the objective
They have retreat timers that can be adjusted in the World Editor.

Additionally avoid spawning close to rocks if able, and keep in mind the LOW NAVMESH might create issues with AI that need to
handle complex paths away from players.


THE NAMING OF OBJECTIVE AND SPAWN_ Are critical when first starting.  They must start with these names.  ALL CAPS. 


If all is working correctly AI should slowly start (to stay performant) spawning in and approaching the objective.
You can now repeat the process for all other bases.  REMEMBER IF YOU EXPAND YOUR BASES TO DUPLICATE THE FIN_BLANK_REGISTER_PLACE_LAST.
DELETE THE OLD ONE, AND RENAME FIN_BLANK_REGISTER_PLACE_LAST2 to FIN_BLANK_REGISTER_PLACE_LAST.

---------------------------------------------------------------------------------------
LIKELY SOLUTION TO ERRORS :
---------------------------------------------------------------------------------------

--e_Spawns[x] is NULL!  - 
YOU NAMED THE SPAWNS INCORRECTLY.  RENAME THEM SPAWN_xxxx.  It requires a SPAWN_ or it will be ignored.


--On startup NULL ERRORS for each base listed - 
YOU MUST PLACE THE FIN_BLANK_REGISTER_PLACE_LAST PREFAB LAST!  It tells the scripts how/when to start up.

--Cannot spawn team members of group SCR_AIGroup<> @ ENTITY ("SCR_AIGroup") at xyz AIWorld is missing in the world!
You forgot to add a SCR_AIWorld prefab to your world for pathfinding.  Make sure to use the correct map.

--GAME START UP NULLS CAN BE NORMAL

Will expand as users encounter issues.
---------------------------------------------------------------------------------------
ADJUSTABLES OF THE SCRIPT
---------------------------------------------------------------------------------------
FIN_COUNTER_WAVE_LOGIC, FIN_ATTACKER_WAVE_LOGIC, and FIN_Counter_Attack_Manager all have timer options.
You can also set custom trooop types (Default FIA dummys) by dragging and dropping them in the World Editor slots.
This will get expanded in the future.

NOTE : The base line timers need to respect one another and have been setup to do so.  Example being of something that could
generate user error 

Adjusting "Wave Timer" to higher then "Total Timer" can induce no additioanl spawns taking place before timer stops.
Adjusting "Total Timer" to low will not allow AI to reach objectives in time - the AI try to move tactically 
(Dont place far away unless you give them a decent time frame to get there IE long Signal Retreat Timeframe).
"Total Timer" vs "Signal Retreat Timeframe" - Total timer is used to stop sending waves.
Signal Retreat Timeframe is used to clean up AI after a given time frame (if they failed to take the objective).



I know this was long-winded.  It kind of has to be.  Enfusion is amazing and complex.  I'm a novice coder.  I might make videos of the install 
process to help with the setup.  Until then this is whats avaliable.  Once you get used to the system you can populate entire maps within a hour or so.
With it being modular it only needs the CONFLICT bases to function so it should work on all maps that have decent pathfinding.

BASE LINE METRICS FOR COUNTER_WAVE_LOGIC
We found the follow to present a trickle in / flood effect.
MEDIUM WAVE SELECTOR : 2
DEFENDERS VS ATTACKERS CHANGE RANGE : 50
TOTAL TIMER : 900
WAVE TIMER : 90
ROVERS RETREAT TIMEFRAME : 600
OLD AI RETREAT TIMEFRAME : 300
SIGNAL RETREAT TIMEFRAME : 1600


