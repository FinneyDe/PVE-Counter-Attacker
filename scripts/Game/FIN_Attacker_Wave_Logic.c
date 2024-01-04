[EntityEditorProps(category: "FIN_Attacker_Wave_Logic", description: "Accesses stored data from Counter Attack Manager to decide actions.")]
class FIN_Attacker_Wave_LogicClass : GenericEntityClass
{
	
}

//GOALS OF ATTACKER

//CHECK WHAT HAPPENS WHEN AI RECLAIMS A BASE - MAKE SURE TO REMOVE IT FROM THE PLAYER LIST.
//IF I REMOVE IT FROM THE LIST, REPACK LIST.  
//KEEP LAST CAP'D AT END FOR PICKING.

class FIN_Attacker_Wave_Logic : GenericEntity
{

	//Values used to determine current players and scale the AI accordingly.
	PlayerManager m_PlayerManager;
	protected int m_totalPlayers;
	
	static FIN_Attacker_Wave_Logic s_FinAttackerLogic;
	[Attribute("1", uiwidget: UIWidgets.CheckBox, "You can turn off counter-attack (of player CAPTURED bases) completely with this toggle", category: "ATTACK VARIABLES")];
	protected bool sendAttackWaves;
	static bool instanceCheck = true;
	
	[Attribute("9", uiwidget: UIWidgets.CheckBox, "Minimum amount of players required online to attack", category: "ATTACK VARIABLES")];
	protected int minPlayersRequired;
	
	//Keeping track of time
	static float m_counterAttackTimer;
	//Current testing timer - set to random range of 10-45 minutes.
	//DO not allow active while WaveLogic is running.
	//[Attribute(defvalue: "600", uiwidget: UIWidgets.Range, desc: "What random random range of time should we wait before sending a counter-attack wave if no base has been taken in awhile?", params: "600 1500")]
	static float CounterAttackTimer;
	
	//Store the objective location (base)
	protected IEntity e_Objective;
	//Obtain the list of spawns to attack the base from locations
	ref protected array <IEntity> e_Spawns = new array<IEntity>();
	//Object to fall back to - typically a spawn point.
	protected IEntity retreatObjective;
	//used to keep track of the AI we've spawned
	ref protected array <IEntity> e_Enemies = new array<IEntity>();

	
	//A list of US bases.  We will always access the LAST 2 form the list.  Maybe last 3.
	static ref array<IEntity> e_playerBases = new array<IEntity>(); 
	//the base we picked to attack.
	protected IEntity selectedBase;
	
	//Left to false for testing
	static bool arePlayersAttacking;

	protected IEntity Counter_Wave_Logic;

	//MIN = 600
	//Time the AI should move BACK to current base (if players didnt take another base) to make sure they dont over-run or stay at players base....
	[Attribute(defvalue: "2700", category: "ATTACK VARIABLES", uiwidget: UIWidgets.Slider, desc: "MIN ATTACK TIMER in seconds (used to make a time range thats random with MAX) - This sets the SOONEST attack waves can go for objectives you've captured that ARE NOT a main base", params: "30 3000 1")]
	protected float userMinAttackTimer;
	
	//MAX = 3001
	//Time the AI should move BACK to current base (if players didnt take another base) to make sure they dont over-run or stay at players base....
	[Attribute(defvalue: "3600", category: "ATTACK VARIABLES", uiwidget: UIWidgets.Slider, desc: "MAX ATTACK TIMER in seconds (used to make a time range thats random with MIN) - This sets the SOONEST attack waves can go for objectives you've captured that ARE NOT a main base", params: "35 7200 1")]
	protected float userMaxAttackTimer;
	
	//TIMERS FOR ATTACK TIME FRAME
	//TODO - THESE VALUES NEED TO BE ACCESSED VIA FIN_COUNTER_WAVE_LOGIC TO NOT OVERRIDE THE USERS INPUT FROM WORLD EDITOR!!!
	static float minAttackTimer;
	static float maxAttackTimer;
		
	//TODO BEFORE FINAL PACKAGE THESE NEED TO BE SET TO PREFAB ARMA DEFAULTS!!
	//Standard rifle squad to spawn
	[Attribute("{CE41AF625D05D0F0}Prefabs/Groups/INDFOR/Group_FIA_RifleSquad.et", category: "TROOP TYPES", desc: "Normal patrol squad to attack base")]
	protected ResourceName m_RegularRfileSquad;
	[Attribute("{6307F42403E9B8A4}Prefabs/Groups/INDFOR/Group_FIA_SharpshooterTeam.et", category: "TROOP TYPES", desc: "Sharpshooters patrol squad to attack base")]
	protected ResourceName m_SharpshootersSquad;
	
	[Attribute("{CE41AF625D05D0F0}Prefabs/Groups/INDFOR/Group_FIA_RifleSquad.et", category: "TROOP TYPES", desc: "Higher skilled - set your own custom AI if you'd like or use REGULAR HERE ALSO")]
	protected ResourceName m_HIGH_SKILL_RegularRifleSquad;
	[Attribute("{CE41AF625D05D0F0}Prefabs/Groups/INDFOR/Group_FIA_RifleSquad.et", category: "TROOP TYPES", desc: "Higher skilled - set your own custom AI if you'd like or use REGULAR HERE ALSO")]
	protected ResourceName m_HIGH_SKILL_RegularRifleSquadAT;
	
	//Movement names as quick ref will fill over time to reduce footwork.
	string movement_PatrolH = "{FBA8DC8FDA0E770D}Prefabs/AI/Waypoints/AIWaypoint_Patrol_Hierarchy.et";
	string movement_SearchNDestroy = "{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et";
	string movement_Attack = "{1B0E3436C30FA211}Prefabs/AI/Waypoints/AIWaypoint_Attack.et";
	string movement_Defend = "{AAE8882E0DE0761A}Prefabs/AI/Waypoints/AIWaypoint_Defend_Hierarchy.et";


	//-------------------------------------------------------------------------------------------------
	
	//Constructor
	//------------------------------------------------------------------------------------------------
	void FIN_Attacker_Wave_Logic(IEntitySource src, IEntity parent)
	{
		arePlayersAttacking = false;
		minAttackTimer = userMinAttackTimer;
		maxAttackTimer = userMaxAttackTimer;
		instanceCheck = sendAttackWaves;
		//Sets the counter attacker to a random time frame between 10m and 25m (in seconds)
		int rndRange = Math.RandomInt(userMinAttackTimer, userMaxAttackTimer);
		CounterAttackTimer = rndRange;
		Print("[FIN_ATTACKER_WAVE_LOGIC] START UP - Counter attack timer : " + CounterAttackTimer, LogLevel.ERROR);
		
		SetFlags(EntityFlags.ACTIVE | EntityFlags.NO_LINK, false);
		SetEventMask(EntityEvent.FRAME);
	}
	
	//Default Destructor
	//------------------------------------------------------------------------------------------------
	void ~FIN_Attacker_Wave_Logic()
	{
	
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		
		//Does the user even want attack waves at all?
		if(sendAttackWaves)
		{
			//Print(arePlayersAttacking + m_counterAttackTimer + CounterAttackTimer, LogLevel.ERROR);
			//If players are not attacking, then we can reduce our timer.
			//This main body logic will be controlled inside WAVE LOGIC to determine if attacks are happening.
			if(!arePlayersAttacking)
			{
				m_counterAttackTimer += timeSlice;
				if (m_counterAttackTimer > CounterAttackTimer)
				{
					//Check the players on the server to determine if we have enough for AI-Counter attacks.
					m_PlayerManager = PlayerManager.Cast(GetGame().GetPlayerManager());		
					m_totalPlayers = m_PlayerManager.GetPlayerCount();	
					
					if(m_totalPlayers > minPlayersRequired)
					{
						//Check our base list the players have, take from the end of list to prevent attacking bases deep in player terrority.
						int startIndex = Math.Max(0, e_playerBases.Count() - 2);
						ref array<IEntity> lastBases = new array<IEntity>();
						
						for (int i = startIndex; i < e_playerBases.Count(); i++)
						{
							lastBases.Insert(e_playerBases[i]);
						}
						
						int randomIndex = Math.RandomInt(0, lastBases.Count());
						if(lastBases.Count() > 0)
						{
							selectedBase = lastBases[randomIndex];
						}
						else
						{
							//The list is empty to just reset the timer and attempt later.
							//This script is controlled also by FIN_COUNTER_WAVE_LOGIC to turn it off when counter attacks are taking place.
							Print("[Attacker_Wave_Logic] lastBases array EMPTY!  New game?  Restart timer and check later.", LogLevel.ERROR);
		
							int rndRange = Math.RandomInt(userMinAttackTimer, userMaxAttackTimer);	
							m_counterAttackTimer = 0;
							CounterAttackTimer = rndRange;
							return;
						}
									
						
						Print("[Attacker_Wave_Logic] Attack Trigger BASE LIST : ", LogLevel.ERROR);
						foreach (IEntity base : e_playerBases)
						{
							Print(base.GetName(), LogLevel.ERROR);
						}
						
						//COMMENTED OUT FOR NOW - TODO
						//MAKE SURE WE HAVE A METHOD TO COUNT DOWN AND TOGGLE THIS BACK AFTER A TIME FRAME.
						//We are attacking dont select again.  Shuts down checks to start the attack.
						//arePlayersAttacking = true;
						//Call method to launch attack.
						LaunchAttack();	
						
						//TODO ----- PIC RANDOM BASE FEED IT THE PICK
						//THIS WORKS - HOWEVER, THE GAME SHOULD HANDLE THIS ON ITS OWN.
						/*
						SCR_CampaignMilitaryBaseComponent notifications = SCR_CampaignMilitaryBaseComponent.Cast(selectedBase.FindComponent(SCR_CampaignMilitaryBaseComponent));	
						if(notifications != null)
						{				
							notifications.GetCampaignFaction().SendHQMessage(SCR_ERadioMsg.BASE_UNDER_ATTACK, notifications.GetCallsign());
							//Print("[Wave Logic - EOnFrame] Hooked notifications", LogLevel.ERROR);				
						}
						else
						{
							Print("[Attack_Wave_Logic - EOnFrame] Unable to locate notifications SCR component", LogLevel.ERROR);
						}			
						*/
						//GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.myTestGui_ID);
						m_counterAttackTimer = 0;	
					}
					else
					{
						//Not enough players - resetting timer for attacks to check again.
						m_counterAttackTimer = 0;
						Print("[Attack_Wave_Logic - EOnFrame] Min number of players not present - Timer reset - TOTAL PLAYERS : " + m_totalPlayers + "Timer : " + CounterAttackTimer, LogLevel.ERROR);
					}					
				}
		
				
			}	
		}
	}
	//Add more dynamic way to spawn different troops for now its platoon hq.
	 void SpawnAIAt(IEntity spawnPoint)
	{
		int rndRoll = Math.RandomInt(5000, 20000);
		 
		 //Print("Spawning AI at: " + spawnPoint.GetName(), LogLevel.ERROR);	
		GetGame().GetCallqueue().CallLater(AISpawner, rndRoll, false, m_RegularRfileSquad, spawnPoint.GetOrigin(), movement_PatrolH, e_Objective.GetOrigin());
		
	
	}
	//Code used from Rabid Squirrel Gaming to learn.  Found useful so kept in case you are looking at this.
	//Rabid Squirrel On YouTube.
	//https://www.youtube.com/watch?v=ihCoPBXQrXA
	void AISpawner(string spawnGroup, vector spawnPosition, string waypointType, vector waypointPosition)
	{
		
				
		
		
		//ALL THOS THIS COULD BE WRITTEN INTO A METHOD - IM BEING LAZY TODAY AND COPY AND PASTING DATA
			
		int rndRoll = Math.RandomInt(1, 100);
		
		//Rare grp roll
		if (rndRoll <= 5)
		{
			int rndSelector = Math.RandomInt(1,100);
			
			if( rndSelector	<= 24)
			{
				Resource sqdHighSkill = GenerateAndValidateResource(m_HIGH_SKILL_RegularRifleSquad);
				if (!sqdHighSkill)
				{
					Print(("[AI Spawner] Unable able to load resource for the spawn group: " + m_HIGH_SKILL_RegularRifleSquad), LogLevel.ERROR);
					return;
				}
				// Generate the spawn parameters and spawn the group 
				SCR_AIGroup group;
				//Print("AT SMALL SPAWNED", LogLevel.ERROR);
			    // 5% chance (1 to 5) to spawn fia_standard_4man_at
			    group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(sqdHighSkill, null, GenerateSpawnParameters(spawnPosition)));
				//Add AI group to array to keep track of them.
				e_Enemies.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				Print("High_Skill Squad Spawned - ", LogLevel.ERROR);
				// Create a waypoint for this group
				CreateWaypoint(group, waypointType, waypointPosition);
			}
			if( rndSelector > 25 && rndSelector <= 49)
			{
				Resource sharpShooters = GenerateAndValidateResource(m_SharpshootersSquad);
				if (!sharpShooters)
				{
					Print(("[AI Spawner] Unable able to load resource for the spawn group: " + spawnGroup), LogLevel.ERROR);
					return;
				}
				// Generate the spawn parameters and spawn the group 
				SCR_AIGroup group;
				//Print("AT SMALL SPAWNED", LogLevel.ERROR);
			    // 5% chance (1 to 5) to spawn fia_standard_4man_at
			    group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(sharpShooters, null, GenerateSpawnParameters(spawnPosition)));
				//Add AI group to array to keep track of them.
				e_Enemies.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				Print("High_Skill Sharpshooters Spawned - ", LogLevel.ERROR);
				// Create a waypoint for this group
				CreateWaypoint(group, waypointType, waypointPosition);
			}
			if( rndSelector > 50 && rndSelector <= 74)
			{
				Resource sqdATHighSkill = GenerateAndValidateResource(m_HIGH_SKILL_RegularRifleSquadAT);
				if (!sqdATHighSkill)
				{
					Print(("[AI Spawner] Unable able to load resource for the spawn group: " + spawnGroup), LogLevel.ERROR);
					return;
				}
				// Generate the spawn parameters and spawn the group 
				SCR_AIGroup group;
				//Print("AT SMALL SPAWNED", LogLevel.ERROR);
			    // 5% chance (1 to 5) to spawn fia_standard_4man_at
			    group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(sqdATHighSkill, null, GenerateSpawnParameters(spawnPosition)));
				//Add AI group to array to keep track of them.
				e_Enemies.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				Print("High_Skill Squad With AT Spawned - ", LogLevel.ERROR);
				// Create a waypoint for this group
				CreateWaypoint(group, waypointType, waypointPosition);
			}
			if( rndSelector > 74 && rndSelector <= 100)
			{
				Resource sqdHighSkill = GenerateAndValidateResource(m_HIGH_SKILL_RegularRifleSquad);
				if (!sqdHighSkill)
				{
					Print(("[AI Spawner] Unable able to load resource for the spawn group: " + spawnGroup), LogLevel.ERROR);
					return;
				}
				// Generate the spawn parameters and spawn the group 
				SCR_AIGroup group;
				//Print("AT SMALL SPAWNED", LogLevel.ERROR);
			    // 5% chance (1 to 5) to spawn fia_standard_4man_at
			    group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(sqdHighSkill, null, GenerateSpawnParameters(spawnPosition)));
				//Add AI group to array to keep track of them.
				e_Enemies.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				Print("High_Skill Squad Spawned - ", LogLevel.ERROR);
				// Create a waypoint for this group
				CreateWaypoint(group, waypointType, waypointPosition);
			}
			
		}
		//Semi rare roll
		else if (rndRoll > 5 && rndRoll <= 50)
		{
			int rndSelector = Math.RandomInt(1,100);
			
			if( rndSelector	<= 24)
			{
				Resource sqdHighSkill = GenerateAndValidateResource(m_HIGH_SKILL_RegularRifleSquad);
				if (!sqdHighSkill)
				{
					Print(("[AI Spawner] Unable able to load resource for the spawn group: " + spawnGroup), LogLevel.ERROR);
					return;
				}
				// Generate the spawn parameters and spawn the group 
				SCR_AIGroup group;
				//Print("AT SMALL SPAWNED", LogLevel.ERROR);
			    // 5% chance (1 to 5) to spawn fia_standard_4man_at
			    group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(sqdHighSkill, null, GenerateSpawnParameters(spawnPosition)));
				//Add AI group to array to keep track of them.
				e_Enemies.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				Print("High_Skill Squad Spawned - ", LogLevel.ERROR);
				// Create a waypoint for this group
				CreateWaypoint(group, waypointType, waypointPosition);
			}
			if( rndSelector > 25 && rndSelector <= 49)
			{
				Resource sharpShooters = GenerateAndValidateResource(m_SharpshootersSquad);
				if (!sharpShooters)
				{
					Print(("[AI Spawner] Unable able to load resource for the spawn group: " + spawnGroup), LogLevel.ERROR);
					return;
				}
				// Generate the spawn parameters and spawn the group 
				SCR_AIGroup group;
				//Print("AT SMALL SPAWNED", LogLevel.ERROR);
			    // 5% chance (1 to 5) to spawn fia_standard_4man_at
			    group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(sharpShooters, null, GenerateSpawnParameters(spawnPosition)));
				//Add AI group to array to keep track of them.
				e_Enemies.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				Print("High_Skill Sharpshooters Spawned - ", LogLevel.ERROR);
				// Create a waypoint for this group
				CreateWaypoint(group, waypointType, waypointPosition);
			}
			if( rndSelector > 50 && rndSelector <= 74)
			{								
				Resource sqdATHighSkill = GenerateAndValidateResource(m_HIGH_SKILL_RegularRifleSquadAT);
				if (!sqdATHighSkill)
				{
					Print(("[AI Spawner] Unable able to load resource for the spawn group: " + spawnGroup), LogLevel.ERROR);
					return;
				}
				// Generate the spawn parameters and spawn the group 
				SCR_AIGroup group;
				//Print("AT SMALL SPAWNED", LogLevel.ERROR);
			    // 5% chance (1 to 5) to spawn fia_standard_4man_at
			    group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(sqdATHighSkill, null, GenerateSpawnParameters(spawnPosition)));
				//Add AI group to array to keep track of them.
				e_Enemies.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				Print("High_Skill Squad With AT Spawned - ", LogLevel.ERROR);
				// Create a waypoint for this group
				CreateWaypoint(group, waypointType, waypointPosition);
				
			}
			if( rndSelector > 74 && rndSelector <= 100)
			{
				Resource sqdHighSkill = GenerateAndValidateResource(m_HIGH_SKILL_RegularRifleSquad);
				if (!sqdHighSkill)
				{
					Print(("[AI Spawner] Unable able to load resource for the spawn group: " + spawnGroup), LogLevel.ERROR);
					return;
				}
				// Generate the spawn parameters and spawn the group 
				SCR_AIGroup group;
				//Print("AT SMALL SPAWNED", LogLevel.ERROR);
			    // 5% chance (1 to 5) to spawn fia_standard_4man_at
			    group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(sqdHighSkill, null, GenerateSpawnParameters(spawnPosition)));
				//Add AI group to array to keep track of them.
				e_Enemies.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				Print("High_Skill Squad Spawned - ", LogLevel.ERROR);
				// Create a waypoint for this group
				CreateWaypoint(group, waypointType, waypointPosition);
			}
			
		}
		//Common
		else
		{
			Resource resource = GenerateAndValidateResource(m_RegularRfileSquad);
			if (!resource)
			{
					Print(("[AI Spawner] Unable able to load resource for the spawn group: " + spawnGroup), LogLevel.ERROR);
					return;
			}
			// Generate the spawn parameters and spawn the group 
			SCR_AIGroup group;
			//Print("REG SQD SPAWNED", LogLevel.ERROR);
		    // Remaining 84% chance (17 to 100) to spawn the typical "resource"
		    group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(spawnPosition)));
			//Add AI group to array to keep track of them.
			e_Enemies.Insert(group);
			if (!group)
			{
				Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
				return;
			}
			Print("Normal Skill Base Squad Spawned - ", LogLevel.ERROR);
			// Create a waypoint for this group
			CreateWaypoint(group, waypointType, waypointPosition);
			
		}
		
		
		
	}
	
	void LaunchAttack()
	{
		    //Clear our list we are about to popualte it and use it
			e_Spawns.Clear();
			array<IEntity> allChildren = new array<IEntity>();
		
		
			//NEED TO SIFT THE LAST THREE OF THE LIST AND PICK ONE - IF IT HAS THREE, IF NOT, TWO, IF NOT JUST TAKE ONE.		
			IEntity child = selectedBase.GetChildren();
			
			while(child)
			{
			    // Make sure child is not null before trying to access its name
			    string childName = child.GetName();
			    
				
			    if (childName.StartsWith("OBJECTIVE"))
			    {
			        e_Objective = child;
			        //Print("OBJECTIVE Child Found - " + childName, LogLevel.ERROR);		        
			    }
			   else
				{
				    // Check for SPAWN_ entities
					//I was lazy in my code and decided to check up to 600 numbers to prevent dupes.
					//If your map is insane and you need more then 600 spawns, increase the number...
				   for (int i = 1; i <= 600; i++)
					{
					    if (childName == "SPAWN_" + i)
					    {
					        e_Spawns.Insert(child); // Directly insert the child
					        //Print("Spawn Child Found - " + childName, LogLevel.ERROR);
					    }
					}
				}
			
			    allChildren.Insert(child);
			    //Print("CHILD -" + childName , LogLevel.ERROR);			
			    child = child.GetSibling(); // GetSibling could return null if there are no more siblings
				
				
			
			}
					
			if(e_Spawns.Count() > 0 && e_Spawns[0] != null)
			{
				retreatObjective = e_Spawns[0];
			}
			else	
			{
					Print("[CheckBase] e_Spawns[0] is null!", LogLevel.ERROR);
			}
		 //Print("COUNTER ATTACK DECIDER MAIN " + m_sendingWaves + e_Enemies, LogLevel.ERROR);    
		
		   
		    // Decide what attack type (single value for testing - 1-3 for finished)
		    int attackType = 3;
		    
		    // Print the entire list of spawn points
		    Print("[FIN_ATTACK_WAVE_LOGIC] Listing all spawn points:", LogLevel.ERROR);
		    foreach (IEntity spawnPoint : e_Spawns)
		    {
		        if (spawnPoint != null)
		        {
		            Print("Spawn Point: " + spawnPoint.GetName(), LogLevel.ERROR);
		        }
		        else
		        {
		            Print("Spawn Point: null", LogLevel.ERROR);
		        }
		    }
		
		    // Check for Hard attack type
		    if (attackType == 3) // Hard
		    {
						
				//TODO sift the list based on hard mode and reduce over all sent waves.  Randomly pull from the list of spawns and use not all.			
		        Print("[FIN_ATTACK_WAVE_LOGIC] ATTACK_WAVE_LOGIC DECIDER - Attacking : " + selectedBase, LogLevel.ERROR);
		        foreach (IEntity spawnPoint : e_Spawns)
		        {
		            // Check if spawnPoint is not null
		            if (spawnPoint != null)
		            {					
					
			            //Print("SPAWNNER TRIGGER", LogLevel.ERROR);
						//WORKS TO SPAWN GROUPS>  INTERESTED IN AMBIENT STYLE SPAWNS TO CAPTURE THEIR CLEANUP AND USE RESPAWNS SO I DONT HAVE TO WRITE MY OWN.
						//THree is heavy but silly cuz they stack.  Timer issue?
						SpawnAIAt(spawnPoint);
						//attackersActive = true;
						//m_sendingWaves = true;	
							
		           }		
				}						
		    }		
	}
	
	protected void CreateWaypoint(SCR_AIGroup group, string waypointType, vector waypointPosition)
	{
		// Generate the resource 
		Resource resource = GenerateAndValidateResource(waypointType);
		
		Resource resource2 = GenerateAndValidateResource(movement_Defend);
		
		if (!resource)
		{
			Print(("[Create Waypoint] Unable able to load resource for the waypoint: " + waypointType), LogLevel.ERROR);
			return;
		}
		
	
		//Crude 50/50 split on attacks (patrols) v defender logic
		int rndRoll = Math.RandomInt(1, 100);
		if (rndRoll > 50)
		{
			AIWaypoint waypointD = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resource2, null, GenerateSpawnParameters(waypointPosition)));
			if (!waypointD)
			{
				Print("[Create Waypoint] Unable to create waypoint!", LogLevel.ERROR);
				return;
			}
			//Print("DEFENDER GROUP", LogLevel.ERROR);
			group.AddWaypoint(waypointD);
			//ai_EwayPoints.Insert(waypointD);
		}
		else
		{
			AIWaypoint waypoint = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(waypointPosition)));
			
			if (!waypoint)
			{
				Print("[Create Waypoint] Unable to create waypoint!", LogLevel.ERROR);
				return;
			}
			group.AddWaypoint(waypoint);
			//ai_EwayPoints.Insert(waypoint);
			//Print("ROVER GROUPP", LogLevel.ERROR);
		}
	}
	
	
	protected Resource GenerateAndValidateResource(string resourceToLoad)
	{
		// Load the resource 
		Resource resource = Resource.Load(resourceToLoad);
		
		// Validate the prefab and show an appropriate error if invalid 
		if (!resource.IsValid())
		{
			Print(("[Generate And Validate Resource] Resource is invalid: " + resourceToLoad), LogLevel.ERROR);
			return null;
		}
		
		// Return the resource 
		return resource;
	}
	
	
	protected EntitySpawnParams GenerateSpawnParameters(vector position)
	{
		// Create a new set of spawn parameters 
		EntitySpawnParams params = EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		
		// Assign the position to those parameters 
		params.Transform[3] = position;
		
		// Return this set of spawn parameters
		return params;
	}
}