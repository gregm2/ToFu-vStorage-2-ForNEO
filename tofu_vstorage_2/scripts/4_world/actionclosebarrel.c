modded class ActionCloseBarrel
{
	override void OnExecuteServer( ActionData action_data )
	{
		Object target_object = action_data.m_Target.GetObject();
		Barrel_ColorBase ntarget = Barrel_ColorBase.Cast( target_object );
		
		PlayerBase player = action_data.m_Player;
		PlayerIdentity playerID;
		string steamID = "";
		
		if (player)
		{
			playerID = player.GetIdentity();
			if (playerID)
			{
				steamID = playerID.GetPlainId();
			}
		}
		if (steamID == "")
		{
			GetGame().AdminLog("Barrel close failed due to no steam ID" );
			return;
		}
		
		if( ntarget )
		{
			if (ntarget.canInteract(steamID) || ntarget.canInteractAdmin(steamID))
			{
				if (ntarget.vst_neo_check_cooldown_and_notify(playerID))
				{
					ntarget.vclose(steamID);
					ntarget.vst_neo_closed_by(playerID); // gotta be first to detect truly empty barrel
					super.OnExecuteServer(action_data);
				}
				/* check function above will perform cooldown notification */
			}
			else
			{
				ntarget.vst_neo_send_locked_notification(playerID);
			}
		}
	}
}