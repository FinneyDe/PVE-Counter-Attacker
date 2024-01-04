modded class SCR_GameModeCampaign : SCR_BaseGameMode
{
//------------------------------------------------------------------------------------------------
	override protected void Start()
	{
		// Handle player spawnpoints override
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(FindComponent(SCR_PlayerSpawnPointManagerComponent));

		if (playerSpawnPointManager)
		{
			if (m_iMaxRespawnRadios >= 0)
			{
				playerSpawnPointManager.EnablePlayerSpawnPoints(true);
				GetGame().GetCallqueue().CallLater(CheckRadioSpawnpointsSignalCoverage, DEFAULT_DELAY, true);
			}
			else
				playerSpawnPointManager.EnablePlayerSpawnPoints(false);
		}

		// Compose custom bases array from header
		SCR_MilitaryBaseManager baseManager = SCR_MilitaryBaseManager.GetInstance();

		if (!baseManager)
			return;

		float customHQSupplies = m_iHQStartingSupplies;
		bool whitelist = false;
		array<string> customBaseList = {};

		SCR_MissionHeaderCampaign header = SCR_MissionHeaderCampaign.Cast(GetGame().GetMissionHeader());

		if (header)
		{
			whitelist = header.m_bCustomBaseWhitelist;
			customHQSupplies = header.m_iStartingHQSupplies;

			foreach (SCR_CampaignCustomBase customBase : header.m_aCampaignCustomBaseList)
			{
				customBaseList.Insert(customBase.GetBaseName());
			}
		}

		array<SCR_CampaignMilitaryBaseComponent> candidatesForHQ = {};
		array<SCR_CampaignMilitaryBaseComponent> controlPoints = {};
		array<SCR_MilitaryBaseComponent> bases = {};
		baseManager.GetBases(bases);

		string baseName;
		SCR_CampaignMilitaryBaseComponent campaignBase;
		int listIndex;

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBase)
				continue;

			// Ignore the base if it's disabled in mission header
			if (header)
			{
				baseName = campaignBase.GetOwner().GetName();
				listIndex = customBaseList.Find(baseName);

				if (listIndex != -1)
				{
					if (!whitelist)
						continue;
				}
				else if (whitelist)
				{
					continue;
				}

				if (whitelist && listIndex != -1)
					campaignBase.ApplyHeaderSettings(header.m_aCampaignCustomBaseList[listIndex]);
			}

			if (!campaignBase.DisableWhenUnusedAsHQ() || !campaignBase.CanBeHQ())
			{
				campaignBase.Initialize();
				m_BaseManager.AddTargetActiveBase();
			}

			if (campaignBase.CanBeHQ())
				candidatesForHQ.Insert(campaignBase);

			if (campaignBase.IsControlPoint())
				controlPoints.Insert(campaignBase);
		}

		m_BaseManager.UpdateBases();
		//FINKONE
		//Adjusted from 2 to 1 - we dont need a USSR base.
		if (candidatesForHQ.Count() < 1)
		{
			Print("Not enough suitable starting locations found in current setup. Check 'Can Be HQ' attributes in SCR_CampaignMilitaryBaseComponent!", LogLevel.ERROR);
			return;
		}

		// Process HQ selection
		array<SCR_CampaignMilitaryBaseComponent> selectedHQs = {};
		m_BaseManager.SelectHQs(candidatesForHQ, controlPoints, selectedHQs);
		m_BaseManager.SetHQFactions(selectedHQs);

		foreach (SCR_CampaignMilitaryBaseComponent hq : selectedHQs)
		{
			hq.SetAsHQ(true);

			if (customHQSupplies == -1)
				hq.SetStartingSupplies(m_iHQStartingSupplies);
			else
				hq.SetStartingSupplies(customHQSupplies);

			if (!hq.IsInitialized())
			{
				hq.Initialize();
				m_BaseManager.AddTargetActiveBase();
			}
		}

		m_BaseManager.InitializeBases(selectedHQs, m_bRandomizeSupplies);

		if (m_iCallsignOffset == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
		{
			int basesCount = m_BaseManager.GetTargetActiveBasesCount();

			Math.Randomize(-1);
			m_iCallsignOffset = Math.RandomIntInclusive(0, Math.Ceil(basesCount * 0.5));
		}

		Replication.BumpMe();

		array<SCR_SpawnPoint> spawnpoints = SCR_SpawnPoint.GetSpawnPoints();

		foreach (SCR_SpawnPoint spawnpoint : spawnpoints)
		{
			DisableExtraSpawnpoint(spawnpoint);
		}

		SCR_SpawnPoint.Event_SpawnPointAdded.Insert(DisableExtraSpawnpoint);

		// Start periodical checks for winning faction
		GetGame().GetCallqueue().CallLater(CheckForWinner, DEFAULT_DELAY, true);

		SCR_CharacterRankComponent.s_OnRankChanged.Insert(OnRankChanged);
		SCR_AmbientVehiclesManager.GetInstance().GetOnVehicleSpawned().Insert(OnAmbientVehicleSpawned);

		m_bStarted = true;
		Replication.BumpMe();
		m_BaseManager.OnAllBasesInitialized();
		OnStarted();
	}
}