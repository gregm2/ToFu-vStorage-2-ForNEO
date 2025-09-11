modded class ActionWorldCraft
{
	// according to code comments, this is when the server gets the recipe ID
	override void HandleReciveData(ActionReciveData action_recive_data, ActionData action_data)
	{
		super.HandleReciveData(action_recive_data, action_data);
		
		int recid;
		string recname = "";
		WorldCraftActionData action_data_wc = WorldCraftActionData.Cast(action_data);
		
		if (!action_data_wc)
		{
			return;
		}
		
		recid = action_data_wc.m_RecipeID;
		PluginRecipesManager module_recipes_manager;
		Class.CastTo(module_recipes_manager,  GetPlugin(PluginRecipesManager) );
		if (!module_recipes_manager)
		{
			return;
		}
		if (recid < 0)
		{
			return;
		}
		
		if (module_recipes_manager.RecipeIDFromClassname("PokeHolesBarrel") == recid)
		{
			Barrel_ColorBase bcb = Barrel_ColorBase.Cast(action_data_wc.m_Target.GetObject());
			if (!bcb)
			{
				return;
			}
			
			PlayerBase p = action_data_wc.m_Player;
			if (!p)
			{
				return;
			}
			
			PlayerIdentity pi = p.GetIdentity();
			if (!pi)
			{
				return;
			}
			bcb.vst_neo_send_pokehole_notification(pi);
		}
	}
};