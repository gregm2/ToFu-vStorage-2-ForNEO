class NEO_VST_Barrel_Commands
{
	/* admin */
	const string VST_NEO_BARREL_UNLOCK_RADIUS_CMD = "/barrel_unlockradius";
	const string VST_NEO_BARREL_UNLOCK_ALL_CMD = "/barrel_unlockall";

	/* user */
	const string VST_NEO_BARREL_SHARE_CMD = "/barrel_share";
	const string VST_NEO_BARREL_HELP_CMD = "/barrel_help";
		
	/* topics */
	const string VST_NEO_BARREL_LOCK_TOPIC = "locking";
	const string VST_NEO_BARREL_UNLOCK_TOPIC = "unlocking";
	const string VST_NEO_BARREL_SHARING_TOPIC = "sharing";
	const string VST_NEO_BARREL_INVALID_TOPIC = "invalid_items";
	const string VST_NEO_BARREL_COOLDOWN_TOPIC = "cooldown";
	const string VST_NEO_BARREL_AUTOCLOSE_TOPIC = "autoclose";
	
	ref array<int> m_vst_neo_barrel_command_hashes;
	
	void NEO_VST_Barrel_Commands()
	{
		m_vst_neo_barrel_command_hashes = new array<int>;
		m_vst_neo_barrel_command_hashes.Insert(VST_NEO_BARREL_UNLOCK_RADIUS_CMD.Hash());
		m_vst_neo_barrel_command_hashes.Insert(VST_NEO_BARREL_UNLOCK_ALL_CMD.Hash());
		m_vst_neo_barrel_command_hashes.Insert(VST_NEO_BARREL_SHARE_CMD.Hash());
		m_vst_neo_barrel_command_hashes.Insert(VST_NEO_BARREL_HELP_CMD.Hash());
	}

	array<string> vst_neo_barrel_command_list(PlayerIdentity identity)
	{
		if (!identity)
		{
			return null;
		}
		array<string> cmdlist = new array<string>;
		cmdlist.Insert(VST_NEO_BARREL_HELP_CMD);
		cmdlist.Insert(VST_NEO_BARREL_SHARE_CMD);
		if (g_Game.GetVSTConfig().isAdmin(identity.GetPlainId()))
		{
			cmdlist.Insert(VST_NEO_BARREL_UNLOCK_RADIUS_CMD);
			cmdlist.Insert(VST_NEO_BARREL_UNLOCK_ALL_CMD);
		}
		return cmdlist;
	}
	
	array<string> vst_neo_barrel_topic_list(PlayerIdentity identity)
	{
		if (!identity)
		{
			return null;
		}
		array<string> topiclist = new array<string>;
		topiclist.Insert(VST_NEO_BARREL_LOCK_TOPIC);
		topiclist.Insert(VST_NEO_BARREL_UNLOCK_TOPIC);
		topiclist.Insert(VST_NEO_BARREL_SHARING_TOPIC);
		topiclist.Insert(VST_NEO_BARREL_INVALID_TOPIC);
		topiclist.Insert(VST_NEO_BARREL_COOLDOWN_TOPIC);
		topiclist.Insert(VST_NEO_BARREL_AUTOCLOSE_TOPIC);
		return topiclist;
	}

	PlayerIdentity vst_neo_getIDFromName(string name)
	{
		array<Man> playerarray = new array<Man>;
		g_Game.GetPlayers(playerarray);

		// if name is surivor, don't process, no way to tell them from others via chat event server-side
		string survivorcheck = name;
		survivorcheck.ToLower();
		if (survivorcheck == "survivor")
		{
			// on neo-farmers, our init.c will notify all named survivor to set their name if one 
			// starts a chat with '/', rely on that here to let them know why it's not working
			return null;
		}
		if (!playerarray)
		{
			Print("[NEO Barrels] player array was null");
			return null;
		}

        foreach (Man man: playerarray)
		{
			if (man)
			{
				PlayerIdentity id = man.GetIdentity();
				if (id)
				{
					if (id.GetName() == name)
					{
						return id;
					}
				}
			}
		}
		return null; // not found
	}

	
	void vst_neo_barrel_unlockradius_cmd(PlayerIdentity identity, float radius)
	{
		if (!identity)
		{
			return;
		}
		string response;
		Man man = identity.GetPlayer();
		if(!man)
		{
			response = "No player object found for command issuer";
			Barrel_ColorBase.vst_neo_send_player_message(identity, response);
			return;
		}
		
		if (g_Game.GetVSTConfig().isAdmin(identity.GetPlainId()))
		{
			if (radius <= 0)
			{
				response = "invalid radius " + radius;
				Barrel_ColorBase.vst_neo_send_player_message(identity, response);
				return;
			}
			int count;
			count = Barrel_ColorBase.vst_neo_unlock_in_radius(man.GetPosition(), radius);
			response = "Unlocked " + count + " barrels";
			Barrel_ColorBase.vst_neo_send_player_message(identity, response);
		}			
	}
	
	void vst_neo_barrel_unlockradius_help(PlayerIdentity identity)
	{
		if (!identity)
		{
			return;
		}
		
		if (g_Game.GetVSTConfig().isAdmin(identity.GetPlainId()))
		{
			string response = "Usage: " + VST_NEO_BARREL_UNLOCK_RADIUS_CMD + " <radius (floating point)>";
			Barrel_ColorBase.vst_neo_send_player_message(identity, response);
			response = "  unlocks locked barrels within a (2D/circular) radius of the command issuer";
			Barrel_ColorBase.vst_neo_send_player_message(identity, response);
		}			
	}
	
	void vst_neo_barrel_unlockall_cmd(PlayerIdentity identity)
	{
		if (!identity)
		{
			return;
		}
		string response;
		
		if (g_Game.GetVSTConfig().isAdmin(identity.GetPlainId()))
		{
			int count;
			count = Barrel_ColorBase.vst_neo_unlock_all();
			response = "Unlocked " + count + " barrels";
			Barrel_ColorBase.vst_neo_send_player_message(identity, response);
		}			
	}
	
	void vst_neo_barrel_unlockall_help(PlayerIdentity identity)
	{
		if (!identity)
		{
			return;
		}
		
		if (g_Game.GetVSTConfig().isAdmin(identity.GetPlainId()))
		{
			string response = "Usage: " + VST_NEO_BARREL_UNLOCK_ALL_CMD + " confirm";
			Barrel_ColorBase.vst_neo_send_player_message(identity, response);
			response = "  unlocks all locked barrels, extra confirm on command is to prevent accidents";
			Barrel_ColorBase.vst_neo_send_player_message(identity, response);
		}			
	}
	
	void vst_neo_barrel_share_cmd(PlayerIdentity identity)
	{
		if (!identity)
		{
			return;
		}
		string message = "Barrel sharing enabled, next person to close your barrel will be added to the owners list";
		Barrel_ColorBase.vst_neo_share_barrel(identity);
		Barrel_ColorBase.vst_neo_send_player_message(identity, message);	
	}
	
	void vst_neo_barrel_share_help(PlayerIdentity identity)
	{
		string response = "Usage: " + VST_NEO_BARREL_SHARE_CMD;
		Barrel_ColorBase.vst_neo_send_player_message(identity, response);
		response = "  Open a barrel, issue this command, then let someone else close your barrel to be added to owners list";
		Barrel_ColorBase.vst_neo_send_player_message(identity, response);
	}

	string vst_neo_barrel_list_string_from_array(array<string> instrings)
	{
		if (!instrings)
		{
			return "";
		}
		int index;
		string output = "";
		
		for (index = 0; index < instrings.Count(); ++index)
		{
			output += instrings.Get(index);
			if ((index + 1) < instrings.Count())
			{
				output += ", ";
			}
		}
		return output;
	}
	
	array<string> vst_neo_barrel_help_lock_topic()
	{
		array<string> content = new array<string>;
		
		content.Insert("Locking: Barrels may be locked to prevent theft or damage. To lock an unlocked barrel, add items and close it (FireBarrels DO NOT lock).");
		content.Insert(" current config:");
		array<string> locktypes = g_Game.GetVSTConfig().Get_LockableTypes();
		if ((locktypes.Find("Barrel_Red") == -1) || (locktypes.Find("Barrel_Green") == -1) || (locktypes.Find("Barrel_Yellow") == -1) || (locktypes.Find("Barrel_Blue") == -1))
		{
			// only print if not all barrels are locking
			content.Insert(" Lockable barrel types: " + vst_neo_barrel_list_string_from_array(locktypes));
		}
		
		int min_items = g_Game.GetVSTConfig().Get_minimum_items_to_lock();
		content.Insert(" Minimum items to lock a barrel: " + min_items);
		bool must_have_flag = g_Game.GetVSTConfig().Get_only_lock_if_in_flag_range();
		if (must_have_flag)
		{
			content.Insert(" Barrels may only be locked in range of a flag");
		}
		
		float min_distance_from_spawn = g_Game.GetVSTConfig().Get_min_distance_from_spawn_to_lock();
		content.Insert(" Barrels must be a minimum of " + min_distance_from_spawn + " meters from original spawn location to lock");
		
		int max_barrels_per_player = g_Game.GetVSTConfig().Get_max_barrels_per_player();
		if (max_barrels_per_player > 0)
		{
			content.Insert(" Each player may only have access to up to " + max_barrels_per_player + " locked barrels");
		}
		return content;
	}

	array<string> vst_neo_barrel_help_unlock_topic()
	{
		array<string> content = new array<string>;
		content.Insert("Unlocking: To unlock an locked barrel, empty all items and close it.");
		content.Insert("  (you must be a barrel owner in order to close it)");
		return content;
	}
	
	array<string> vst_neo_barrel_help_share_topic()
	{
		array<string> content = new array<string>;
		content.Insert("Sharing: To share a locked barrel, open a locked barrel that you own. Then issue");
		content.Insert("  the '" + VST_NEO_BARREL_SHARE_CMD + "' command and have your friend close the barrel");
		content.Insert("  you should both get a chat message displaying the updated list of barrel owners/");
		return content;
	}
	
	array<string> vst_neo_barrel_help_invalid_items_topic()
	{
		array<string> content = new array<string>;
		content.Insert("Invalid items: some items can't be added to barrels, either they require storage of special details");
		content.Insert("  that is not currently supported. Or can be used to avoid game mechanics like rotting food");
		content.Insert("  Invalid items will be rejected by the barrel and set on the ground near it");
		content.Insert("current invalid items:");
		array<string> bli = g_Game.GetVSTConfig().Get_Blacklist();
		if (bli)
		{
			content.Insert(vst_neo_barrel_list_string_from_array(bli));
		}
		return content;
	}
	
	array<string> vst_neo_barrel_help_cooldown_topic()
	{
		array<string> content = new array<string>;
		content.Insert("Cooldown: There are two cooldown settings in our barrel mod. One prevents rapid open/close");
		content.Insert("  actions that would trigger excessive file operations (we delete items from closed barrels and ");
		content.Insert("  save them to files). We also have a cool down on the pop-up notifications to avoid flooding your ");
		content.Insert("  screens with messages.");
		int actioncooldown = g_Game.GetVSTConfig().Get_action_cooldown_secs();
		int notifycooldown = g_Game.GetVSTConfig().Get_notification_cooldown_secs();
		content.Insert("  open/close cooldown: " + actioncooldown + " notification cooldown: " + notifycooldown);
		return content;
	}
	
	array<string> vst_neo_barrel_help_autoclose_topic()
	{
		array<string> content = new array<string>;
		content.Insert("Auto-close: In order to reduce items the server is accounting for and improve barrel security");
		content.Insert("  barrels will auto-close after some random time interval and when no player is close to them.");
		int autoclosemin = g_Game.GetVSTConfig().Get_auto_close_random_seconds_min();
		int autoclosemax = g_Game.GetVSTConfig().Get_auto_close_random_seconds_max();
		int autoclosedist = g_Game.GetVSTConfig().Get_auto_close_distance();
		content.Insert("  min/max auto-close delay (seconds): " + autoclosemin + "/" + autoclosemax + " minimum player distance: " + autoclosedist + "m");
		
		return content;
	}
	
	void vst_neo_barrel_help_print_topic(PlayerIdentity identity, array<string> content)
	{
		if ((!identity) || (!content))
		{
			return;
		}
		foreach (string line: content)
		{
			Barrel_ColorBase.vst_neo_send_player_message(identity, line);
		}
	}
	
	void vst_neo_barrel_help_command(PlayerIdentity identity, array<string> msg_strings)
	{
		string message;
		int index;
		array<string> cmds = vst_neo_barrel_command_list(identity);
		array<string> topics = vst_neo_barrel_topic_list(identity);
		array<string> content = new array<string>;
		
		if (!identity)
		{
			return;
		}
		
		if ((msg_strings.Count() <= 1) || (msg_strings.Count() > 2))
		{
			content.Insert("NEO Barrels Mod:");
			content.Insert("  Barrels have a server-side mod to use alternate storage to improve server performance.");
			content.Insert("  Locking features are also available to reduce risk of raiding/theft (FireBarrels DO NOT lock).");
			content.Insert("  Use " + VST_NEO_BARREL_HELP_CMD + " followed by a command or topic for more information.");
			vst_neo_barrel_help_print_topic(identity, content);
			if (cmds)
			{
				content.Clear();
				content.Insert("Commands:");
				message = "  " + vst_neo_barrel_list_string_from_array(cmds);
				content.Insert(message);
				vst_neo_barrel_help_print_topic(identity, content);
			}
			if (topics)
			{
				content.Clear();
				content.Insert("Topics:");
				message = "  " + vst_neo_barrel_list_string_from_array(topics);
				content.Insert(message);
				vst_neo_barrel_help_print_topic(identity, content);
			}
			return;
		}
		else // 2 items in the list
		{
			bool helpissued = false;
			if (g_Game.GetVSTConfig().isAdmin(identity.GetPlainId()))
			{
				switch (msg_strings[1])
				{
					case VST_NEO_BARREL_UNLOCK_RADIUS_CMD:						
						vst_neo_barrel_unlockradius_help(identity);
						helpissued = true;
						break;
					
					case VST_NEO_BARREL_UNLOCK_ALL_CMD:
						vst_neo_barrel_unlockall_help(identity);
					    helpissued = true;
						break;
				}
			}
			if (!helpissued)
			{
				switch (msg_strings[1])
				{
					case VST_NEO_BARREL_SHARE_CMD:
						vst_neo_barrel_share_help(identity);
						break;
					
					case VST_NEO_BARREL_LOCK_TOPIC:
						content = vst_neo_barrel_help_lock_topic();
						vst_neo_barrel_help_print_topic(identity, content);
						break;
					
					case VST_NEO_BARREL_UNLOCK_TOPIC:
						content = vst_neo_barrel_help_unlock_topic();
						vst_neo_barrel_help_print_topic(identity, content);
						break;
					
					case VST_NEO_BARREL_SHARING_TOPIC:
						content = vst_neo_barrel_help_share_topic();
						vst_neo_barrel_help_print_topic(identity, content);
						break;
					
					case VST_NEO_BARREL_INVALID_TOPIC:
						content = vst_neo_barrel_help_invalid_items_topic();
						vst_neo_barrel_help_print_topic(identity, content);
						break;
					
					case VST_NEO_BARREL_COOLDOWN_TOPIC:
						content = vst_neo_barrel_help_cooldown_topic();
						vst_neo_barrel_help_print_topic(identity, content);
						break;
					
					case VST_NEO_BARREL_AUTOCLOSE_TOPIC:
						content = vst_neo_barrel_help_autoclose_topic();
						vst_neo_barrel_help_print_topic(identity, content);
						break;
					
					default:
						message = "Unknown command or topic, try " + VST_NEO_BARREL_HELP_CMD + " for a list. Make sure commands have leading '/'";
						break;
				}
			}
		}		
	}
	
	void vst_neo_barrel_handle_command(string name, string message)
	{
		PlayerIdentity identity;
		if (message.Get(0) == "/")
		{
			TStringArray strs = new TStringArray;
			float radius;
			message.Split(" ", strs);
			if (strs.Count() > 0)
			{
				if (m_vst_neo_barrel_command_hashes.Find(strs[0].Hash()) == -1)
				{
					return;
				}
				
				identity = vst_neo_getIDFromName(name);
				
				if (!identity)
				{
					return;
				}
				
				switch(strs[0])
				{
					case VST_NEO_BARREL_UNLOCK_RADIUS_CMD:						
						if (strs.Count() > 1)
						{
							radius = strs[1].ToFloat();
							vst_neo_barrel_unlockradius_cmd(identity, radius);
						}
						else
						{
							vst_neo_barrel_unlockradius_help(identity);
						}						
						break;
					
					case VST_NEO_BARREL_UNLOCK_ALL_CMD:
						if (strs.Count() > 1)
						{
							if (strs[1] == "confirm")
							{
								vst_neo_barrel_unlockall_cmd(identity);
							}
						}
						else
						{
							vst_neo_barrel_unlockall_help(identity);
						}						
						break;
					
					case VST_NEO_BARREL_SHARE_CMD:
						vst_neo_barrel_share_cmd(identity);
						break;
					
					case VST_NEO_BARREL_HELP_CMD:
						vst_neo_barrel_help_command(identity, strs);
						break;
				}
			}
		}
	}
}	