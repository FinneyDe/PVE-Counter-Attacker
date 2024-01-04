modded class SCR_CampaignMilitaryBaseManager : SCR_CampaignMilitaryBaseManager
{
	
	[Attribute("0", desc: "Can this base be picked as a faction's main base?", category: "Campaign")]
	protected bool m_bCanBeHQ;
	//------------------------------------------------------------------------------------------------
	override protected void OnBaseFactionChanged(SCR_MilitaryBaseComponent base, Faction newFaction)
	{
		IEntity ownerEntity = base.GetOwner();
		//string entityName = ownerEntity.GetName();
		//string entName = "FIN_Counter_Attack_Manager_1";
		
		
		
		BaseWorld world = GetGame().GetWorld();
		if(ownerEntity != null)	
		{
			
			GenericEntity counterAttackManagerSetter = GenericEntity.Cast(world.FindEntityByName("FIN_COUNTER_ATTACK_MANAGER"));
			FIN_Counter_Attack_Manager.recentlyTakenBase = ownerEntity;
			Print("FIN_OVERRIDER_CMBM - Found Entity : " + ownerEntity.GetName() + " Attempted Push " + FIN_Counter_Attack_Manager.recentlyTakenBase.GetName(), LogLevel.ERROR);
			
			//Manager for all boses being populated.
			bool isEntityAlreadyInList = false;
			foreach (IEntity baseEntity : FIN_Counter_Attack_Manager.e_Bases) {
			if (baseEntity == ownerEntity) 
				{
			        isEntityAlreadyInList = true;
			        break;
			    }
			}		
			// If the entity is not in the list, add it
			if (!isEntityAlreadyInList) 
			{
			    FIN_Counter_Attack_Manager.e_Bases.Insert(ownerEntity);
				Print("FIN_OVERRIDER_CMBM - added base to list " + ownerEntity.GetName(), LogLevel.ERROR);
			}
			
			
			if(FIN_Counter_Attack_Manager.recentlyTakenBase.GetName() != "FIN_BLANK_REGISTER_PLACE_LAST")
			{
				//If its relay 4 ignore its last to get added.  If its OTHER then relay4 our timer is up and its objective time.  
				//Just update it as need be I was to lazy to confront this problem with more conviction.
				//TODO
				//Circle back to this later and improve its handling to detect better.  We should perhaps sleep this even listener until a start up timer is complete.
				
				//IF THE BASE WAS CAPUTRED BY PLAYERS, SEND.  
				//WITHOUT THIS CHECK LEADS TO MASSIVE AMOUNTS OF AI IF AI RETAKES IT...
				if(newFaction != null)
				{
					if(newFaction.GetFactionKey() == "US")
					{
						FIN_Counter_Attack_Manager.newAttackOrder = true;
					}
				}
				else
				{
					Print("FIN_OVERRIDER_CMBM -NEW FACTION NULL - SOMEONE ELSE TOOK THE BASE", LogLevel.ERROR);
				}
				
			}
			//Determines if the player running this mod wants the attacker AI to even be enabled.
			if(FIN_Attacker_Wave_Logic.instanceCheck)
			{
				//Used to determine if we need to push the base to our US list for enemy counter attacks inside FIN_ATTACK_WAVE_LOGIC
				if(newFaction != null && newFaction.GetFactionKey() == "US")
				{	
					//Assign current faciton setting to determine if we should stop sending waves.
					FIN_Counter_Attack_Manager.recentFaction = newFaction.GetFactionKey();
					
					//If its a player base, lets not attack that.  I logged in one morning to 40 vehicles on fire, and 30 enemies in the base eating US MREs.
					Print("[FIN_OVERRIDER_CMBM - US FACTION TAKEN BASE] - Can BE HQ? : " + m_bCanBeHQ, LogLevel.ERROR);	
					string baseName = ownerEntity.GetName();
					if(!baseName.StartsWith("MainBase"))
					//Was not working - perhaps because when the game starts it is not set yet.  Rather then a pause I just deicded to check string name.
				    //if (!m_bCanBeHQ)
				   {			   
						//Our toggle to makes sure that if we re-take a base, we set the bool to captured because its false.
						//FIN_Counter_Attack_Manager.m_recaptured = false;				
						//Used to insert the base that players own so we can decide which one to attack later.
						FIN_Attacker_Wave_Logic.e_playerBases.Insert(ownerEntity);
						Print("[FIN_OVERRIDER_CMBM - US FACTION TAKEN BASE] ADDED TO LIST : " + newFaction.GetFactionKey() + " -- Entity Name -- " + ownerEntity.GetName(), LogLevel.ERROR);		
				   }
				   else
				   {	
						Print("FIN_OVERRIDER_CMBM - Was a main base - bypass", LogLevel.ERROR);	
				   }						
				}
				//Used to determine if the base taken is by the FIA AI - if it has lets check out list, we can also use this to toggle OFF sending attackers so we dont over populate the obiective they took back.
				else if (newFaction != null && newFaction.GetFactionKey() == "FIA")
				{	
					//Assign current faciton setting to determine if we should stop sending waves.
					FIN_Counter_Attack_Manager.recentFaction = newFaction.GetFactionKey();
						
					 //FIN_Counter_Attack_Manager.m_recaptured = true;	          
				     string baseNameToRemove = ownerEntity.GetName();
				     for (int i = 0; i < FIN_Attacker_Wave_Logic.e_playerBases.Count(); i++)
				     {
						
						//Check if its null on start up will generate nulls.
				         if (FIN_Attacker_Wave_Logic.e_playerBases[i] != null && FIN_Attacker_Wave_Logic.e_playerBases[i].GetName() == baseNameToRemove)
				         {
							//Used to remove attack list - why would they attack their own base.
				             FIN_Attacker_Wave_Logic.e_playerBases.Remove(i);
				             Print("[FIN_OVERRIDER_CMBM - Base removed from player bases list] : " + baseNameToRemove, LogLevel.ERROR);
				             break; // Break after removing to avoid index out of bounds error
				         }
				     }		       		   
				}
				else
				{
					Print("FIN_OVERRIDER_CMBM - NEW FACTION NULL!! Possible USSR Throw (Normal Message)", LogLevel.ERROR);
				}
			}

			
			//GETFactionkey returns "FIA", "US" so on...			
			//Print("FACTION : " + newFaction.GetFactionKey(), LogLevel.ERROR);		
			//attackWaveComp.e_playerBases.Insert(ownerEntity);

			
		}
		

		
		if (!m_Campaign.IsProxy())
			EvaluateControlPoints();
	}
	//FINKONE
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	override void InitializeBases(notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs, bool randomizeSupplies)
	{
	    array<SCR_CampaignMilitaryBaseComponent> basesSorted = {};
	    SCR_CampaignMilitaryBaseComponent baseCheckedAgainst;
	    vector originHQ1 = selectedHQs[0].GetOwner().GetOrigin();
	    float distanceToHQ;
	    bool indexFound;
	    int callsignIndex;
	    array<int> allCallsignIndexes = {};
	    array<int> callsignIndexesBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetBaseCallsignIndexes();
	
	    // Grab all valid base callsign indexes (if BLUFOR has the index)
	    foreach (int indexBLUFOR : callsignIndexesBLUFOR)
	    {
	        allCallsignIndexes.Insert(indexBLUFOR);
	    }
	
	    int callsignsCount = allCallsignIndexes.Count();
	    Math.Randomize(-1);
	    Faction defaultFaction;
	
	    foreach (int iBase, SCR_CampaignMilitaryBaseComponent campaignBase : m_aBases)
	    {
	        if (!campaignBase.IsInitialized())
	            continue;
	
	        defaultFaction = campaignBase.GetFaction(true);
	
	        // Apply default faction set in FactionAffiliationComponent or INDFOR if undefined
	        if (!campaignBase.GetFaction())
	        {
	            if (defaultFaction)
	                campaignBase.SetFaction(defaultFaction);
	            else
	                campaignBase.SetFaction(m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.INDFOR));
	        }
	
	        // Register bases in range of each other
	        campaignBase.UpdateBasesInRadioRange();
	
	        // Assign callsign
	        if (campaignBase.GetType() == SCR_ECampaignBaseType.BASE)
	        {
	            callsignIndex = allCallsignIndexes.GetRandomIndex();
	            campaignBase.SetCallsignIndex(allCallsignIndexes[callsignIndex]);
	            allCallsignIndexes.Remove(callsignIndex);
	        }
	        else
	        {
	            // Relays use a dummy callsign just so search by callsign is still possible
	            campaignBase.SetCallsignIndex(callsignsCount + iBase);
	        }
	
	        // Sort bases by distance to a HQ so randomized supplies can be applied fairly (if enabled)
	        if (randomizeSupplies && campaignBase.GetType() == SCR_ECampaignBaseType.BASE)
	        {
	            indexFound = false;
	            distanceToHQ = vector.DistanceSqXZ(originHQ1, campaignBase.GetOwner().GetOrigin());
	
	            for (int i = 0, count = basesSorted.Count(); i < count; i++)
	            {
	                baseCheckedAgainst = basesSorted[i];
	
	                if (distanceToHQ < vector.DistanceSqXZ(originHQ1, baseCheckedAgainst.GetOwner().GetOrigin()))
	                {
	                    basesSorted.InsertAt(campaignBase, i);
	                    indexFound = true;
	                    break;
	                }
	            }
	
	            if (!indexFound)
	                basesSorted.Insert(campaignBase);
	        }
	    }
	
	    if (randomizeSupplies)
	        AddRandomSupplies(basesSorted, selectedHQs);
	}


	//! Add randomized supplies to each base, calculate batches so each side encounters similarly stacked bases
	override void AddRandomSupplies(notnull array<SCR_CampaignMilitaryBaseComponent> basesSorted, notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		array<int> suppliesBufferBLUFOR = {};
		array<int> suppliesBufferOPFOR = {};
		int intervalMultiplier = Math.Floor((m_Campaign.GetMaxStartingSupplies() - m_Campaign.GetMinStartingSupplies()) / m_Campaign.GetStartingSuppliesInterval());
		FactionKey factionToProcess;
		vector basePosition;
		float distanceToHQ1;
		//float distanceToHQ2;
		int suppliesToAdd;

		foreach (SCR_CampaignMilitaryBaseComponent base : basesSorted)
		{
			if (base.IsHQ())
				continue;

			basePosition = base.GetOwner().GetOrigin();
			distanceToHQ1 = vector.DistanceSq(basePosition, selectedHQs[0].GetOwner().GetOrigin());
			//FINKONE
			//We dont need to worry about a second base.  Static starting location.
			//distanceToHQ2 = vector.DistanceSq(basePosition, selectedHQs[1].GetOwner().GetOrigin());

			//if (distanceToHQ1 > distanceToHQ2)
			//	factionToProcess = selectedHQs[1].GetCampaignFaction().GetFactionKey();
			//else
			factionToProcess = selectedHQs[0].GetCampaignFaction().GetFactionKey();

			// Check if we have preset supplies stored in buffer
			if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR) && !suppliesBufferBLUFOR.IsEmpty())
			{
				suppliesToAdd = suppliesBufferBLUFOR[0];
				suppliesBufferBLUFOR.RemoveOrdered(0);
			}
			//else if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.OPFOR) && !suppliesBufferOPFOR.IsEmpty())
			//{
			//	suppliesToAdd = suppliesBufferOPFOR[0];
			//	suppliesBufferOPFOR.RemoveOrdered(0);
			//}
			else
			{
				// Supplies from buffer not applied, add random amount, store to opposite faction's buffer
				suppliesToAdd = m_Campaign.GetMinStartingSupplies() + (m_Campaign.GetStartingSuppliesInterval() * Math.RandomIntInclusive(0, intervalMultiplier));

				if (factionToProcess == m_Campaign.GetFactionKeyByEnum(SCR_ECampaignFaction.BLUFOR))
					suppliesBufferOPFOR.Insert(suppliesToAdd);
				else
					suppliesBufferBLUFOR.Insert(suppliesToAdd);
			}

			base.SetStartingSupplies(suppliesToAdd);
		}
	}
	
	override void SelectHQs(notnull array<SCR_CampaignMilitaryBaseComponent> candidates, notnull array<SCR_CampaignMilitaryBaseComponent> controlPoints, out notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
	    // If only one candidate is available, select it directly.
	    if (candidates.Count() == 1)
	    {
	        selectedHQs = {candidates[0]};
	        return;
	    }
	
	    // If more candidates are present, select one at random (or apply any other logic you prefer).
	    Math.Randomize(-1);
	    SCR_CampaignMilitaryBaseComponent selectedHQ = candidates.GetRandomElement();
	    selectedHQs = {selectedHQ};
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetHQFactions(notnull array<SCR_CampaignMilitaryBaseComponent> selectedHQs)
	{
		 // Ensure there's at least one HQ in the array
	    if (selectedHQs.Count() < 1)
	    {
	        Print("No HQs selected.", LogLevel.ERROR);
	        return;
	    }
	
	    SCR_CampaignFaction factionBLUFOR = m_Campaign.GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
	
	    // Set the faction for the first HQ only
	    selectedHQs[0].SetFaction(factionBLUFOR);
	}
}
