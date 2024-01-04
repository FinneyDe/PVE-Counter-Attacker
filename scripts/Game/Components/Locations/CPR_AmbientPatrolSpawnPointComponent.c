// Custom class extending ScriptComponent for spawning AI patrols
modded class SCR_AmbientPatrolSpawnPointComponent : ScriptComponent
{
    // Gramps' addition: attribute to control the number of respawn waves. 
    // -1 for infinite respawn, 0 for no respawn.
    [Attribute("0", UIWidgets.EditBox, "How many waves will the group respawn. (0 = no respawn, -1 = infinite respawn)", "-1 inf 1")]
    protected int m_iRespawnWaves;	
	
	AIGroup m_Group;
	
	
    // Overrides the SpawnPatrol method from the parent class.
    override void SpawnPatrol()
    {
		
        // Gets the faction affiliation component from the owner entity.
        SCR_FactionAffiliationComponent comp = SCR_FactionAffiliationComponent.Cast(GetOwner().FindComponent(SCR_FactionAffiliationComponent));
        
        // If there is no faction affiliation component, return early.
        if (!comp)
            return;
        
        // Try to get the affiliated faction or the default affiliated faction.
        SCR_Faction faction = SCR_Faction.Cast(comp.GetAffiliatedFaction());
        if (!faction)
            faction = SCR_Faction.Cast(comp.GetDefaultAffiliatedFaction());
        
        // Update the spawn point if the faction has changed or if there's a respawn period set.
        if (faction != m_SavedFaction || m_iRespawnPeriod > 0)
            Update(faction);
        
        // Mark as spawned.  Variable is declared in the SCR but able ot be accessed here without a ref (the override refs natuerally)
        m_bSpawned = true;        		
		
        // If the prefab is empty, return early.
        if (m_sPrefab.IsEmpty())
            return;
        
        // Load the prefab resource.
        Resource prefab = Resource.Load(m_sPrefab);
        // If the prefab is invalid, return early.
        if (!prefab || !prefab.IsValid())
            return;
		
        // Set up the parameters for spawning the entity.
        EntitySpawnParams params = EntitySpawnParams();
        params.TransformMode = ETransformMode.WORLD;
        params.Transform[3] = GetOwner().GetOrigin();
        Math.Randomize(-1);
        
        // Randomly decide the spawn point if no respawn period is set and a waypoint is assigned.
        if (m_iRespawnPeriod == 0 && m_Waypoint && Math.RandomFloat01() >= 0.5)
        {
            AIWaypointCycle cycleWP = AIWaypointCycle.Cast(m_Waypoint);
            
            // If a cycle of waypoints exists, randomly select one as the spawn point.
            if (cycleWP)
            {
                array<AIWaypoint> waypoints = {};
                cycleWP.GetWaypoints(waypoints);
                params.Transform[3] = waypoints.GetRandomElement().GetOrigin();
            }
        }
        
        // Spawn the AI group.
        m_Group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(prefab, null, params));
        
        // If the group wasn't successfully created, return early.
        if (!m_Group)
            return;
        
        // Configure the group's spawning behavior.
        if (!m_Group.GetSpawnImmediately())
        {
            if (m_iMembersAlive > 0)
                m_Group.SetMaxUnitsToSpawn(m_iMembersAlive);
            
            // Gramps' addition: if there's a respawn period, spawn units multiple times.
            if (m_iRespawnPeriod > 0)    
                for(int k=3;k>0;k--)   
                    m_Group.SpawnUnits();
            else 
                m_Group.SpawnUnits(); // Otherwise, spawn units normally.
        }
        
        // Add a waypoint to the group.
        m_Group.AddWaypoint(m_Waypoint);
        
        // If there is a respawn period and respawn waves are set, attach an event handler.
        if ((m_iRespawnPeriod != 0 && m_iRespawnWaves != 0) || (m_iRespawnPeriod != 0 && m_iRespawnWaves == -1))
            m_Group.GetOnAgentRemoved().Insert(OnAgentRemoved);
    }
	
	override void OnAgentRemoved()
	{
		#ifndef AR_CAMPAIGN_TIMESTAMP
		if (!m_Group || m_Group.GetAgentsCount() > 0 || m_fRespawnTimestamp >= Replication.Time())
			return;
		#else
		if (!m_Group || m_Group.GetAgentsCount() > 0)
			return;

		ChimeraWorld world = GetOwner().GetWorld();
		if (m_fRespawnTimestamp.GreaterEqual(world.GetServerTimestamp()))
			return;
		#endif
		
		// Set up respawn timestamp, convert s to ms, reset original group size
		#ifndef AR_CAMPAIGN_TIMESTAMP
		m_fRespawnTimestamp = (Replication.Time() + (m_iRespawnPeriod * 1000));
		#else
		m_fRespawnTimestamp = world.GetServerTimestamp().PlusSeconds(m_iRespawnPeriod);
		#endif
		/* Gramps added >>> */if(m_iRespawnWaves != -1)		m_iRespawnWaves--;/* <<< Gramps added */
		m_iMembersAlive = -1;
		m_bSpawned = false;
	}
};