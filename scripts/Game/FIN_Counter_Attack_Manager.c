[EntityEditorProps(category: "FIN_Counter_Attack_Manager", description: "Maintains static data for waves.")]
class FIN_Counter_Attack_ManagerClass : GenericEntityClass
{
	
}
	//If you are readind this - this is nothing more then a glorified pre-defined spawn and objective placing spawner.
	//This is written in a crude, barbaric, poorly planned manner but it some how turned out ok.  
	//I'm a self taught hobbyist so if you find this code offensive due to its low tech use and poor strucutre I'm sorry.
	//Ohter then that I hope you enjoy.
	//Fin
	
class FIN_Counter_Attack_Manager : GenericEntity
{
	
    static FIN_Counter_Attack_Manager s_FinCounterAttackManager;
	
	//[Attribute(defvalue: "5", uiwidget: UIWidgets.Slider, desc: "Print cycle period (in seconds)", params: "1 30 1")]
	//protected int m_iCycleDuration;
	
	protected float m_fWaitingTime = float.MAX;	// trigger Print on start
	
	static IEntity recentlyTakenBase;
	static string recentFaction;
	static bool newAttackOrder = false;
	
	//Attempts at making sure we track the current base, poorly implemented and disabled.
	//static bool m_recaptured = false;
	
	//Did the enemy retake a base?  Lets start a timer in case the players say fuck that base.
	static bool recaptureCleanupNeeded;	
	//The list of AI we need to add to this array so we can keep track (just in case they attack a different base or don't come back in awhile)
	ref static array<IEntity> AI_To_Clean_List = new array<IEntity>();
	
	//Timer so we can determine when its time clean up in seconds (10 mins)
	[Attribute(defvalue: "1500", category: "COUNTER ATTACKER SETTINGS", uiwidget: UIWidgets.Slider, desc: "Cleanup timer if players do not reattack the same base if the AI takes it back (players defeated).  Replaces persistent AI with ambient AI for server performance saving.", params: "600 3000 1")]
	protected float timerToCleanUp;
	//trigger to reset and or count down.
	protected float m_timerToCleanup;
	
	//Ambieht patrol prefab.
	[Attribute("{9273AB931008C271}Prefabs/Systems/AmbientPatrol/AmbientPatrolSpawnpoint_FIA.et", category: "COUNTER ATTACKER SETTINGS", desc: "When we clean up, we replace the costly persistent AI with the costly ambient ones ;)")]
	protected ResourceName ambientPrefab;
	
	//LIST ALL BASES - WE NEED TO DISABLE CAPTURE COMPONENTS TO PREVENT OTHER ACTIONS (LONE WOLFS CAN WAIT)
	static ref array<IEntity> e_Bases = new array<IEntity>(); 
	protected bool m_captureComponenetsOff = false;
	
	
	//------------------------------------------------------------------------------------------------
	void FIN_Counter_Attack_Manager(IEntitySource src, IEntity parent)
	{
				
		SetFlags(EntityFlags.ACTIVE | EntityFlags.NO_LINK, false);
		SetEventMask(EntityEvent.FRAME);
		if(s_FinCounterAttackManager)
		{
		    Print("Only one instance of FIN_Counter_Attack_Manager is allowed in the world!", LogLevel.ERROR);
			delete this;
			return;
		}
		
		s_FinCounterAttackManager = this;

	}
	
	//Default Destructor
	//------------------------------------------------------------------------------------------------
	void ~FIN_Counter_Attack_Manager()
	{
	
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		//Below works - however its just a pain to recreate all the triggers and what not that disable does.  		
		//Was debating using my own lists to prevent capping multi objectives.  Debating not now.
		//In the end a person will use the tools they have;  This was the quickest way for me to maintain control of capturing objectives.
		
		if(FIN_Attacker_Wave_Logic.arePlayersAttacking)
		{
			if(e_Bases != null && e_Bases.Count() > 0)
			{		
				//Print("[FIN_Counter_Attack_Manager] e_Bases Fuond", LogLevel.ERROR);		
				//players are attacking, disable the capture of other bases to make that base the focus.
				foreach(IEntity entity : e_Bases)
				{
					if(entity != null)
					{
						
					
						if(!m_captureComponenetsOff)
						{
							
							
							SCR_SeizingComponent captureComp = SCR_SeizingComponent.Cast(entity.FindComponent(SCR_SeizingComponent));
							//SCR_CampaignMilitaryBaseComponent notifications = SCR_CampaignMilitaryBaseComponent.Cast(selectedBase.FindComponent(SCR_CampaignMilitaryBaseComponent));
							//Unkonw if correct call - disable might make unstable.
							//Noticed m_bEnabled
							if(captureComp != null)
							{
								//Disable works - but how do we get them back on?
								//captureComp.Disable();
								BaseGameTriggerEntity triggerEntity = captureComp.TriggerGetter();
								if (triggerEntity != null)
								{
								    vector transform[4];
								    triggerEntity.GetTransform(transform);
								
								    // Manually update the Y component of the position
								    vector position = transform[3];
								    float newY = position[1] - 75; // Subtract 75 from the Y component
								    position[1] = newY; // Assign the new Y value back to the position vector
								    transform[3] = position; // Update the transform array with the new position
								
								    triggerEntity.SetTransform(transform);
								
								    //Print("[FIN_Counter_Attack_Manager] Moved trigger entity down - " + entity.GetName(), LogLevel.ERROR);  
								}
																		
							}
							else
							{
								Print("[FIN_Counter_Attack_Manager] captureComp null! -possible starting base" + entity.GetName(), LogLevel.ERROR);	
							}
						}
						
					}
				}
				//changed all to off - toggle our check.
				m_captureComponenetsOff = true;
				//Print("[FIN_Counter_Attack_Manager]m_captureComponenetsOff - " + m_captureComponenetsOff, LogLevel.ERROR);	
			}
			else
			{
				Print("[FIN_Counter_Attack_Manager] e_BASES NULL!  Aborting.", LogLevel.ERROR);
			}
		}
		
		if(!FIN_Attacker_Wave_Logic.arePlayersAttacking)
		{
			//Print("[FIN_Counter_Attack_Manager] attacks stopped - open back up objectives - ", LogLevel.ERROR);
			//players are not capturing, re-enable and retoggle bool.  We are starting to spider-web hard and I'm not sure if I care.  ;)
			if(m_captureComponenetsOff)
			{
				if(e_Bases != null && e_Bases.Count() > 0)
				{
					Print("[FIN_Counter_Attack_Manager] attacks stopped found list!", LogLevel.ERROR);
					foreach(IEntity entity : e_Bases)
					{
						if(entity != null)
						{
							
								Print("[FIN_Counter_Attack_Manager] attacks stopped found entity!", LogLevel.ERROR);
								SCR_SeizingComponent captureComp = SCR_SeizingComponent.Cast(entity.FindComponent(SCR_SeizingComponent));
								//SCR_CampaignMilitaryBaseComponent notifications = SCR_CampaignMilitaryBaseComponent.Cast(selectedBase.FindComponent(SCR_CampaignMilitaryBaseComponent));
								//Unkonw if correct call - disable might make unstable.
								//Noticed m_bEnabled
								if(captureComp != null)
								{
									BaseGameTriggerEntity triggerEntity = captureComp.TriggerGetter();
									if (triggerEntity != null)
									{
									    vector transform[4];
									    triggerEntity.GetTransform(transform);
									
									    // Manually update the Y component of the position
									    vector position = transform[3];
									    float newY = position[1] + 75; // Subtract 75 from the Y component
									    position[1] = newY; // Assign the new Y value back to the position vector
									    transform[3] = position; // Update the transform array with the new position
									
									    triggerEntity.SetTransform(transform);
									
									    //Print("[FIN_Counter_Attack_Manager] Moved trigger entity down - " + entity.GetName(), LogLevel.ERROR);  
									}
										Print("[FIN_Counter_Attack_Manager] e_Base comp enabled - " + entity.GetName(), LogLevel.ERROR);	
								}
								else
								{
									Print("[FIN_Counter_Attack_Manager] captureComp null!" + entity.GetName(), LogLevel.ERROR);	
								}
							
							
						}
					}
					m_captureComponenetsOff = false;
				}
			}
		}
		
		
		
		
		
		
		
		//if recaptured toggles true we needa start tracking the AI.
		if(recaptureCleanupNeeded)
		{
			//make sure timer ticks with game time
			m_timerToCleanup += timeSlice;
			if(m_timerToCleanup > timerToCleanUp)
			{
				Print("[FIN_COUNTER_ATTACK_MANAGER] CALLED CLEAN UP ACTION!", LogLevel.ERROR);
				//reset timer.
				m_timerToCleanup = 0;
				if(AI_To_Clean_List != null && AI_To_Clean_List.Count() > 0)
				{
					//time to cleanup
					//I think the most natural way would be to replace the AI thats always present, with abmient events using old AI's location.
					foreach (IEntity persistentAI : AI_To_Clean_List)
					{
						//group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(spawnPosition)));
						Resource resource = GenerateAndValidateResource(ambientPrefab);
						if(resource != null)
						{
							//RECHECK EACH THING ON THE LIST BECAUSE IT MIGHT BE DEAD!
							if(persistentAI != null)
							{
								//We might perhaps want to limit this down to 5 groups max?  Upon coming into range of ambient spawners a chug can happen.
								GetGame().SpawnEntityPrefab(resource, null, GenerateSpawnParameters(persistentAI.GetOrigin()));
								SCR_EntityHelper.DeleteEntityAndChildren(persistentAI);
								Print("[FIN_COUNTER_ATTACK_MANAGER]Created ambient - deleting persistent entity!", LogLevel.ERROR);
							}
							else
							{
								Print("[FIN_COUNTER_ATTACK_MANAGER] CLEANUP RECAPTURE -  WE LOST TRACK OF A UNIT?  THEY DED?", LogLevel.ERROR);
							}
						}
						else
						{
							Print("[FIN_COUNTER_ATTACK_MANAGER] AMBIENT PREFAB NULL!  SET IN WORLD EDITOR!", LogLevel.ERROR);
						}
					}
					//We've cleaned up our stuff, placed ambient spawns, its time to clean our list for next use.
					AI_To_Clean_List.Clear();
					//lastly turn out cleanup checks off - we are done.
					recaptureCleanupNeeded = false;
				}
				else
				{
					Print("[FIN_COUNTER_ATTACK_MANAGER] Failed to find a popualted list!  Did Counter Wave Logic send it before accessing it?", LogLevel.ERROR);
				}
			}
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