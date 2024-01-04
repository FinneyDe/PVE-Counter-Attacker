//Custom class in attempts to over-rider and bypass character controller issues with AI reviving and throwing server fatal.
modded class SCR_CharacterControllerComponent : ScriptComponent
{
	override protected void OnConsciousnessChanged(bool conscious)
	{
		OnLifeStateChanged(GetLifeState());
		
		if (conscious)
			return;

		if (IsDead())
			return;

		AIControlComponent aiControl = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
		if (!aiControl || !aiControl.IsAIActivated())
			return;
		
		IEntity currentWeapon;
		BaseWeaponManagerComponent wpnMan = GetWeaponManagerComponent();
		if (wpnMan && wpnMan.GetCurrentWeapon())
			currentWeapon = wpnMan.GetCurrentWeapon().GetOwner();
		
		//COMMENTED OUT --- FINKONE ---------------------------------------------------------------------------------------------------------------------------------------------------
		//Tested with MAX and tested with OFF - default setting leads to AI to case a fatal server crash when very high in numbers.  
		//Unable to determine if its due to odd situation (throwing custom smoke grenades for OUR server), or if weapon is disappearing due to resource limits of pushing the engine.
		//If you are not using any custom eqp AI feel free to delete this file.  
		//If you receive a 130c Line of SCR_CharacterControllerComponenet feel free to reinstall this file.
		
		//Generated behaviour bypassed by disabling - AI will not track their weapon when they get knocked unconcious - upon standing they will be weaponless.
		//IF they have a sidearm, or secondary main weapon they will pull it out.  This leads to the fighting force feeling beaten up and I honestly don't mind it.

		//Print("FIN_OVERRIDER_CHARACTERCONTROLLERCOMPONENT", LogLevel.ERROR);
		
		//COMMENTED OUT LINES ---------------------------------------------------------------------------------------------------------------------------------------------------------
		//if (currentWeapon)
			//TryEquipRightHandItem(null, EEquipItemType.Max, true);
	}
	
}