class VST_Config
{	
	
	static const string CONFIG_ROOT = "$profile:ToFuVStorage/";
    static const string FULLPATH = "$profile:ToFuVStorage/VST_Config.json";
	
	protected int script_logging = 0;
	
	protected bool display_owners_to_admins = true;
	
	protected int auto_close_random_seconds_min = 120;
	protected int auto_close_random_seconds_max = 240;
	protected int auto_close_distance = 10;
	
	protected int notification_cooldown_secs = 10;
	protected int action_cooldown_secs = 5;
	
	protected float min_distance_from_spawn_to_lock = 50.0; // set to 0.0 to disable prevention
	
	protected int max_barrels_per_player = 0; // set to 0 to not limit
	
	protected bool disable_barrel_protection = false;
	
	protected bool only_lock_if_in_flag_range = false;
	
	protected int minimum_items_to_lock = 1;
	
	protected ref array<string> Blacklist;
	protected ref array<string> Admins;
	protected ref array<string> LockableTypes;

	protected string claim_message_title = "Barrel Claimed";
	protected string claim_message_body = "You have claimed this barrel and it will be locked to others when closed. To unclaim this barrel, simply close it when it is empty. (enter '/barrel_help' in chat for more info)";
	protected string claim_message_icon = "set:dayz_inventory image:barrel";
	protected float claim_message_show_time_secs = 5.0;
	
	protected string tooclosetospawn_message_title = "Barrel Can't Be Locked Here";
	protected string tooclosetospawn_message_body = "Barrels have to be moved before locking, preferrably hidden or in a base. (enter '/barrel_help' in chat for more info)";
	protected string tooclosetospawn_message_icon = "set:dayz_inventory image:barrel";
	protected float tooclosetospawn_message_show_time_secs = 5.0;
	
	protected string toomanylocked_message_title = "Too Many Owned Barrels";
	protected string toomanylocked_message_body = "You have reached the maximum number of barrels you can own and can not be added to any more. (enter '/barrel_help' in chat for more info)";
	protected string toomanylocked_message_icon = "set:dayz_inventory image:barrel";
	protected float toomanylocked_message_show_time_secs = 5.0;
	
	protected string unclaim_message_title = "Barrel Unclaimed";
	protected string unclaim_message_body = "You have unclaimed this barrel and others may now claim it. To reclaim this barrel, simply close it with any item in it. (enter '/barrel_help' in chat for more info)";
	protected string unclaim_message_icon = "set:dayz_inventory image:barrel";
	protected float unclaim_message_show_time_secs = 5.0;

	protected string locked_message_title = "Barrel Locked";
	protected string locked_message_body = "Another player has claimed this barrel and it is locked, preventing other others from opening it or damaging it. (enter '/barrel_help' in chat for more info)";
	protected string locked_message_icon = "set:dayz_inventory image:barrel";
	protected float locked_message_show_time_secs = 5.0;

	protected string cooldown_message_title = "Barrel action cooldown";
	protected string cooldown_message_body = "Do not perform actions on virtual storage barrels too frequently, try again after a few seconds. (enter '/barrel_help' in chat for more info)";
	protected string cooldown_message_icon = "set:dayz_inventory image:barrel";
	protected float cooldown_message_show_time_secs = 5.0;
	
	protected string blacklist_message_title = "Invalid item for virtual storage";
	protected string blacklist_message_body = "The item you have placed in the barrel is incompatible with virtual storage and has been ejected. Look for it in vicinity view. (enter '/barrel_help' in chat for more info)";
	protected string blacklist_message_icon = "set:dayz_inventory image:barrel";
	protected float blacklist_message_show_time_secs = 5.0;
	
	[NonSerialized()]
	protected ref array<int> Admins_hashes = {};

		
	void VST_Config()
	{
		if (GetGame().IsServer())
		{			
		
			if (!FileExist(CONFIG_ROOT))
			{
				MakeDirectory(CONFIG_ROOT);
			}

			if (!FileExist(FULLPATH))
			{
				Default();
				return; 
			}

			Load();
		}
    }
	

	void Update_Admin_Hashes()
	{
		if (Admins_hashes)
		{
			Admins_hashes.Clear();
			if (Admins)
			{
				foreach(string adminsteamid : Admins)
				{
					int hash = adminsteamid.Hash();
					if (Admins_hashes.Find(hash) == -1)
					{
						Admins_hashes.Insert(hash);
					}
				}
			}
		}
	}

		
	bool isAdmin(string steamid)
	{
		if ((Admins) && (Admins_hashes) && (Admins_hashes.Count() > 0))
		{
			int hash = steamid.Hash();
			if (Admins_hashes.Find(hash) == -1)
			{
				return false;
			}
			
			if(Admins.Find(steamid) == -1)
			{
				return false;
			}
			
			return true;
		}
		
		Print("[NEO Barrels] admin check could not find admin list or hashes");
		return false;
	}

	bool isLockable(string barreltypename)
	{
		if ((LockableTypes) && (LockableTypes.Find(barreltypename) != -1))
		{
			return true;
		}
		return false;
	}

	void Load()
    {
		JsonFileLoader<VST_Config>.JsonLoadFile(FULLPATH, this);
		Save(); // UpdateAdminHashes call will happen here
    }

	
	protected void Save()
    {
        JsonFileLoader<VST_Config>.JsonSaveFile(FULLPATH, this);
        Update_Admin_Hashes();
    }

	protected void Default()
    {
		
		script_logging = 0;
        
		display_owners_to_admins = true;
		
		auto_close_random_seconds_min = 120;
		auto_close_random_seconds_max = 240;
		auto_close_distance = 10;
		
				
		notification_cooldown_secs = 10;
		action_cooldown_secs = 5;
		
		min_distance_from_spawn_to_lock = 50.0;
		
		max_barrels_per_player = 0;
		
		disable_barrel_protection = false;
		
		only_lock_if_in_flag_range = false;
	
		minimum_items_to_lock = 1;
			
		Blacklist = new array<string>;
		Blacklist.Insert("WrittenNote");
		Blacklist.Insert("VehicleKeyBase");
		Blacklist.Insert("MCK_CarKey_Base");
		Blacklist.Insert("ChickenBreastMeat");
		Blacklist.Insert("GoatSteakMeat");
		Blacklist.Insert("SheepSteakMeat");
		Blacklist.Insert("PigSteakMeat");
		Blacklist.Insert("WolfSteakMeat");
		Blacklist.Insert("BearSteakMeat");
		Blacklist.Insert("DeerSteakMeat");
		Blacklist.Insert("CowSteakMeat");
		Blacklist.Insert("Lard");
		Blacklist.Insert("Carp");
		Blacklist.Insert("Mackerel");
		Blacklist.Insert("Barrel_Green");
		Blacklist.Insert("Barrel_Blue");
		Blacklist.Insert("Barrel_Red");
		Blacklist.Insert("Barrel_Yellow");
		
		Admins = new array<string>;
		Admins.Insert("12345678901234567");
		
		LockableTypes = new array<string>;
		LockableTypes.Insert("Barrel_Green");
		LockableTypes.Insert("Barrel_Blue");
		LockableTypes.Insert("Barrel_Red");
		LockableTypes.Insert("Barrel_Yellow");		
		
		claim_message_title = "Barrel Claimed";
		claim_message_body = "You have claimed this barrel and it will be locked to others when closed. To unclaim this barrel, simply close it when it is empty";
		claim_message_icon = "set:dayz_inventory image:barrel";
		claim_message_show_time_secs = 5.0;
		
        tooclosetospawn_message_title = "Barrel Can't Be Locked Here";
	    tooclosetospawn_message_body = "Barrels have to be moved before locking, preferrably hidden or in a base";
	    tooclosetospawn_message_icon = "set:dayz_inventory image:barrel";
	    tooclosetospawn_message_show_time_secs = 5.0;

		toomanylocked_message_title = "Too Many Owned Barrels";
		toomanylocked_message_body = "You have reached the maximum number of barrels you can own and can not be added to any more";
		toomanylocked_message_icon = "set:dayz_inventory image:barrel";
		toomanylocked_message_show_time_secs = 5.0;
		
		unclaim_message_title = "Barrel Unclaimed";
		unclaim_message_body = "You have unclaimed this barrel and others may now claim it. To reclaim this barrel, simply close it with any item in it";
		unclaim_message_icon = "set:dayz_inventory image:barrel";
		unclaim_message_show_time_secs = 5.0;
	
		locked_message_title = "Barrel Locked";
		locked_message_body = "Another player has claimed this barrel and it is locked, preventing other others from opening it or damaging it";
		locked_message_icon = "set:dayz_inventory image:barrel";
		locked_message_show_time_secs = 5.0;
	
		cooldown_message_title = "Barrel cooldown";
		cooldown_message_body = "Please be patient while contents are saved/restored, then you may interact with the barrel";
		cooldown_message_icon = "set:dayz_inventory image:barrel";
		cooldown_message_show_time_secs = 5.0;

		blacklist_message_title = "Invalid item for virtual storage";
		blacklist_message_body = "The item you have placed in the barrel is incompatible with virtual storage and has been ejected. Look for it in vicinity view";
		blacklist_message_icon = "set:dayz_inventory image:barrel";
		blacklist_message_show_time_secs = 5.0;

		Save(); // UpdateAdminHashes call will happen here
	}

	array<string> Get_Blacklist()
	{
		return Blacklist;
	}
	
	array<string> Get_Admins()
	{
		return Admins;
	}
	
	array<int> Get_Admin_Hashes()
	{
		return Admins_hashes;
	}
	
	array<string> Get_LockableTypes()
	{
		return LockableTypes;
	}

	int Get_script_logging()
	{
		return script_logging;
	}
	
	bool Get_display_owners_to_admins()
	{
		return display_owners_to_admins;
	}
	
	void Set_display_owners_to_admins(bool displayOwners)
	{
		display_owners_to_admins = displayOwners;
	}
	
	int Get_auto_close_random_seconds_min()
	{
		return auto_close_random_seconds_min;
	}

	int Get_auto_close_random_seconds_max()
	{
		return auto_close_random_seconds_max;
	}
	
	int Get_auto_close_distance()
	{
		return auto_close_distance;
	}
	
	// Set methods for test access
	void Set_auto_close_random_seconds_min(int new_min)
	{
		auto_close_random_seconds_min = new_min;
	}

	void Set_auto_close_random_seconds_max(int new_max)
	{
		auto_close_random_seconds_max = new_max;
	}
	
	void Set_auto_close_distance(int newdistance)
	{
		auto_close_distance = newdistance;		
	}
	
	string Get_claim_message_title()
	{
		return claim_message_title;
	}
	string Get_claim_message_body()
	{
		return claim_message_body;
	}
	string Get_claim_message_icon()
	{
		return claim_message_icon;
	}
	float Get_claim_message_show_time_secs()
	{
		return claim_message_show_time_secs;
	}

	string Get_tooclosetospawn_message_title()
	{
		return tooclosetospawn_message_title;
	}
	string Get_tooclosetospawn_message_body()
	{
		return tooclosetospawn_message_body;
	}
	string Get_tooclosetospawn_message_icon()
	{
		return tooclosetospawn_message_icon;
	}
	float Get_tooclosetospawn_message_show_time_secs()
	{
		return tooclosetospawn_message_show_time_secs;
	}
	
	string Get_toomanylocked_message_title()
	{
		return toomanylocked_message_title;
	}
	string Get_toomanylocked_message_body()
	{
		return toomanylocked_message_body;
	}
	string Get_toomanylocked_message_icon()
	{
		return toomanylocked_message_icon;
	}
	float Get_toomanylocked_message_show_time_secs()
	{
		return toomanylocked_message_show_time_secs;
	}
	
	string Get_unclaim_message_title()
	{
		return unclaim_message_title;
	}
	string Get_unclaim_message_body()
	{
		return unclaim_message_body;
	}
	string Get_unclaim_message_icon()
	{
		return unclaim_message_icon;
	}
	float Get_unclaim_message_show_time_secs()
	{
		return unclaim_message_show_time_secs;
	}
	
	string Get_locked_message_title()
	{
		return locked_message_title;
	}
	string Get_locked_message_body()
	{
		return locked_message_body;
	}
	string Get_locked_message_icon()
	{
		return locked_message_icon;
	}
	float Get_locked_message_show_time_secs()
	{
		return locked_message_show_time_secs;
	}
	
	string Get_cooldown_message_title()
	{
		return cooldown_message_title;
	}
	string Get_cooldown_message_body()
	{
		return cooldown_message_body;
	}
	string Get_cooldown_message_icon()
	{
		return cooldown_message_icon;
	}
	float Get_cooldown_message_show_time_secs()
	{
		return cooldown_message_show_time_secs;
	}
	
	string Get_blacklist_message_title()
	{
		return blacklist_message_title;
	}
	string Get_blacklist_message_body()
	{
		return blacklist_message_body;
	}
	string Get_blacklist_message_icon()
	{
		return blacklist_message_icon;
	}
	float Get_blacklist_message_show_time_secs()
	{
		return blacklist_message_show_time_secs;
	}
	
	int Get_notification_cooldown_secs()
	{
		return notification_cooldown_secs;
	}
	
	int Get_action_cooldown_secs()
	{
		return action_cooldown_secs;
	}
	
	float Get_min_distance_from_spawn_to_lock()
	{
		return min_distance_from_spawn_to_lock;
	}
	
	void Set_min_distance_from_spawn_to_lock(float mindistance)
	{
		min_distance_from_spawn_to_lock = mindistance;
	}
	
	int Get_max_barrels_per_player()
	{
		return max_barrels_per_player;
	}
	
	void Set_max_barrels_per_player(int newmaxbarrels)
	{
		max_barrels_per_player = newmaxbarrels;
	}

	bool Get_disable_barrel_protection()
	{
		return disable_barrel_protection;
	}
	
	void Set_disable_barrel_protection(bool disable)
	{
		disable_barrel_protection = disable;
	}
	
	bool Get_only_lock_if_in_flag_range()
	{
		return only_lock_if_in_flag_range;
	}
	
	void Set_only_lock_if_in_flag_range(bool requireflag)
	{
		only_lock_if_in_flag_range = requireflag;
	}
	
	int Get_minimum_items_to_lock()
	{
		return minimum_items_to_lock;
	}
	
	void Set_minimum_items_to_lcok(int minitems)
	{
		minimum_items_to_lock = minitems;
	}
};