[EntityEditorProps(category: "FIN_Counter_Wave_Logic", description: "Accesses stored data from Counter Attack Manager to decide actions.")]
class FIN_Counter_Wave_LogicClass : GenericEntityClass
{
	
}

class FIN_Counter_Wave_Logic : GenericEntity
{
	
	//TODO
	//LONG-TERM DEALING WITH AI THAT TOOK BACK A BASE.  
	//START A TIMER TO TRACK WEHN DEFENDERS RECAPTURE A LOCATION FROM US
	//CLEAR THEM FROM THIS - ADD THEM TO FIN_LONG_TERM_AI_MANAGER
	//START TIMER IN LONG TERM MANAGER - MOVE THEM BACK TO THE OBJECTIVE?  HAND OVER?  
	//SAVE LOCATION, DETROY THAT PERSISTENT ENTITY
	//SPAWN A AMBIENT PATROL IN ITS PLACE
	//THIS WAY IF PLAYERS BY-PASS THAT OBJECTIVE WE DON'T WASTE AI POWER (THINK IF AI CAPTURES MULTIPLE LOCATIONS BACK)
	
	//Sound FILES
	[Attribute(desc: "Sound effect played at specific events")]
    protected string m_testWav;

	
	//Values used to determine current players and scale the AI accordingly.
	PlayerManager m_PlayerManager;
	protected int m_totalPlayers;
	
	//Adjsutor for how many spawns to select to tune difficulty.
	static int EasyWaveSelector;//determines role of responding units.  Defenders when complete will occupy the base, attackers will respond to fires and move to contact.
	[Attribute(defvalue: "2", category: "ATTACK VARIABLES", uiwidget: UIWidgets.Slider, desc: "Determines the amount of spawns to skip.  Skip more, less dudes show up", params: "1 10 1")]
	protected int MediumWaveSelector;//determines role of responding units.  Defenders when complete will occupy the base, attackers will respond to fires and move to contact.

	
	
	//determines role of responding units.  Defenders when complete will occupy the base, attackers will respond to fires and move to contact.
	[Attribute(defvalue: "50", category: "ATTACK VARIABLES", uiwidget: UIWidgets.Slider, desc: "Determines changes of defenders (stay at base) vs attackers (seek players/contact) - 1 - 100, 1 = 1% defenders, 99% attackers.", params: "1 100 1")]
	protected int defender_vs_attacks_chanceRange;
	
	//Are we still sending waves?
	protected bool m_sendingWaves = false;
	//How long (in seconds) are we sending waves?
	[Attribute(defvalue: "900", category: "ATTACK VARIABLES", uiwidget: UIWidgets.Slider, desc: "How long (in seconds) are we going to send waves?", params: "1 1200 1")]
	protected float m_totalTimer;
	static float playerDefaultTimer;	
	
	protected float m_fTotalTimer = 0;
	//Time between waves
	[Attribute(defvalue: "90", category: "ATTACK VARIABLES", uiwidget: UIWidgets.Slider, desc: "Time  (in seconds) between attempting to send another attack wave", params: "1 1100 1")]
	protected float m_waveTimer;	
	protected float m_fWaveTimer = 0;
	static float playerDefaultWaveTimer;
	
	//Time the AI should move BACK to current base (if players didnt take another base) to make sure they dont over-run or stay at players base....
	[Attribute(defvalue: "600", category: "ATTACK VARIABLES", uiwidget: UIWidgets.Slider, desc: "Determines time frame (in seconds) attackers should stay attackers.  Added in attempts to stop main base from getting over ran", params: "1 1200 1")]
	protected float rovers_retreat_timeframe;
	protected float m_rover_retreatTimer;
	
	//Time the AI should move BACK to current base (if players didnt take another base) to make sure they dont over-run or stay at players base....
	[Attribute(defvalue: "300", category: "ATTACK VARIABLES", uiwidget: UIWidgets.Slider, desc: "When a new objective is caputred we might still have old AI Alive - we need to clean them up.  We've told them to retreat, how long (in seconds) do we give them before deletion?", params: "1 1200 1")]
	protected float oldAi_retreat_timeframe;
	protected float m_oldAirover_retreatTimer;
	
	//How long should the AI attack the objective before attempting retreat if they fail to capture it?
	//Possible issues - can we check faction owner?  If they own it don't leave.
	//If they do own it, failed attack, leave - go into flee mechanics?
	//TODO CODE TO RANDOM TIMER, RANGE 10 - 25 minutes.
	[Attribute(defvalue: "1600", category: "ATTACK VARIABLES", uiwidget: UIWidgets.Slider, desc: "When an objective is caputred, how long (in seconds) should the AI maintain their assault vs retreat from objective?  Once count down is done they will retreat (and delete with Old AI timer)", params: "1 3000 1")]
	protected float signal_retreat_timeframe;
	protected float m_signal_retreat_timeframe;
	//bool related to if the attacking force should withdraw from contact -- after a random time frame (10-25 minutes) this will toggle true, once true - they will be given an order to flee to spawn object.
	//Trigger for 10 min count down will start, and after that deletion.
	protected bool StartRetreatTimer = false;
	protected bool activeRetreat = false;
	//A single spawn point from last objective to retreat back to.
	protected IEntity retreatObjective;
	
	

	protected bool needsCleaning = false;
	bool attackersActive = false;
	
	
	//sounds to advise of counter-attack happening.
	protected float notifcationTimer = 35.0;
	protected float m_notificationTimer;
	protected bool notified = false;
	
	

	protected float m_fWaitingTime = float.MAX;	// trigger Print on start	
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Duration to check if base has changed (in seconds) - Leave me alone", params: "1 30 1")]
	protected int m_iTimerToCheckBaseChange;		// in seconds

	//The actual objective.
	protected IEntity currentObjective;
	//The Entity that the enemy will seek out.
	protected IEntity e_Objective;
	ref protected array<IEntity> e_Spawns = new array<IEntity>(); 
	
	//Keep track of the AI I've spawned 
	//Reasons : If new objective is taken - order old AI to move back to A spawn (for clean up, move them to ai_PastBase list), defensive mode, and wait a period of time before despawn to clean them up.
	//Possible timer for retreat to object (for rovers) to prevent smaller groups of players from getting over ran - leave as current ai_currentBase.
	ref protected array<IEntity> ai_currentBase = new array<IEntity>();
	ref protected array<IEntity> ai_pastBase = new array<IEntity>();
	//used to track the waypoints as entities are spawned 
	ref protected array<IEntity> ai_EwayPoints = new array<IEntity>();
	//Actual entity array tracker for deletion anda access
	ref protected array<IEntity> ai_actualEntity = new array<IEntity>();

	
	//TODO BEFORE FINAL PACKAGE THESE NEED TO BE SET TO PREFAB ARMA DEFAULTS!!
	//TODO BEFORE FINAL PACKAGE THESE NEED TO BE SET TO PREFAB ARMA DEFAULTS!!

	[Attribute("{CE41AF625D05D0F0}Prefabs/Groups/INDFOR/Group_FIA_RifleSquad.et", category: "TROOP TYPES", desc: "Normal patrol squad to attack base")]
	protected ResourceName m_RegularRfileSquad;
	[Attribute("{6307F42403E9B8A4}Prefabs/Groups/INDFOR/Group_FIA_SharpshooterTeam.et", category: "TROOP TYPES", desc: "Sharpshooters patrol squad to attack base")]
	protected ResourceName m_SharpshootersSquad;
	
	[Attribute("{CE41AF625D05D0F0}Prefabs/Groups/INDFOR/Group_FIA_RifleSquad.et", category: "TROOP TYPES", desc: "Higher skilled - set your own custom AI if you'd like or use REGULAR HERE ALSO")]
	protected ResourceName m_HIGH_SKILL_RegularRifleSquad;
	[Attribute("{CE41AF625D05D0F0}Prefabs/Groups/INDFOR/Group_FIA_RifleSquad.et", category: "TROOP TYPES", desc: "Higher skilled - set your own custom AI if you'd like or use REGULAR HERE ALSO")]
	protected ResourceName m_HIGH_SKILL_RegularRifleSquadAT;
	[Attribute("{CE41AF625D05D0F0}Prefabs/Groups/INDFOR/Group_FIA_RifleSquad.et", category: "TROOP TYPES", desc: "Higher skilled - set your own custom AI if you'd like or use REGULAR HERE ALSO")]
	protected ResourceName m_Special_Ops_Enemy;
	
	/*
	//Standard rifle squad to spawn
	[Attribute("{CE41AF625D05D0F1}Prefabs/Groups/INDFOR/Group_FIA_RifleSquad.et", category: "TROOP TYPES", desc: "Normal patrol squad to attack base")]
	protected ResourceName m_RegularRfileSquad;
	[Attribute("{6307F42403E9B8A5}Prefabs/Groups/INDFOR/Group_FIA_SharpshooterTeam.et", category: "TROOP TYPES", desc: "Sharpshooters patrol squad to attack base")]
	protected ResourceName m_SharpshootersSquad;
	
	[Attribute("{4B8EA71B8F67FF1A}Prefabs/Groups/INDFOR/Group_FIA_RifleSquadHighSKILL.et", category: "TROOP TYPES", desc: "Higher skilled - set your own custom AI if you'd like or use REGULAR HERE ALSO")]
	protected ResourceName m_HIGH_SKILL_RegularRifleSquad;
	[Attribute("{59353F1FCF5FC173}Prefabs/Groups/INDFOR/Group_FIA_RifleSquadATHighSKILL.et", category: "TROOP TYPES", desc: "Higher skilled - set your own custom AI if you'd like or use REGULAR HERE ALSO")]
	protected ResourceName m_HIGH_SKILL_RegularRifleSquadAT;
	*/
	
	//Movement names as quick ref will fill over time to reduce footwork.
	string movement_PatrolH = "{FBA8DC8FDA0E770D}Prefabs/AI/Waypoints/AIWaypoint_Patrol_Hierarchy.et";
	string movement_SearchNDestroy = "{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et";
	string movement_Attack = "{1B0E3436C30FA211}Prefabs/AI/Waypoints/AIWaypoint_Attack.et";
	string movement_Defend = "{AAE8882E0DE0761A}Prefabs/AI/Waypoints/AIWaypoint_Defend_Hierarchy.et";

	 static FIN_Counter_Wave_Logic s_FIN_Counter_Wave_Logic;
	//Constructor
	//------------------------------------------------------------------------------------------------
	void FIN_Counter_Wave_Logic(IEntitySource src, IEntity parent)
	{
		//Grabbing the waver timer the players / servers set.
		//Using this to restore old metrics - attempting to reduce lower difficulty waves.
		playerDefaultTimer = m_totalTimer;
		playerDefaultWaveTimer = m_waveTimer;
		EasyWaveSelector = 3;
		Print("WAVE LOGIC " + EasyWaveSelector);
		SetFlags(EntityFlags.ACTIVE | EntityFlags.NO_LINK, false);
		SetEventMask(EntityEvent.FRAME);		
		if(s_FIN_Counter_Wave_Logic)
		{
		    Print("Only one instance of FIN_Counter_Attack_Manager is allowed in the world!", LogLevel.ERROR);
			delete this;
			return;
		}	
		s_FIN_Counter_Wave_Logic = this;
	}
	
	//Default Destructor
	//------------------------------------------------------------------------------------------------
	void ~FIN_Counter_Wave_Logic()
	{
	
	}
	
	//Base checking logic to determine if the base has changed, if it has, array the children in that base to determine spawning mechanics locations.
	//------------------------------------------------------------------------------------------------
	protected void CheckBaseStatus()
	{
		
		currentObjective = FIN_Counter_Attack_Manager.recentlyTakenBase;
		
		//Print("FIN_Counter_Wave_Logic - player total : " + totalPlayers + " MANAGER:" + FIN_Counter_Attack_Manager.recentlyTakenBase.GetName() + " LOGIC:" + currentObjective.GetName() + " AttackOrder " + FIN_Counter_Attack_Manager.newAttackOrder, LogLevel.ERROR);
		
		
		if(FIN_Counter_Attack_Manager.newAttackOrder && FIN_Counter_Attack_Manager.recentlyTakenBase.GetName() != "FIN_BLANK_REGISTER_PLACE_LAST")
		{
			
			e_Spawns.Clear();
			notified = false;
			array<IEntity> allChildren = new array<IEntity>();
			IEntity child = currentObjective.GetChildren();
			
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
			//Moved to sedngingwaves
			CounterAttackDecider();
			FIN_Counter_Attack_Manager.newAttackOrder = false;
			StartRetreatTimer = true;
			//Reset our list in case its not clear from last objective.
			if(ai_currentBase != null && ai_currentBase.Count() > 0)
			{
				ai_currentBase.Clear();
			}
		}
		else
		{
			//Sleep
			//Print("Current Objective IS EQUAL to MANAGER - RETURN : " + currentObjective, LogLevel.ERROR);
			return;
		}
							
	}
	
	void CounterAttackDecider()
	{

		//Print("COUNTER ATTACK DECIDER MAIN " + m_sendingWaves + ai_currentBase, LogLevel.ERROR);   
				 	
		//We are suffering a counter-attack, stop attackers from showing up in attacker scripts.
		//Also reset the timer so if we just capture we are not faced with a attack right away.
		//Turn off counter attacks and reroll the timer range.	
		//Just in case players forget to place this entity the logs will say so.
		if(FIN_Attacker_Wave_Logic.instanceCheck == true)
		{
			Print("[FIN_COUNTER_WAVE_LOGIC] Located FIN_ATTACK_WAVE_LOGIC, putting it into standby mode", LogLevel.ERROR);
			int rndRange = Math.RandomInt(FIN_Attacker_Wave_Logic.minAttackTimer, FIN_Attacker_Wave_Logic.maxAttackTimer);		
			FIN_Attacker_Wave_Logic.arePlayersAttacking =true;
			FIN_Attacker_Wave_Logic.m_counterAttackTimer = 0;
			FIN_Attacker_Wave_Logic.CounterAttackTimer = rndRange;
		}
		else
		{
			Print("[FIN_COUNTER_WAVE_LOGIC] Failed to locate our static FIN_ATTACKER_WAVE_LOGIC!", LogLevel.ERROR);
		}		    
		// Print the entire list of spawn points
		
		//Print("[FIN_COUNTER_WAVE_LOGIC] Listing all spawn points:", LogLevel.ERROR);
		foreach (IEntity spawnPoint : e_Spawns)
		{
		    if (spawnPoint != null)
		    {
		        //Print("Spawn Point: " + spawnPoint.GetName(), LogLevel.ERROR);
		    }
	        else
		    {
                 Print("Spawn Point: null", LogLevel.ERROR);
		    }
	    }
		
		m_PlayerManager = PlayerManager.Cast(GetGame().GetPlayerManager());		
		m_totalPlayers = m_PlayerManager.GetPlayerCount();	
		Print("Players in game " + m_totalPlayers, LogLevel.ERROR);	
		//Easy mode - send 1/3 spawns.
		// Check if the number of players is 8 or less (Easy Mode)
		if (m_totalPlayers <= 8)
		{
		   
			/*
			SCR_AudioSourceConfiguration audioConfig = new SCR_AudioSourceConfiguration();
			audioConfig.m_sSoundProject = m_testWav; // Replace with the actual path
			audioConfig.m_sSoundEventName = "Test"; // Name of the sound event
		
			// Configure other properties of audioConfig as needed
			SCR_SoundManagerEntity soundManager = GetGame().GetSoundManagerEntity();
			if (soundManager != null)
			{
				// Play the sound
				soundManager.CreateAndPlayAudioSource(this, audioConfig);
			}
			else
			{
				Print("[FIN_Counter_Wave_Logic] Failed to find sound manager!", LogLevel.ERROR);
			}
			*/
			//Lets reduce the over all wave timer in half from default.
			//Setting the time back to normal use.
			m_totalTimer = playerDefaultTimer;
			m_waveTimer = playerDefaultWaveTimer;
			
			 Print("Easy Waves - players present " + m_totalPlayers + " totalTimer is now : " + m_totalTimer + " defaultPlayerTimer is : " + playerDefaultTimer, LogLevel.ERROR);
		    array<IEntity> shuffleSpawns = new array<IEntity>();
		    foreach (IEntity entity : e_Spawns)
		    {
		        shuffleSpawns.Insert(entity);
		    }
		    ShuffleArray(shuffleSpawns);
		
		    // Use only one-third of the spawns, skipping three each time
		    for (int i = 0; i < shuffleSpawns.Count(); i += EasyWaveSelector)
		    {
		        IEntity spawnPoint = shuffleSpawns[i];
		        if (spawnPoint != null)
		        {
		            SpawnAIAt(spawnPoint);
		            attackersActive = true;
		            m_sendingWaves = true;
		            Print("Easy Waves: Sending waves " + m_sendingWaves + " " + m_totalPlayers, LogLevel.ERROR);    
		        }
		    }
		}
		// Check if the number of players is between 9 and 16 (Medium Mode)
		if (m_totalPlayers >= 9 && m_totalPlayers <= 16)
		{
		    Print("Medium Waves Main Body " + m_totalPlayers, LogLevel.ERROR);	
			
			
			//Setting the time back to normal use.
			m_totalTimer = playerDefaultTimer;
			m_waveTimer = playerDefaultWaveTimer;
			
		    array<IEntity> shuffleSpawns = new array<IEntity>();
		    foreach (IEntity entity : e_Spawns)
		    {
		        shuffleSpawns.Insert(entity);
		    }
		    ShuffleArray(shuffleSpawns);
		
		    // Use half of the spawns, skipping two each time
		    for (int i = 0; i < shuffleSpawns.Count(); i += MediumWaveSelector)
		    {
		        IEntity spawnPoint = shuffleSpawns[i];
		        if (spawnPoint != null)
		        {
		            SpawnAIAt(spawnPoint);
		            attackersActive = true;
		            m_sendingWaves = true;
		            Print("Medium Waves - players present " + m_totalPlayers + " totalTimer is now : " + m_totalTimer + " defaultPlayerTimer is : " + playerDefaultTimer, LogLevel.ERROR);
		        }
		    }
		}
		//hard mode.  Send all spawns.
		if(m_totalPlayers >= 17)
		{
			
			
			//Setting the time back to normal use.
			m_totalTimer = playerDefaultTimer;
			m_waveTimer = playerDefaultWaveTimer;
			
			//Structure to make possible variable waves even on hardness.  Debating using.
			 Print("Hard Waves Main Body " + m_totalPlayers, LogLevel.ERROR);	
		     // Decide what attack type (single value for testing - 1-3 for finished)
		     int attackType = 3;
			 if (attackType == 3) // Hard
		     {				
				//TODO sift the list based on hard mode and reduce over all sent waves.  Randomly pull from the list of spawns and use not all.			
		        Print("[FIN_COUNTER_WAVE_LOGIC]COUNTER ATTACK DECIDER HARD", LogLevel.ERROR);
		        foreach (IEntity spawnPoint : e_Spawns)
		        {
		            // Check if spawnPoint is not null
		            if (spawnPoint != null)
		            {					
					
			            //Print("SPAWNNER TRIGGER", LogLevel.ERROR);
						//WORKS TO SPAWN GROUPS>  INTERESTED IN AMBIENT STYLE SPAWNS TO CAPTURE THEIR CLEANUP AND USE RESPAWNS SO I DONT HAVE TO WRITE MY OWN.
						//THree is heavy but silly cuz they stack.  Timer issue?
						SpawnAIAt(spawnPoint);
						attackersActive = true;
						m_sendingWaves = true;	
						Print("Hard Waves - players present " + m_totalPlayers + " totalTimer is now : " + m_totalTimer + " defaultPlayerTimer is : " + playerDefaultTimer, LogLevel.ERROR);
		           }
				
				}
							
		    }
		}
	}
	
	
	//OnFrame counter to determine timeframe for checks.
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		/*
		//Attack notification m_waveTimer
		if(!notified)
		{
		
			m_notificationTimer += timeSlice;
			if(m_notificationTimer > notifcationTimer)
			{
					if(currentObjective != null)
					{
						//I was able to get it to hook once and then it kinda causes a fatal - very unreliable way to advise counter attack.
						//Most likely will use something generic in terms of audio play.
						SCR_CampaignMilitaryBaseComponent notifications = SCR_CampaignMilitaryBaseComponent.Cast(currentObjective.FindComponent(SCR_CampaignMilitaryBaseComponent));	
						if(notifications != null)
						{				
						    notifications.GetCampaignFaction().SendHQMessage(SCR_ERadioMsg.BASE_UNDER_ATTACK, notifications.GetCallsign());
							//Print("[Wave Logic - EOnFrame] Hooked notifications", LogLevel.ERROR);				
						}
						else
						{
							Print("[Wave Logic - EOnFrame] Unable to locate notifications SCR component", LogLevel.ERROR);
						}
						notified = true;
						m_notificationTimer = 0;
						return;
					}
					else
					{
					//Print("[Wave Logic - EOnFrame] Unable to locate currentObjective!t", LogLevel.ERROR);
						m_notificationTimer = 0;
					}
			}
		
		}
		*/
		if(StartRetreatTimer)
		{
		
			m_signal_retreat_timeframe += timeSlice;
			if(m_signal_retreat_timeframe > signal_retreat_timeframe)
			{
				m_signal_retreat_timeframe = 0;
				Print("Timer to retreat TRIGGERED", LogLevel.ERROR);
				
				//Dont let the AI attempt to go defend the base or defeats the purpose.			
				attackersActive = false;
				StartRetreatTimer = false;
				activeRetreat = true;
				needsCleaning = true;
				//Copy our current list to a "past" list in case the players moved to another base for cleanup.
				SCR_CampaignMilitaryBaseComponent notifications = SCR_CampaignMilitaryBaseComponent.Cast(currentObjective.FindComponent(SCR_CampaignMilitaryBaseComponent));	
				if(notifications != null)
				{				
					//notifications.GetCampaignFaction().SendHQMessage(SCR_ERadioMsg.WINNING, notifications.GetCallsign());
					//The current wave was pulled - we can start up our counter-attack for rnd base now.
					//Used to make sure the user has placed a attack manager.	
					if(FIN_Attacker_Wave_Logic.instanceCheck == true)
					{
						FIN_Attacker_Wave_Logic.arePlayersAttacking = false;
						FIN_Attacker_Wave_Logic.m_counterAttackTimer = 0;
					}
					else
					{
						Print("[FIN_COUNTER_WAVE_LOGIC] Failed to locate our static FIN_ATTACKER_WAVE_LOGIC!", LogLevel.ERROR);
					}		
					Print("[Wave Logic - EOnFrame] Hooked notifications", LogLevel.ERROR);				
				}
				else
				{
					Print("[Wave Logic - EOnFrame] Unable to locate notifications SCR component", LogLevel.ERROR);
				}
				
				//We need to check if the AI recpautred - if they did, don't send them away... they won.
				SCR_CampaignMilitaryBaseComponent teamCheck = SCR_CampaignMilitaryBaseComponent.Cast(currentObjective.FindComponent(SCR_CampaignMilitaryBaseComponent));
				if(teamCheck != null)
				{
					Faction faction = teamCheck.GetFaction();
					if(faction.GetFactionKey() == "US")
					{
						ForceDefensive();			
					}
					else
					{				
						Print("[Wave Logic - EOnFrame] GetFactionKey() != US, so attackers should stay.", LogLevel.ERROR);					
					}
				}
				else
				{
					Print("[Wave Logic - EOnFrame] teamcheck NULL!  Unable to tell attackers to stay.", LogLevel.ERROR);		
				}
			}
		}
		//Determine if its time to clean up old AI from last objective.  They've been given a retreat order, now its time to track the time frame to deletion.
		if(needsCleaning)
		{
			m_oldAirover_retreatTimer += timeSlice;
			if(m_oldAirover_retreatTimer > oldAi_retreat_timeframe)
			{
				m_oldAirover_retreatTimer = 0;
				Print("AI DELETION TIME FROM NEEDS CLEANING TIME CHECK", LogLevel.ERROR);
				OldAIDeletion();				
			}
		}
		
		
		if(attackersActive)
		{
			//Time frame to turn rovers into base defenders should the player fail to capture the next object.  Orders active attackers to retreat tactically.
			//THIS SHOULD ALWAYS BE HIGHER THEN THE SENDING WAVES TIMER.  
			m_rover_retreatTimer += timeSlice;
			if(m_rover_retreatTimer > rovers_retreat_timeframe)
			{
				//We need to retreat the AI in case they tracked the players back to the main base - tell them to go defensive at current objective.
				//reset timer.
				m_rover_retreatTimer = 0;
				//Check all ai and access their AIGroup componenet to tell them to move back.
				Print("[FIN_COUNTER_WAVE_LOGIC]TRIGGERING DEFENSIVE PASS NOW", LogLevel.ERROR);
				ForceDefensive();
			}
		}
		
		//Checking time frame for a new base taken.  Can reduce.
		//Print("SENDING WAVE?" + m_sendingWaves + " INSIDE TICK, m_fTotalTimer : " + m_fTotalTimer + " m_totalTimer " + m_totalTimer + " m_fWaveTimer " + m_fWaveTimer + " m_waveTimer " + m_waveTimer + " m_fWaitingTime " + m_fWaitingTime + " m_iCycleDuration " + m_iCycleDuration, LogLevel.ERROR	);
		//Print(FIN_Counter_Attack_Manager.recentlyTakenBase.GetName(), LogLevel.ERROR);
		if(m_sendingWaves)
		{																
			//total time AI are going to keep attacking the base.
	        m_fTotalTimer += timeSlice; 
	    	if (m_fTotalTimer > m_totalTimer) 
			{
				//We are no longer sending waves because the timer is up - reset things.
				//total timer reset
		        m_fTotalTimer = 0;
				//wave timer reset
				m_fWaveTimer = 0;
			    Print("[FIN_COUNTER_WAVE_LOGIC]TIMER OUT FOR WAVES TO STOP ", LogLevel.ERROR);	
				
				//stop stepping into wave sender logic
	            m_sendingWaves = false;
				//retoggle of AI to attack since we've finished the waves.
				FIN_Attacker_Wave_Logic.arePlayersAttacking = false;
				//clear our list for either enemy recap or next objective
			    CleanSpawnerList();
			    return;
	        }
	 		
			//Wave countdown for when the next wave will attack the base.			
			m_fWaveTimer += timeSlice;
			if (m_fWaveTimer > m_waveTimer)
			{
				//generic hard number in seconds for the next wave.  Will adjust.
				m_fWaveTimer = 0;
				if(FIN_Counter_Attack_Manager.recentFaction == "US")
				{
					//if(newFaction != null && newFaction.GetFactionKey() == "US")
					
					Print("[FIN_COUNTER_WAVE_LOGIC] WAVE SENT US DETECTED : " + FIN_Counter_Attack_Manager.recentFaction, LogLevel.ERROR);		
					//issue attack spawner order since we are sending another wave.
					CounterAttackDecider();			
				}
				else
				{
					Print("[FIN_COUNTER_WAVE_LOGIC] WAVE --NOT-- SENT - New faction possibly took it back : TRIGGER CLEAN UP IN CASE PLAYERS DON'T RETURN! " + FIN_Counter_Attack_Manager.recentFaction, LogLevel.ERROR);
					m_fTotalTimer = 0;
					//wave timer reset - the waves
					m_fWaveTimer = 0;	
					//We now need to hand the list of persistent AI to our Attack Manager to replace them over time with ambient spawners should the players decide not to reattack (to free up AI resources)
					//ai_currentBase in our list in here that has access to the current AI.
					//Take each one from that list and add it to the ai list in manager.
					if(!FIN_Counter_Attack_Manager.recaptureCleanupNeeded)
					{
						if(ai_currentBase != null && ai_currentBase.Count() >0)
						{
							foreach (IEntity currentAI : ai_currentBase)
							{
								FIN_Counter_Attack_Manager.AI_To_Clean_List.Insert(currentAI);
							}
							//toggle to cleanup since we've got our list populated.
							FIN_Counter_Attack_Manager.recaptureCleanupNeeded = true;
							
							//Added to stop checking on waves to be sent.
							m_sendingWaves = false;
							//Added in attempts to re-trigger attack waves.
							FIN_Attacker_Wave_Logic.arePlayersAttacking = false;
							
							
						}
						else
						{
							Print("[FIN_COUNTER_WAVE_LOGIC] Attempted to populate Manager with old AI to clean!  Failed to find ai_currentBase in WAVE LOGIC!", LogLevel.ERROR);
						}
					}
				}	
			}		
			
		}
		
		m_fWaitingTime += timeSlice;
		if (m_fWaitingTime < m_iTimerToCheckBaseChange)
		{
				//GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.myTestGui_ID);
				return;
		}

		m_fWaitingTime = 0;
		//Double check its not a startup relay attempt.
		if(FIN_Counter_Attack_Manager.recentlyTakenBase.GetName() != "FIN_BLANK_REGISTER_PLACE_LAST")
		{
			//Print("[FIN_COUNTER_WAVE_LOGIC] calling CheckBaseStatus()", LogLevel.ERROR);
			CheckBaseStatus();
		}
	}
	
	void CleanSpawnerList()
	{
		//int currentIndex = 0;
		//int totalSpawns = e_Spawns.Count();
		//Print("ATTACK LIST CLEANER ENTERED", LogLevel.ERROR);
		//foreach (IEntity spawnPoint : e_Spawns)
		//{
		            // Check if spawnPoint is not null
		//     if (spawnPoint != null)
		//     {
		                //Print("SPAWNNER TRIGGER", LogLevel.ERROR);
					//WORKS TO SPAWN GROUPS>  INTERESTED IN AMBIENT STYLE SPAWNS TO CAPTURE THEIR CLEANUP AND USE RESPAWNS SO I DONT HAVE TO WRITE MY OWN.
					//THree is heavy but silly cuz they stack.  Timer issue?
		//		SpawnAIAt(spawnPoint);
									
		//     }
				
		//	currentIndex++;
				
		//	if(currentIndex >= totalSpawns)
		//	{
				Print("[FIN_COUNTER_WAVE_LOGIC] LISTED SPAWNER LIST CLEARED", LogLevel.ERROR);
 				e_Spawns.Clear();
		//		break;					
		//	}
		//}
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
		
		//Rare grp roll - AT
		if (rndRoll <= 20)
		{
			Print("Rolled Rare");
			//50/50 roll for spec ops vs standard AT.
			int rndSelector = Math.RandomInt(1,100);
			if( rndSelector	<= 25)
			{
				Print("AT Squad");
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
				ai_currentBase.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				//Print("High_Skill Squad With AT Spawned - ", LogLevel.ERROR);
				// Create a waypoint for this group
				CreateWaypoint(group, waypointType, waypointPosition);
			}if( rndSelector >= 26)
			{
				Print("SpecOps");
				Resource specOps = GenerateAndValidateResource(m_Special_Ops_Enemy);
				if (!specOps)
				{
					Print(("[AI Spawner] Unable able to load resource for the spawn group: " + spawnGroup), LogLevel.ERROR);
					return;
				}
				// Generate the spawn parameters and spawn the group 
				SCR_AIGroup group;
				//Print("AT SMALL SPAWNED", LogLevel.ERROR);
			    // 5% chance (1 to 5) to spawn fia_standard_4man_at
			    group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(specOps, null, GenerateSpawnParameters(spawnPosition)));
				//Add AI group to array to keep track of them.
				ai_currentBase.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				//Print("High_Skill Squad With AT Spawned - ", LogLevel.ERROR);
				// Create a waypoint for this group
				CreateWaypoint(group, waypointType, waypointPosition);
			}
			
		}
		//Semi rare roll
		else if (rndRoll >= 11 && rndRoll <= 75)
		{
			Print("Semi-Rare roll");
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
				ai_currentBase.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				//Print("High_Skill Squad Spawned - ", LogLevel.ERROR);
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
				ai_currentBase.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
				//Print("High_Skill Sharpshooters Spawned - ", LogLevel.ERROR);
				// Create a waypoint for this group
				CreateWaypoint(group, waypointType, waypointPosition);
			}
			if( rndSelector > 50 && rndSelector <= 74)
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
				ai_currentBase.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
			//	Print("High_Skill Squad With AT Spawned - ", LogLevel.ERROR);
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
				ai_currentBase.Insert(group);
				if (!group)
				{
					Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
					return;
				}
			//	Print("High_Skill Squad Spawned - ", LogLevel.ERROR);
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
			ai_currentBase.Insert(group);
			if (!group)
			{
				Print("[AI Spawner] Unable to spawn group!", LogLevel.ERROR);
				return;
			}
		//	Print("Normal Skill Base Squad Spawned - ", LogLevel.ERROR);
			// Create a waypoint for this group
			CreateWaypoint(group, waypointType, waypointPosition);
			
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
		
	
		
		int rndRoll = Math.RandomInt(1, 100);
		if (rndRoll > defender_vs_attacks_chanceRange)
		{
			AIWaypoint waypointD = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resource2, null, GenerateSpawnParameters(waypointPosition)));
			if (!waypointD)
			{
				Print("[Create Waypoint] Unable to create waypoint!", LogLevel.ERROR);
				return;
			}
			//Print("DEFENDER GROUP", LogLevel.ERROR);
			group.AddWaypoint(waypointD);
			ai_EwayPoints.Insert(waypointD);
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
			ai_EwayPoints.Insert(waypoint);
			//Print("ROVER GROUPP", LogLevel.ERROR);
		}
	}
	
	
	protected void ForceDefensive()
	{	
		//Used to force attacking waves back to base based off a time frame in case the players get in trouble.
		if(attackersActive)
		{
			if (ai_currentBase != null && ai_currentBase.Count() > 0)
		    {
		        for (int i = 0; i < ai_currentBase.Count(); i++)
		        {
		            SCR_AIGroup group = SCR_AIGroup.Cast(ai_currentBase[i]);
		            if (group)
		            {
		                // Remove the current waypoint associated with this group
		                AIWaypoint currentWaypoint = AIWaypoint.Cast(ai_EwayPoints[i]);
		                if (currentWaypoint != null)
		                {
		                    group.RemoveWaypoint(currentWaypoint);
		                }
						
						Resource resource = GenerateAndValidateResource(movement_Defend);
			
						if (!resource)
						{
							Print("[ForceDefensive] Unable able to load resource for the waypoint: ", LogLevel.ERROR);
							return;
						}
					
						AIWaypoint newWaypoint = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(e_Objective.GetOrigin())));
						if (!newWaypoint)
						{
							Print("[ForceDefensive] Unable to create waypoint!", LogLevel.ERROR);
							return;
						}
		                group.AddWaypoint(newWaypoint);
		
		                // Replace the old waypoint with the new one in the array
		                ai_EwayPoints[i] = newWaypoint;
		
		                Print("Updated waypoint for group", LogLevel.ERROR);
		            }
		        }
		    }
		    else
		    {        
		        Print("[ForceDefensive] ai_currentBase is NULL or empty!", LogLevel.ERROR);
		        return;
		    }
		}
		
		if(activeRetreat)
		{
			if(retreatObjective != null)
			{
				 Print("[ForceDefensive][ActiveRetreat] have retreat point!", LogLevel.ERROR);
				
				// Clear ai_pastBase before copying to avoid duplicating elements
				ai_pastBase.Clear();
				if(ai_currentBase != null && ai_currentBase.Count() > 0)
				{
					// Add all elements from ai_currentBase to ai_pastBase
					foreach (IEntity entity : ai_currentBase)
					{
					    ai_pastBase.Insert(entity);			
					}
					//clearing current list.
					ai_currentBase.Clear();
				}
				if (ai_pastBase != null && ai_pastBase.Count() > 0)
			    {
			        for (int i = 0; i < ai_pastBase.Count(); i++)
			        {
			            SCR_AIGroup group = SCR_AIGroup.Cast(ai_pastBase[i]);
			            if (group)
			            {
			                // Remove the current waypoint associated with this group
			                AIWaypoint currentWaypoint = AIWaypoint.Cast(ai_EwayPoints[i]);
			                if (currentWaypoint != null)
			                {
			                    group.RemoveWaypoint(currentWaypoint);
			                }
							
							Resource resource = GenerateAndValidateResource(movement_Defend);
				
							if (!resource)
							{
								Print("[ForceDefensive][ActiveRetreat] Unable able to load resource for the waypoint: ", LogLevel.ERROR);
								return;
							}
							if(retreatObjective != null)
							{
								AIWaypoint newWaypoint = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(retreatObjective.GetOrigin())));
								if (!newWaypoint)
								{
									Print("[ForceDefensive][ActiveRetreat] Unable to create waypoint!", LogLevel.ERROR);
									return;
								}
				                group.AddWaypoint(newWaypoint);
				
				                // Replace the old waypoint with the new one in the array
				                ai_EwayPoints[i] = newWaypoint;
				
				                Print("Updated waypoint for group", LogLevel.ERROR);
							}
							else
							{
								Print("[Create Waypoint] Unable to create waypoint!", LogLevel.ERROR);
							}
			            }
			        }
			    }
			    else
			    {        
			        Print("[ForceDefensive] ai_currentBase is NULL or empty!", LogLevel.ERROR);
			        return;
			    }
			}
		}
		else
		{
			 Print("[ForceDefensive][ActiveRetreat] retreat point null!", LogLevel.ERROR);
		}
		
	}
	
	
	protected void OldAIDeletion()
	{
		//FOR TESTING - WHEN COMPLETE RETURN VALUE TO ai_pastBase!!!!!
		//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//Ill forget - if you are reading this tell me in discord to change this im most likely huntinng this down now....
		//Unless its uncommented then I figured it out... 
		
		 Print("[OldAIDeletion] Entered!", LogLevel.ERROR);
		if(ai_EwayPoints != null && ai_EwayPoints.Count() > 0)
		{
			foreach (IEntity entity : ai_EwayPoints)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
				Print("[OldAIDeletion]Waypoints Removed!", LogLevel.ERROR);
				
			}
			//Clearing the waypoints
			ai_EwayPoints.Clear();
		}
		if(ai_pastBase != null && ai_pastBase.Count() > 0)
		{
			foreach (IEntity entity : ai_pastBase)
			{
				 Print("[OldAIDeletion] DELETING SCR_AIGROUPS!", LogLevel.ERROR);
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
				
			}
			//Removing the AI list from past
			ai_pastBase.Clear();
		}
		Print("[FIN_Counter_Wave_Logic] OldAIDelete call complete", LogLevel.ERROR);
		
		/*
		//TESTING MODEL BYPASS
		if(ai_currentBase != null && ai_currentBase.Count() > 0)
		{
			foreach (IEntity entity : ai_currentBase)
			{
				 Print("[OldAIDeletion] DELETING SCR_AIGROUPS!", LogLevel.ERROR);
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
				
			}
			//Removing the AI list from past
			ai_currentBase.Clear();
		}
		*/			
		activeRetreat = false;
		needsCleaning = false;
		retreatObjective = null;
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
	// Method to shuffle an array
	void ShuffleArray(array<IEntity> arr)
	{
	    for (int i = arr.Count() - 1; i > 0; i--)
	    {
	        int j = Math.RandomInt(0, i + 1);
	        IEntity temp = arr[j];
	        arr[j] = arr[i];
	        arr[i] = temp;
	    
		
		
		}
	}
}

