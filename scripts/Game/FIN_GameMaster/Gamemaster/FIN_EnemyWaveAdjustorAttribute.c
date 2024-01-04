[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class FIN_SmallEnemyWaveAdjustorEditorAttribute : SCR_BaseValueListEditorAttribute
{		
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) 
			return null;
		
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;
		
		ChimeraWorld world = ChimeraWorld.CastFrom(gameMode.GetWorld());
		if (!world)
			return null;
		
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager) 
			return null;
			
		//Print("Value from Wave_Logic " + FIN_Counter_Wave_Logic.EasyWaveSelector);
		return SCR_BaseEditorAttributeVar.CreateInt(FIN_Counter_Wave_Logic.EasyWaveSelector);
	}
	
	
	
	//The Time preset doesn't actually set anything other then the slider
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		//Print("Var : " + var + "Int " + var.GetInt());
		FIN_Counter_Wave_Logic.EasyWaveSelector = var.GetInt();
		return;
	}	
	
	override void PreviewVariable(bool setPreview, SCR_AttributesManagerEditorComponent manager)
	{
		if (!manager)
			return;		
	}
};
