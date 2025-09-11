
modded class Barrel_ColorBase
{
	static const string s_vst_neo_placeholder_type = "SparkPlug";
	
	// thanks to TrueDolphin for this little trick: https://github.com/TrueDolphin/references/blob/main/other/class-nearby.c#L6
	static ref array <Barrel_ColorBase> s_vst_neo_barrel_array;
	
	static ref map <string, int> s_vst_neo_user_barrel_count;
	protected bool m_vst_neo_owners_counted_from_load = false;
	
	static ref array <PlayerIdentity> s_vst_neo_barrel_sharers;
	
	protected bool m_vst_hasitems;
	protected bool m_vst_wasplaced; // On NEO we will be using this for claimed
	protected bool m_vst_metadata_updated; // flag to indicate if metadata has been updated since last save
	protected vector m_vst_ce_spawn_position;
	protected ref array<string> m_vst_owner_names = {};
	protected ref array<string> m_vst_owner_steamids = {}; // may not match order of names
	protected ref array<int> m_vst_owner_steamid_hashes = {}; // do not store as metadata, re-generate whenver steamid array updates
	
	protected int m_didVStorage;
	
	protected int m_auto_close_random_seconds_min;
	protected int m_auto_close_random_seconds_max;
	protected int m_auto_close_distance;

	//protected ref array<string> m_BlackListItems;
	
	protected float m_vst_neo_last_notify_time; // use GetGame().GetTickTime() (value is seconds)
	protected float m_vst_neo_last_action_time; // use GetGame().GetTickTime() (value is seconds)
	
	protected bool m_vst_neo_is_restoring; // flag indicating items being restored from disk

	protected string m_vst_neo_typename;
	
	protected ref Timer m_vst_neo_autoclose_timer;
	
	protected PlayerIdentity m_vst_neo_sharerID;
	
	void Barrel_ColorBase()
	{
		// True dolphin array trick from above:
		if (!s_vst_neo_barrel_array)
		{
			s_vst_neo_barrel_array = {};
		}
		s_vst_neo_barrel_array.Insert(this);
	    
		// constructors should not require 'super' calls
		m_vst_wasplaced = false;
		
		m_vst_metadata_updated = false;
		
		m_didVStorage = false;	

		m_auto_close_random_seconds_min = g_Game.GetVSTConfig().Get_auto_close_random_seconds_min();
		m_auto_close_random_seconds_max = g_Game.GetVSTConfig().Get_auto_close_random_seconds_max();
		m_auto_close_distance = g_Game.GetVSTConfig().Get_auto_close_distance();

		m_vst_neo_last_notify_time = 0.0;
		m_vst_neo_last_action_time = 0.0;
		
		m_vst_neo_is_restoring = false;
		
		m_vst_ce_spawn_position = vector.Zero;
		
		m_vst_neo_owners_counted_from_load = false;
		
		if(GetGame().IsDedicatedServer())
		{
			if(g_Game.GetVSTConfig().Get_script_logging() == 1)
				Print("[vStorage] scheduling open/close check in 60 sec.");
				
			//GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(vst_timer_start, 60000, false, false);
		}
		
		m_vst_neo_typename = GetType();
	}
	
	void ~Barrel_ColorBase()
	{
		if (m_vst_neo_autoclose_timer)
		{
			m_vst_neo_autoclose_timer.Stop();
		}
		
		if (s_vst_neo_barrel_array) 
		{
			s_vst_neo_barrel_array.RemoveItem(this);
		}
		
		if (s_vst_neo_user_barrel_count)
		{
			if ((m_vst_owner_steamids) && (m_vst_owner_steamids.Count() > 0))
			{
				foreach(string steamid: m_vst_owner_steamids)
				{
					vst_neo_subtract_barrel_user_count(steamid);
				}
			}
		}
		//GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(vst_timer_start);
		//GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(vst_timer_end);
		//GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(vst_neo_throwout_blacklist_item);
	}
	
	static int vst_neo_unlock_all()
	{
		int count = 0;
		
		if ((s_vst_neo_barrel_array) && (s_vst_neo_barrel_array.Count() > 0))
		{
			foreach (Barrel_ColorBase barrel: s_vst_neo_barrel_array)
			{
				if(!barrel) 
				{
					Print("[NEO Barrels] had null barrel in list");
					continue;					
				}
				
				if (barrel.m_vst_wasplaced)
				{
					barrel.Unclaim();
					++count;
				}
			}
		}
		return count;
	}
	
	static int vst_neo_unlock_in_radius(vector position, float radius)
	{
		int count = 0;
		vector checkposition;
		checkposition[0] = position[0];
		checkposition[1] = 0;
		checkposition[2] = position[2];
		
		if ((s_vst_neo_barrel_array) && (s_vst_neo_barrel_array.Count() > 0))
		{
			foreach (Barrel_ColorBase barrel: s_vst_neo_barrel_array)
			{
				if(!barrel) 
				{
					Print("[NEO Barrels] had null barrel in list");
					continue;					
				}
				
				radius = radius * radius;
				vector barrelpos;
				vector barrel3dpos = barrel.GetPosition();
				barrelpos[0] = barrel3dpos[0];
				barrelpos[1] = 0;
				barrelpos[2] = barrel3dpos[2];
				
				if (vector.DistanceSq(checkposition, barrelpos) < radius)
				{
					if (barrel.m_vst_wasplaced)
					{
						barrel.Unclaim();
						++count;
					}
				}
			}
		}
		return count;
	}
	
	static void vst_neo_share_barrel(PlayerIdentity sharer)
	{
		if (!s_vst_neo_barrel_sharers)
		{
			s_vst_neo_barrel_sharers = new array<PlayerIdentity>;
		}
		if (s_vst_neo_barrel_sharers.Find(sharer) == -1)
		{
			s_vst_neo_barrel_sharers.Insert(sharer);
		}
	}
	
	static void vst_neo_send_player_message(PlayerIdentity identity, string message)	
	{
		Param1<string> Msgparam;
		Msgparam = new Param1<string>(message);
		GetGame().RPCSingleParam(identity.GetPlayer(), ERPCs.RPC_USER_ACTION_MESSAGE, Msgparam, true, identity);
	}
	
	string vst_neo_get_express_filename(string steamid)
	{
		// we don't have any express barrels yet, but may add them
		return "$profile:ToFuVStorage_V2/"+steamid+".save";
	}
	
	string vst_neo_get_save_filename()
	{
		int b1;
		int b2;
		int b3;
		int b4;
		string filename;
		GetPersistentID(b1, b2, b3, b4);
		
		filename = "$profile:ToFuVStorage_V2/container_"+b1+"_"+b2+"_"+b3+"_"+b4+".save";
		return filename;
	}
	
	string vst_neo_get_meta_filename()
	{
		int b1;
		int b2;
		int b3;
		int b4;
		string filename;
		GetPersistentID(b1, b2, b3, b4);
		
		filename = "$profile:ToFuVStorage_V2/container_"+b1+"_"+b2+"_"+b3+"_"+b4+".meta";
		return filename;
	}
	
	string vst_neo_get_save_filename_V1()
	{
		int b1;
		int b2;
		int b3;
		int b4;
		string filename;
		GetPersistentID(b1, b2, b3, b4);
		
		filename = "$profile:ToFuVStorage/container_"+b1+"_"+b2+"_"+b3+"_"+b4+".save";
		return filename;
	}
	
	string vst_neo_get_meta_filename_V1()
	{
		int b1;
		int b2;
		int b3;
		int b4;
		string filename;
		GetPersistentID(b1, b2, b3, b4);
		
		filename = "$profile:ToFuVStorage/container_"+b1+"_"+b2+"_"+b3+"_"+b4+".meta";
		return filename;
	}
	
	void vst_neo_save_metadata()
	{
		autoptr tofuvStorageContainerMeta containerObjMeta = new tofuvStorageContainerMeta();
		string filename;
		
		if(this.GetType() != "tofu_vstorage_q_barrel_express")
		{
			filename = vst_neo_get_meta_filename();
		}
		else
		{
			//filename = steamid+".meta";
			return;
		}
		
		containerObjMeta.m_vst_hasitems = m_vst_hasitems;
		containerObjMeta.m_vst_wasplaced = m_vst_wasplaced;
		containerObjMeta.m_vst_owner_names = m_vst_owner_names;
		containerObjMeta.m_vst_owner_steamids = m_vst_owner_steamids;
		
		FileSerializer file = new FileSerializer();
		if (file.Open(filename, FileMode.WRITE))
		{
			file.Write(containerObjMeta);
			file.Close();
			//Print("Metadata Serialized and saved");

		}
		
		m_vst_metadata_updated = false;
	}
	
	void vst_neo_load_metadata()
	{
		string filename;
		
		if(this.GetType() != "tofu_vstorage_q_barrel_express")
		{
			filename = vst_neo_get_meta_filename();
		}
		else
		{
			//filename = steamid+".meta";
			return;
		}
		
		// before the first save, meta file won't exist, especially loading this into a live server
		if (!FileExist(filename))
		{
			return;
		}
		
		autoptr tofuvStorageContainerMeta containerObjMeta = new tofuvStorageContainerMeta();
		FileSerializer file = new FileSerializer();
		if (file.Open(filename, FileMode.READ))
		{
			file.Read(containerObjMeta);
			file.Close();
			//Print("Metadata Serialized and saved");
		}

		m_vst_hasitems = containerObjMeta.m_vst_hasitems;
		m_vst_wasplaced = containerObjMeta.m_vst_wasplaced;
		m_vst_owner_names = containerObjMeta.m_vst_owner_names;
		m_vst_owner_steamids = containerObjMeta.m_vst_owner_steamids;
		
		if ((m_vst_owner_steamids) && (m_vst_owner_steamids.Count() > 0))
		{
			m_vst_owner_steamid_hashes.Clear();
			foreach (string steamid: m_vst_owner_steamids)
			{
				int steamid_hash = steamid.Hash();
				if (m_vst_owner_steamid_hashes.Find(steamid_hash) == -1)
				{
					m_vst_owner_steamid_hashes.Insert(steamid_hash);
				}
				
				// update barrel count
				if (!m_vst_neo_owners_counted_from_load)
				{
					vst_neo_add_barrel_user_count(steamid);
				}
			}
			m_vst_neo_owners_counted_from_load = true; // don't repeat owner loading
		}
		// commented out, place holder object should handle this now
		//if (m_vst_hasitems)
		//{
		//	SetTakeable(false);
		//}
	}
	
	void vst_neo_load_metadata_V1()
	{
		string filename;
		
		if(this.GetType() != "tofu_vstorage_q_barrel_express")
		{
			filename = vst_neo_get_meta_filename_V1();
		}
		else
		{
			//filename = steamid+".meta";
			return;
		}
		
		// before the first save, meta file won't exist, especially loading this into a live server
		if (!FileExist(filename))
		{
			return;
		}
		
		autoptr tofuvStorageContainerMeta_V1 containerObjMeta = new tofuvStorageContainerMeta_V1();
		FileSerializer file = new FileSerializer();
		if (file.Open(filename, FileMode.READ))
		{
			file.Read(containerObjMeta);
			file.Close();
			//Print("Metadata Serialized and saved");
		}

		m_vst_hasitems = containerObjMeta.m_vst_hasitems;
		m_vst_wasplaced = containerObjMeta.m_vst_wasplaced;
		
		// recover mixed up steamID and store into new steam id array
		/*
					string steamid_part1 = steamid.Substring(0,6);
					string steamid_part2 = steamid.Substring(6,6);
					string steamid_part3 = steamid.Substring(12,5);
					saveSteamid(steamid_part1,steamid_part2,steamid_part3);
				...
					void saveSteamid(string a, string b, string c) {
						string mod1 = "9"+a; 
						string mod2 = "9"+b;
						string mod3 = "9"+c;
						
						m_vst_steamid1 = mod1.ToInt();
						m_vst_steamid2 = mod2.ToInt();
						m_vst_steamid3 = mod3.ToInt();
						
						m_vst_wasplaced = true;
					}
		*/
		string mod1 = containerObjMeta.m_vst_steamid1.ToString();
		string mod2 = containerObjMeta.m_vst_steamid2.ToString();
		string mod3 = containerObjMeta.m_vst_steamid3.ToString();
		
		string steamid = mod1.Substring(1, mod1.Length() - 1) + mod2.Substring(1, mod2.Length() - 1) + mod3.Substring(1, mod3.Length() - 1);
		
		m_vst_owner_steamids.Insert(steamid);
		// name will have to get an update later after a close operation
		
		vst_neo_save_metadata(); /* convert to V2 */
		vst_neo_load_metadata(); /* now loaded and owner hashes updated */
		DeleteFile(filename);

		return;
	}

	void vst_neo_closed_by(PlayerIdentity identity)
	{
		if (!identity)
		{
			return; 
		}
		
		if(GetGame().IsServer())
		{
			GameInventory gi = GetInventory();
			CargoBase cb;
			
			if(gi)
			{
				cb = gi.GetCargo();
			}
			
			if (!cb)
			{
				return;
			}
			
			int item_count = cb.GetItemCount();
			
			if (item_count > 0)
			{
				// if the barrel was not empty and was unclaimed, claim it
				if ((!m_vst_wasplaced) || (m_vst_neo_sharerID))
				{
					Claim(identity);
				}
			}
			if (item_count == 0)
			{
				if (m_vst_wasplaced)
				{
					Unclaim(identity);
				}
			}
		}
	}
	
	void vst_neo_add_barrel_user_count(string steamid)
	{
		if (!s_vst_neo_user_barrel_count)
		{
			s_vst_neo_user_barrel_count = new map<string, int>;
		}
		
		int count;
		
		if (s_vst_neo_user_barrel_count.Find(steamid, count))
		{
			++count;
			s_vst_neo_user_barrel_count.Set(steamid, count);
		}
		else
		{
			s_vst_neo_user_barrel_count.Insert(steamid, 1);
		}
	}
	
	void vst_neo_subtract_barrel_user_count(string steamid)
	{
		if (!s_vst_neo_user_barrel_count)
		{
			return; // no mapping to subtract from
		}
		
		int count;
		
		if (s_vst_neo_user_barrel_count.Find(steamid, count))
		{
			--count;
			s_vst_neo_user_barrel_count.Set(steamid, count);
		}
		else
		{
			Print("[NEO Barrels] vst_neo_subtract_barrel_user_count called with steam id not in table " + steamid);
		}
	}
	
	bool vst_neo_action_cooldown_expired()
	{
		if (m_vst_neo_is_restoring)
		{
			return false; // block actions while restore is in progress
		}
		
		int cooldown = g_Game.GetVSTConfig().Get_action_cooldown_secs();
		if (cooldown == 0)
		{
			return true;
		}
		float currentTick = GetGame().GetTickTime();
		float checkTime = m_vst_neo_last_action_time + cooldown;
		
		if (checkTime < currentTick)
		{
			m_vst_neo_last_action_time = currentTick;
			return true;
		}
		return false;
	}
	
	bool vst_neo_notify_cooldown_expired()
	{
		int cooldown = g_Game.GetVSTConfig().Get_notification_cooldown_secs();
		if (cooldown == 0)
		{
			return true;
		}
		float currentTick = GetGame().GetTickTime();
		float checkTime = m_vst_neo_last_notify_time + cooldown;
		
		if (checkTime < currentTick)
		{
			m_vst_neo_last_notify_time = currentTick;
			return true;
		}
		return false;
	}
	
	bool vst_neo_check_cooldown_and_notify(PlayerIdentity identity)
	{
		if (vst_neo_action_cooldown_expired())
			return true;
		
		if (!vst_neo_notify_cooldown_expired())
		{
			return false; /* cooldown has not expired, but don't flood messages */
		}
		
		if (!identity)
		{
			return false; /* can't pass NULL here */
		}
		
		string title = g_Game.GetVSTConfig().Get_cooldown_message_title();
		string body = g_Game.GetVSTConfig().Get_cooldown_message_body();
		string icon = g_Game.GetVSTConfig().Get_cooldown_message_icon();
		float show_time = g_Game.GetVSTConfig().Get_cooldown_message_show_time_secs();
		
		NotificationSystem.SendNotificationToPlayerIdentityExtended(identity, show_time, title, body, icon);
		return false;
	}
	
	void vst_neo_send_locked_notification(PlayerIdentity identity)
	{
		if (!vst_neo_notify_cooldown_expired())
		{
			return; /* don't flood messages */
		}
		
		if (!identity)
		{
			return; /* can't pass NULL here */
		}
		
		string title = g_Game.GetVSTConfig().Get_locked_message_title();
		string body = g_Game.GetVSTConfig().Get_locked_message_body();
		string icon = g_Game.GetVSTConfig().Get_locked_message_icon();
		float show_time = g_Game.GetVSTConfig().Get_locked_message_show_time_secs();
		
		NotificationSystem.SendNotificationToPlayerIdentityExtended(identity, show_time, title, body, icon);
	}
	
	void vst_neo_send_blacklist_notification(EntityAI item)
	{
		if (!vst_neo_notify_cooldown_expired())
		{
			return; /* don't flood messages */
		}
		
		if (!item)
		{
			return; /* can't pass NULL here */
		}
		
		string title = g_Game.GetVSTConfig().Get_blacklist_message_title();
		string body = g_Game.GetVSTConfig().Get_blacklist_message_body();
		body = body + " Failed type: " + item.GetDisplayName();
		string icon = g_Game.GetVSTConfig().Get_blacklist_message_icon();
		float show_time = g_Game.GetVSTConfig().Get_blacklist_message_show_time_secs();
		
		autoptr array<Man> players = new array<Man>();
		GetGame().GetPlayers(players);
			
		foreach (Man man : players)
		{
			if (!man)
			{
				continue;
			}
			// message all players in 2 meters
			if (vector.DistanceSq(GetPosition(), man.GetPosition()) < 4.0)
			{
				PlayerIdentity pi;
				pi = man.GetIdentity();
				if (pi)
				{
					NotificationSystem.SendNotificationToPlayerIdentityExtended(pi, show_time, title, body, icon);
				}
			}
		}
	}
	
	void vst_neo_send_pokehole_notification(PlayerIdentity identity)
	{
		if (!vst_neo_notify_cooldown_expired())
		{
			return; /* don't flood messages */
		}
		
		if (!identity)
		{
			return; /* can't pass NULL here */
		}
		
		string title = g_Game.GetVSTConfig().Get_pokehole_message_title();
		string body = g_Game.GetVSTConfig().Get_pokehole_message_body();
		string icon = g_Game.GetVSTConfig().Get_pokehole_message_icon();
		float show_time = g_Game.GetVSTConfig().Get_pokehole_message_show_time_secs();
		
		NotificationSystem.SendNotificationToPlayerIdentityExtended(identity, show_time, title, body, icon);
	}
	
	void vst_neo_send_claim_notification(PlayerIdentity identity)
	{
		if (!vst_neo_notify_cooldown_expired())
		{
			return; /* don't flood messages */
		}
		
		if (!identity)
		{
			return; /* can't pass NULL here */
		}
		
		string title = g_Game.GetVSTConfig().Get_claim_message_title();
		string body = g_Game.GetVSTConfig().Get_claim_message_body();
		string icon = g_Game.GetVSTConfig().Get_claim_message_icon();
		float show_time = g_Game.GetVSTConfig().Get_claim_message_show_time_secs();
		
		NotificationSystem.SendNotificationToPlayerIdentityExtended(identity, show_time, title, body, icon);
	}
	
	void vst_neo_send_tooclosetospawn_notification(PlayerIdentity identity)
	{
		if (!vst_neo_notify_cooldown_expired())
		{
			return; /* don't flood messages */
		}
		
		if (!identity)
		{
			return; /* can't pass NULL here */
		}
		
		string title = g_Game.GetVSTConfig().Get_tooclosetospawn_message_title();
		string body = g_Game.GetVSTConfig().Get_tooclosetospawn_message_body();
		string icon = g_Game.GetVSTConfig().Get_tooclosetospawn_message_icon();
		float show_time = g_Game.GetVSTConfig().Get_tooclosetospawn_message_show_time_secs();
		
		NotificationSystem.SendNotificationToPlayerIdentityExtended(identity, show_time, title, body, icon);
	}
	
	void vst_neo_send_toomanylocked_notification(PlayerIdentity identity)
	{
		if (!vst_neo_notify_cooldown_expired())
		{
			return; /* don't flood messages */
		}
		
		if (!identity)
		{
			return; /* can't pass NULL here */
		}
		
		string title = g_Game.GetVSTConfig().Get_toomanylocked_message_title();
		string body = g_Game.GetVSTConfig().Get_toomanylocked_message_body();
		string icon = g_Game.GetVSTConfig().Get_toomanylocked_message_icon();
		float show_time = g_Game.GetVSTConfig().Get_toomanylocked_message_show_time_secs();
		
		NotificationSystem.SendNotificationToPlayerIdentityExtended(identity, show_time, title, body, icon);
	}
	
	void vst_neo_send_unclaim_notification(PlayerIdentity identity)
	{
		if (!vst_neo_notify_cooldown_expired())
		{
			return; /* don't flood messages */
		}
		
		if (!identity)
		{
			return; /* can't pass NULL here */
		}
		
		string title = g_Game.GetVSTConfig().Get_unclaim_message_title();
		string body = g_Game.GetVSTConfig().Get_unclaim_message_body();
		string icon = g_Game.GetVSTConfig().Get_unclaim_message_icon();
		float show_time = g_Game.GetVSTConfig().Get_unclaim_message_show_time_secs();
		
		NotificationSystem.SendNotificationToPlayerIdentityExtended(identity, show_time, title, body, icon);
	}
	

	void vst_neo_send_five_per_line(PlayerIdentity identity, string premessage, array<string> strarray)
	{
		string line = premessage;
		int index = 0;
		bool linesent = false;
		
		if ((strarray) && (strarray.Count() > 0))
		{
			for (index = 0; index < strarray.Count(); ++index)
			{
				line += strarray.Get(index);
				linesent = false;
				if (((index % 5) != 0) || (index == 0))
				{
					line += ", ";
				}
				else
				{
					vst_neo_send_player_message(identity, line);
					line = "    ";
					linesent = true;
				}
			}
			if (!linesent)
			{
				vst_neo_send_player_message(identity, line);
			}
		}
	}
	
	bool canShare()
	{
		if (s_vst_neo_barrel_sharers)
		{
			// each pass, try and remove an ID of a player who may have logged out while on list
			int index = 0;
			PlayerIdentity nullcheck;
			for (index = 0; index < s_vst_neo_barrel_sharers.Count(); index++)
			{
				nullcheck = s_vst_neo_barrel_sharers.Get(index);
				if (!nullcheck)
				{
					s_vst_neo_barrel_sharers.Remove(index);
					break; // array changed, don't keep going over it
				}
			}
			
			foreach(PlayerIdentity id: s_vst_neo_barrel_sharers)
			{
				if (id)
				{
					if (canInteract(id))
					{
						s_vst_neo_barrel_sharers.RemoveItem(id);
						m_vst_neo_sharerID = id;
						return true; // no break needed due to return
					}
				}
			}
		}
		return false;
	}
	
	bool canInteract(PlayerIdentity identity)
	{
		//Print(steamid);
	
		/*
		Print(m_vst_wasplaced);
		*/
		
		if (!identity)
		{
			Print("[NEO Barrels] canInteract received null identity");
			return false;
		}
		
		if (!isLockable())
		{
			return true;
		}
		
		string steamid = identity.GetPlainId();
		
		if(m_vst_wasplaced == false) {
			return true;
		}
				
		if ((m_vst_owner_steamid_hashes) && (m_vst_owner_steamid_hashes.Count() > 0))
		{
			// can rapidly reject non-owners with integer rather than string compare
			int steamid_hash = steamid.Hash();
			if (m_vst_owner_steamid_hashes.Find(steamid_hash) == -1)
			{
				return false;
			}
		}
		
		// now check string match
		if ((m_vst_owner_steamids) && (m_vst_owner_steamids.Find(steamid) != -1))
		{
			/* make sure name is on list if loading from v1 */
			string name = identity.GetName();
			if (m_vst_owner_names.Find(name) == -1)
			{
				m_vst_owner_names.Insert(name);
			}
			return true;
		}
				
		// default to no
		return false;
	}
	
	bool canInteractAdmin(PlayerIdentity identity, bool allow_owner_display = true)
	{
		if (!identity)
		{
			Print("[NEO Barrels] canInteract received null identity");
			return false;
		}
		
		string steamid = identity.GetPlainId();
		
		if (g_Game.GetVSTConfig().isAdmin(steamid))
		{
			if ((allow_owner_display) && (g_Game.GetVSTConfig().Get_display_owners_to_admins()))
			{
				vst_neo_send_five_per_line(identity, "Owner names: ", m_vst_owner_names);
				vst_neo_send_five_per_line(identity, "Owner IDs: ", m_vst_owner_steamids);
			}
			return true;
		}
		return false;
	}
	
	bool isLockable()
	{
		bool lockabletype = g_Game.GetVSTConfig().isLockable(m_vst_neo_typename);
		
		bool flag_check_ok = true;
		if (g_Game.GetVSTConfig().Get_only_lock_if_in_flag_range())
		{
			if (vst_neo_IsTargetInActiveRefresherRange())
			{
				flag_check_ok = true;
			}
			else
			{
				flag_check_ok = false;
			}
		}
		
		bool item_count_ok = true;
		int minimum_items = g_Game.GetVSTConfig().Get_minimum_items_to_lock();
		
		if (minimum_items > 1) // only need to check if set above 1
		{
			GameInventory gi;
			CargoBase cb;
			
			gi = GetInventory();
			
			if (gi)
			{
				cb = gi.GetCargo();
				if (cb)
				{
					if (cb.GetItemCount() < minimum_items)
					{
						item_count_ok = false;
					}
				}
			}			
		}
				
		return lockabletype && flag_check_ok && item_count_ok;
	}
	
	bool isProtectionDisabled()
	{
		return g_Game.GetVSTConfig().Get_disable_barrel_protection();
	}
	
	// stolen from bohemia playerbase.c, slight tweaks
	bool vst_neo_IsTargetInActiveRefresherRange()
	{
		array<vector> temp = new array<vector>;
		temp = GetGame().GetMission().GetActiveRefresherLocations();
		if (!temp)
		{
			return false;
		}
		
		int count = temp.Count();
		if (count > 0)
		{
			vector pos = GetPosition();
			for (int i = 0; i < count; i++)
			{
				if (vector.Distance(pos,temp.Get(i)) < GameConstants.REFRESHER_RADIUS)
					return true;
			}
			
			return false;
		}
		else
		{
			return false;
		}
	}
	
	void Claim(PlayerIdentity identity)
	{
		if (!identity)
		{
			return; 
		}
		
		if ((m_vst_neo_sharerID) && (m_vst_neo_sharerID == identity))
		{
			vst_neo_send_player_message(identity, "barrel share disabled");
			m_vst_neo_sharerID = null;
		}
		
		// if protections are disabled don't allow claiming
		if ((isProtectionDisabled()) || (!isLockable()))
		{
			return;
		}
		
		float mindist = g_Game.GetVSTConfig().Get_min_distance_from_spawn_to_lock();
		if ((mindist != 0.0) && (m_vst_ce_spawn_position != vector.Zero))
		{
			mindist = mindist * mindist; // now squared
			if (vector.DistanceSq(GetPosition(), m_vst_ce_spawn_position) < mindist)
			{
				vst_neo_send_tooclosetospawn_notification(identity);
				return;
			}

		}
		
		// is player trying to lock too many barrels (if limit is <=0, then no limit)
		string steamid = identity.GetPlainId();		
		int max_claims = g_Game.GetVSTConfig().Get_max_barrels_per_player();
		if (max_claims > 0)
		{
			if (s_vst_neo_user_barrel_count)
			{
				int currentcount = 0;
				if (s_vst_neo_user_barrel_count.Find(steamid, currentcount))
				{
					if (currentcount >= max_claims)
					{
						vst_neo_send_toomanylocked_notification(identity);
						return;
					}
				}
			}
		}
		
		string playerName = identity.GetName();
		
		int steamid_hash = steamid.Hash();
		
		if(g_Game.GetVSTConfig().Get_script_logging() == 1)
		{
			Print("[vStorage] player "+steamid+ "claimed barrel "+GetType()+" at pos "+GetPosition());
		}

		
		if (m_vst_owner_names.Find(playerName) == -1)
		{
			m_vst_owner_names.Insert(playerName);
			m_vst_metadata_updated = true;
		}
		if (m_vst_owner_steamids.Find(steamid) == -1)
		{
			m_vst_owner_steamids.Insert(steamid);
			m_vst_metadata_updated = true;
			
			vst_neo_add_barrel_user_count(steamid);
		}
		
		if (m_vst_owner_steamid_hashes.Find(steamid_hash) == -1)
		{
			m_vst_owner_steamid_hashes.Insert(steamid_hash);
		}
		if (!m_vst_wasplaced)
		{
			m_vst_wasplaced = true;
			m_vst_metadata_updated = true;
		}

		if (m_vst_metadata_updated)
		{
			vst_neo_save_metadata();
		}
		
		vst_neo_send_claim_notification(identity);
		
		if (m_vst_neo_sharerID)
		{
			vst_neo_send_five_per_line(identity, "Updated Owners: ", m_vst_owner_names);
			vst_neo_send_five_per_line(m_vst_neo_sharerID, "Updated Owners: ", m_vst_owner_names);
			m_vst_neo_sharerID = null;
		}
	}
	
	void Unclaim(PlayerIdentity identity = null) 
	{
		m_vst_wasplaced = false;
		m_vst_owner_names.Clear();
		
		foreach (string steamid: m_vst_owner_steamids)
		{
			vst_neo_subtract_barrel_user_count(steamid);
		}
		
		m_vst_owner_steamids.Clear();
		m_vst_owner_steamid_hashes.Clear();
		if (identity) // if administratively unlocked via cftools/command ID will be null
		{
			vst_neo_send_unclaim_notification(identity);
		}
		m_vst_metadata_updated = true;
		vst_neo_save_metadata();
	}

	void vst_timer_start(bool express = false)
	{

		if(!((m_auto_close_random_seconds_min > 0) && (m_auto_close_random_seconds_max > m_auto_close_random_seconds_min)))
			return;
			
		if(IsOpen())
		{
			if ((m_vst_neo_autoclose_timer) && (m_vst_neo_autoclose_timer.IsRunning()))
			{
				return; // already running
			}
			
			int autoclose_timer;
			if(express)
			{
				autoclose_timer = 20;
			}
			else
			{
				autoclose_timer = Math.RandomInt(m_auto_close_random_seconds_min, m_auto_close_random_seconds_max);
			}
			//GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(vst_timer_end, autoclose_timer, false);
			if (!m_vst_neo_autoclose_timer)
			{
				m_vst_neo_autoclose_timer = new Timer();
			}
			m_vst_neo_autoclose_timer.Run(autoclose_timer, this, "vst_timer_end", null, false);
			if(g_Game.GetVSTConfig().Get_script_logging() == 1)
				Print("[vStorage] Starting " + autoclose_timer +" ms autoclose timer for " + GetType() + " at Position " +GetPosition() );
		}
		
	}

	void vst_timer_end()
	{
		if(!IsOpen()) return;

		GameInventory gi;
		CargoBase cb;
		
		gi = GetInventory();
		
		if (!gi)
		{
			return;
		}
		
		cb = gi.GetCargo();
		if (!cb)
		{
			return;
		}
		
		// do not auto close an empty barrel that may be a rain barrel
		if (cb.GetItemCount() == 0)
			return;
			
		bool PlayerIsAround = false;

		CEApi ce = GetCEApi();
		if(ce)
		{
			// enfusion modders discord swears this is faster than getting all objects 
			if (ce.AvoidPlayer(GetPosition(), m_auto_close_distance))
			{
				PlayerIsAround = false;
			}
			else 
			{
				PlayerIsAround = true;
			}
		}
		else
		{
			// if we can't get a CE interface, use the original method
			array<Object> items_in_vicinity = new array<Object>;
			GetGame().GetObjectsAtPosition(GetPosition(), m_auto_close_distance, items_in_vicinity, NULL);
			
			for (int i = 0; i < items_in_vicinity.Count(); i++)
			{
				EntityAI item_in_vicinity = EntityAI.Cast(items_in_vicinity.Get(i));
				if (item_in_vicinity && item_in_vicinity.IsKindOf("SurvivorBase"))
				{
					PlayerIsAround = true;
					break;
				}
			}
		}

		if(!PlayerIsAround)
		{
			if(g_Game.GetVSTConfig().Get_script_logging() == 1)
				Print("[vStorage] No player(s) around, autoclosing "+GetType()+" at pos "+GetPosition());
			vclose();
			Close();
		} else
		{
			if(g_Game.GetVSTConfig().Get_script_logging() == 1)
				Print("[vStorage] Player(s) around near "+GetType()+" at pos "+GetPosition()+", not closing, restarting timer");
			
			vst_timer_start(false);
		}
	}

	void setItems(bool items)
	{
		if (m_vst_hasitems != items)
		{
			m_vst_metadata_updated = true;
		}
		m_vst_hasitems = items;
		if (m_vst_metadata_updated)
		{
			vst_neo_save_metadata();
		}
	}
	
	override void OnStoreSave(ParamsWriteContext ctx)
	{		
		super.OnStoreSave(ctx);
		if(m_vst_metadata_updated) // only save if there is an unsaved update
		{
			vst_neo_save_metadata();
		}
	}
	
	// moved OnStoreLoad to AfterStoreLoad as that's when GetPersistentID is valid
	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();
		vst_neo_load_metadata();
		vst_timer_start();
	}
	
	
	bool vst_IsOnBlacklist(EntityAI item)
	{
		
		/*
		if(!m_BlackListItems || m_BlackListItems.Count() == 0) {
			//Print("[vStorage] m_BlackListItems array nicht bekannt oder leer ");
			return false;
		}
		*/
		
		int i;
		string BlackListClass;

		for (i = 0; i < g_Game.GetVSTConfig().Get_Blacklist().Count(); i++)
		{
			BlackListClass = g_Game.GetVSTConfig().Get_Blacklist().Get(i);
			if(item.IsKindOf(BlackListClass)) 
			{
				//Print("[vStorage] Found Item "+BlackListClass+" IN blacklist");
				return true;
			}
			
		}

		array<EntityAI> items_in_storage = new array<EntityAI>;
		EntityAI item_in_storage;

		item.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items_in_storage);

		for (i = 0; i < items_in_storage.Count(); i++)
		{
			item_in_storage = items_in_storage.Get(i);
			if (item_in_storage && vst_IsOnBlacklist(item_in_storage))
			{
				//Print("[vStorage] Found Child Item "+BlackListClass+" IN blacklist");
				return true;
			}
		};

		//Print("[vStorage] ITEM "+item+" IS NOT ON BLACKLIST");
		return false;
	}

//	override bool CanReceiveItemIntoCargo (EntityAI item)
//    {
//		return !vst_IsOnBlacklist(item);
///    }
    void vst_neo_throwout_blacklist_item(EntityAI item)
	{
		if(!item)
		{
			return;
		}
		
		//derived from MiscGameplayFunctions.DropAllItemsInInventoryInBounds
		ItemBase ib = this; // cast is uneccessary and will produce compiler warning
		vector halfExtents = m_HalfExtents;
		autoptr array<EntityAI> items = new array<EntityAI>;
		ib.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);
		
		vector direction = ib.GetDirection();
		float dot = vector.Dot(direction, vector.Forward);
		
		float angle = Math.Acos(dot);	
		if (direction[0] < 0)
			angle = -angle;	

		float cos = Math.Cos(angle);
		float sin = Math.Sin(angle);
		
		EntityAI itemtothrow;
		int count = items.Count();
		for ( int i = 0; i < count; ++i )
		{
			itemtothrow = items.Get(i);
			if ((item) && (item == itemtothrow))
				ib.GetInventory().DropEntityInBounds(InventoryMode.SERVER, ib, itemtothrow, halfExtents, angle, cos, sin);
		}
		
		vst_neo_send_blacklist_notification(item);
	}
	
	override void EECargoIn(EntityAI item)
	{
		super.EECargoIn(item);
		
		// do not reject items being restored
		if (m_vst_neo_is_restoring)
		{
			return;
		}
		
		if (!vst_IsOnBlacklist(item))
		{
			return;
		}
		
		/* give item a tenth of a second to settle other calls in the place cargo change */
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(vst_neo_throwout_blacklist_item, 10, false, item);
		
	}
	
	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
	{
		if (newLevel == GameConstants.STATE_RUINED && !GetHierarchyParent() && !IsOpen())
		{
			vopen(null, "");
		}
		super.EEHealthLevelChanged(oldLevel,newLevel,zone);
	}
	
	override void EEDelete(EntityAI parent)
	{
		string filename = vst_neo_get_save_filename();
		
		if (FileExist(filename))
		{
			DeleteFile(filename);
		}
		
		filename = vst_neo_get_meta_filename();
		
		if (FileExist(filename))
		{
			DeleteFile(filename);
		}
		super.EEDelete(parent);
	}
	
	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		if (!m_vst_wasplaced)
		{
			return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		}
		
		if (isProtectionDisabled())
		{
			return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
		}
		
		PlayerBase playerSource;
		playerSource = PlayerBase.Cast( source.GetHierarchyParent() );
		PlayerIdentity pi;
		if (playerSource)
		{
			pi = playerSource.GetIdentity();
		}
		if (pi)
		{
			if (canInteract(pi) || canInteractAdmin(pi, false))
			{
				return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
			}
			vst_neo_send_locked_notification(pi);
		}
		
		return false;
	}
	
	override void EEOnCECreate()
	{
		super.EEOnCECreate();
		if (m_vst_ce_spawn_position != vector.Zero)
		{
			Print("[NEO BARRELS] WARNING: got EEOnCECreate twice at " + GetPosition());
		}
		m_vst_ce_spawn_position = GetPosition();
		m_vst_metadata_updated = true;
		vst_neo_save_metadata();
	}
    // commented out, place holder object should handle this now
	//override void SetTakeable(bool pState)
	//{
		//if (m_vst_hasitems)
		//{
		//	super.SetTakeable(false); // This syncs to client so should block moving barrels that aren't really "empty"
		//	return;
		//}
		//super.SetTakeable(pState);
	//}
		
	// commented out, place holder should handle this now
	//override bool IsEmpty()
	//{
		// block poking holes in a barrel that's not "actually" empty in the call to the pokeholesbarrel recipe's CanDo() function
		//if (m_vst_hasitems)
		//{
		//	return false;
		//}
		//return super.IsEmpty();
	//}
	
	bool vopen(PlayerBase player, string steamid = "")
	{
		
		m_vst_neo_is_restoring = true;
		
		bool failedItems = false;
		
		string filename;
		
		if(this.GetType() != "tofu_vstorage_q_barrel_express")
		{
			filename = vst_neo_get_save_filename();
		}
		else
		{
			filename = steamid+".save";
		}
		
		if (!FileExist(filename))
		{
			filename = vst_neo_get_save_filename_V1(); // v1 files should be compatible, set filename so it is loaded and deleted at end.
			if (!FileExist(filename))
			{
				m_vst_neo_is_restoring = false;
				return true; // no saved data to restore (avoids clearing contents)
			}
		}
		if(g_Game.GetVSTConfig().Get_script_logging() == 1)
			Print("hasitems on restore: " + m_vst_hasitems);
		// clear place holder object if it is supposed to be empty
		if (m_vst_hasitems)
		{
			autoptr array<EntityAI> items_to_store = new array<EntityAI>;
			GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items_to_store);
			int count = items_to_store.Count();
			if(g_Game.GetVSTConfig().Get_script_logging() == 1)
			{
				Print("restoring, found " + count + "items in restore step of empty storage");
			}
			for (int j= 0; j < count; j++)
			{
				EntityAI item_in_storage_to_delete = items_to_store.Get(j);
				if ((item_in_storage_to_delete) && (item_in_storage_to_delete.GetType() == s_vst_neo_placeholder_type))
				{
					if(g_Game.GetVSTConfig().Get_script_logging() == 1)
					{
						Print("deleting " + item_in_storage_to_delete.GetDisplayName());
					}
					//item_in_storage_to_delete.Delete();
					g_Game.ObjectDelete(item_in_storage_to_delete); // don't defer deletion, need place holder gone to make room for cargo
				}
			}
		}
		
		FileSerializer openfile = new FileSerializer();
		autoptr tofuvStorageContainer loadedContainerObj = new tofuvStorageContainer() ;
		
		if (openfile.Open(filename, FileMode.READ)) {
			if(openfile.Read(loadedContainerObj)) {
				foreach(tofuvStorageObj item : loadedContainerObj.storedItems) {
					if(!vrestore(item, this, player))
						failedItems = true;
				}
			}
			openfile.Close();
			
			if(this.GetType() == "tofu_vstorage_q_barrel_express" || this.GetType() == "tofu_vstorage_q_barrel_travel")
			{
				if (FileExist(filename)) {
					DeleteFile(filename);
					//Print("[vStorage] DELETED FILE "+filename_q);
				}
			}
			
		}
		
		
		if(GetGame().IsDedicatedServer())
		{
			if(this.GetType() != "tofu_vstorage_q_barrel_express")
			{
				vst_timer_start(false);
				
				if(g_Game.GetVSTConfig().Get_script_logging() == 1)
					Print("[vStorage] Player "+steamid+" opened Barrel "+GetType()+" at position "+GetPosition());
			}
			
			//SoundSynchRemoteReset();
		}
		
		// if someone destroys a closed barrel, don't give them their stuff twice if they open it
		// also may lead to duplication on a server crash
		filename = vst_neo_get_save_filename();
		
		if (FileExist(filename))
		{
			DeleteFile(filename);
		}
		
		m_vst_neo_is_restoring = false;
		
		if(failedItems) 
		{
			return false;
		}
		
		
		return true;
		
	}

	void vclose(string steamid = "")
	{
		if(m_vst_neo_is_restoring)
		{
			return;
		}
		
		autoptr tofuvStorageContainer containerObj = new tofuvStorageContainer();
		
		int b1;
		int b2;
		int b3;
		int b4;
		string filename;
		GetPersistentID(b1, b2, b3, b4);
		string persistentIdToSave = "container_" + b1 + "_" + b2 + "_" + b3 + "_" + b4;
		
		if(this.GetType() != "tofu_vstorage_q_barrel_express")
		{
			filename = vst_neo_get_save_filename();
		}
		else
		{
			//filename = vst_neo_get_express_filename();
			return;
		}
		
		
		containerObj.persistentId = persistentIdToSave;
		containerObj.storedItems = new array<ref tofuvStorageObj>;
		
		autoptr array<EntityAI> items = new array<EntityAI>;
		GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);

		int count = items.Count();
		int original_count = count;
		for (int i = 0; i < count; i++)
		{
			EntityAI item_in_storage = items.Get(i);
			if (item_in_storage)
			{
				if(g_Game.GetVSTConfig().Get_script_logging() == 1)
				Print("[vStorage] closing found item: " + item_in_storage.GetDisplayName());
				containerObj.storedItems.Insert(vstore(item_in_storage));
			}
		}
		if(g_Game.GetVSTConfig().Get_script_logging() == 1)
		{
			Print("[vStorage] in close items check count = " + count + " original_count = " + original_count);
		}
		if(count == 1)
			setItems(false);
		else 
			setItems(true);

		//Print("[vStorage] vclose() ");
		
		FileSerializer file = new FileSerializer();
		if (file.Open(filename, FileMode.WRITE))
		{
			if(file.Write(containerObj))
			{
				autoptr array<EntityAI> items_to_store = new array<EntityAI>;
				GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items_to_store);
				count = items_to_store.Count();
				for (int j= 0; j < count; j++)
				{
					EntityAI item_in_storage_to_delete = items_to_store.Get(j);
					if (item_in_storage_to_delete) 
					{
						//item_in_storage_to_delete.Delete();
						g_Game.ObjectDelete(item_in_storage_to_delete); // don't defer deletion, make room for placeholder
					}
				}
			}
			file.Close();
			//Print("Content Serialized and saved");
		}
		if (m_vst_hasitems) 
		{
			// put a single place holder in there, so clients know it's not empty, we clear it on open
			GetInventory().CreateEntityInCargo(s_vst_neo_placeholder_type);
		}
		////Print("[vStorage] vclose() end");
	}

	tofuvStorageObj vstore(EntityAI item_in_storage)
	{
		autoptr tofuvStorageObj itemObj = new tofuvStorageObj();
		InventoryLocation item_in_storage_location = new InventoryLocation;
		item_in_storage.GetInventory().GetCurrentInventoryLocation( item_in_storage_location );
		ItemBase item_to_check = ItemBase.Cast(item_in_storage);
		
		itemObj.itemName = item_in_storage.GetType();
		itemObj.itemRow = item_in_storage_location.GetRow();
		itemObj.itemCol = item_in_storage_location.GetCol();
		itemObj.itemFliped = item_in_storage_location.GetFlip();
		itemObj.itemIdx = item_in_storage_location.GetIdx();
		itemObj.itemSlotId = item_in_storage_location.GetSlot();
		itemObj.itemInventoryType = item_in_storage_location.GetType();
		
		if(item_to_check.HasQuantity())
		{
			itemObj.itemQuantity = item_to_check.GetQuantity();
			if(item_to_check.IsLiquidContainer())
				itemObj.itemLiquidType = item_to_check.GetLiquidType();
		} else
		{
			itemObj.itemQuantity = 0;
			itemObj.itemLiquidType = 0;
		}
		
		if(item_in_storage.m_EM)
		{
			itemObj.itemEnergy =  Math.Ceil(item_in_storage.m_EM.GetEnergy());
			itemObj.itemHasEnergy = true;
		}
		
		Magazine magazine_check = Magazine.Cast(item_in_storage);
		Ammunition_Base ammo_check = Ammunition_Base.Cast(item_in_storage);

		if(item_in_storage.IsMagazine() && !(ammo_check && ammo_check.IsAmmoPile()))
		{
			itemObj.itemType = "magazine";
			itemObj.itemAmmoQuantity = magazine_check.GetAmmoCount();
			
			for (int f = 0; f < magazine_check.GetAmmoCount(); f++)
			{
				float dmg;
				string class_name;
				magazine_check.GetCartridgeAtIndex(f, dmg, class_name);
				string ma_temp = f.ToString() + "," + class_name;
				
				itemObj.itemMagInhalt.Insert(ma_temp);
			}
			//Print("MAGAZINE AMMO: "+magazine_check.GetAmmoCount());
		}
		
		array<string> a_itemWpnBarrelInfo = new array<string>;
		array<string> a_itemWpnInternalMagInfo = new array<string>;
				
		if(item_in_storage.IsWeapon())
		{
			Weapon_Base wpn = Weapon_Base.Cast(item_in_storage);
			float damage = 0.0;
			string type;
			string itemWpnBarrelInfo;
			string itemWpnInternalMagInfo;
			
			for (int mi = 0; mi < wpn.GetMuzzleCount(); ++mi)
			{
				if (!wpn.IsChamberEmpty(mi))
				{
					if (wpn.GetCartridgeInfo(mi, damage, type))
					{
						//Print ("[Im Lauf] " +mi+" "+damage+" "+type);
						string bi_temp = ""+mi+","+type;
						a_itemWpnBarrelInfo.Insert(bi_temp);
						//PushCartridgeToChamber(mi, damage, type);
					}
				}
				
				for (int ci = 0; ci < wpn.GetInternalMagazineCartridgeCount(mi); ++ci)
				{
					if (wpn.GetInternalMagazineCartridgeInfo(mi, ci, damage, type))
					{
						//Print ("[In internen Mag] " +mi+" "+ci+" "+damage+" "+type);
						string ci_temp = ""+mi+","+type;
						a_itemWpnInternalMagInfo.Insert(ci_temp);
						//PushCartridgeToInternalMagazine(mi, damage, type);
					}
				}
			}
			
			if(a_itemWpnBarrelInfo.Count() > 0)
			{
				for (int bi = 0; bi < a_itemWpnBarrelInfo.Count(); bi++)
				{
					itemWpnBarrelInfo += a_itemWpnBarrelInfo.Get(bi)+"|";
				}
				itemObj.itemWpnBarrelInfo = itemWpnBarrelInfo;
				//Print(itemWpnBarrelInfo);
			}
			
			if(a_itemWpnInternalMagInfo.Count() > 0)
			{
				for (int im = 0; im < a_itemWpnInternalMagInfo.Count(); im++)
				{
					itemWpnInternalMagInfo += a_itemWpnInternalMagInfo.Get(im)+"|";
				}
				itemObj.itemWpnInternalMagInfo = itemWpnInternalMagInfo;
				//Print(itemWpnInternalMagInfo);
			}
		}
		
		if(ammo_check && ammo_check.IsAmmoPile())
		{
			itemObj.itemAmmoQuantity = ammo_check.GetAmmoCount();
		}
		
		itemObj.itemHealth = item_in_storage.GetHealth();
						
		if(item_in_storage.IsFood())
		{
			Edible_Base item_to_check_food = Edible_Base.Cast(item_in_storage);
			if(item_to_check_food)
			{
				itemObj.itemFoodstage = item_to_check_food.GetFoodStageType();
			}
		}
		
		if (item_to_check.m_vst_neo_is_easter_egg)
		{
			EasterEgg ee = EasterEgg.Cast(item_in_storage);
			if (ee)
			{
				itemObj.itemEECaptureState = ee.vst_neo_GetCaptureState();
				itemObj.itemEECreatureType = ee.vst_neo_GetCreatureType();
				itemObj.itemWetness = ee.vst_neo_GetParScale(); // re-purposed variable
				itemObj.itemEEDangerSound = ee.vst_neo_GetDangerSound();
				itemObj.itemEECapSoundHash = ee.vst_neo_GetCaptureSoundHash();
				itemObj.itemEERelSoundHash = ee.vst_neo_GetReleaseSoundHash();
				itemObj.itemEEIsEgg = true;
			}
			else
			{
				Print("[NEO Barrels] object claiming to be easter egg was not");
			}
		}
		else
		{
			itemObj.itemEEIsEgg = false;
		}
		
		if (item_to_check.m_vst_neo_is_paper)
		{
			WrittenNoteData wnd = item_to_check.GetWrittenNoteData();
			if (wnd)
			{
				itemObj.itemPprNoteData = wnd.GetNoteText();
			}
			itemObj.itemPprIsPaper = true;
		}
			
		itemObj.itemTemp = item_to_check.GetTemperature();
		
		if (!item_to_check.m_vst_neo_is_easter_egg) // wetness used in eastereggs for m_ParScale
		{
			itemObj.itemWetness = item_to_check.GetWet();
		}		
		
		array<EntityAI> items = new array<EntityAI>;
		item_in_storage.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);
		int count = items.Count();
		for (int i = 0; i < count; i++)
		{
			EntityAI item_in_storage_child = items.Get(i);
			if (item_in_storage_child) {
				itemObj.itemChildren.Insert(vstore(item_in_storage_child));
				m_didVStorage = true;
			}
		};
		
		
		/*
		Print("------- NEW ITEM ---------");
		Print(itemObj.itemName);
		Print(itemObj.itemType);
		Print(itemObj.itemRow);
		Print(itemObj.itemCol);
		Print(itemObj.itemFliped);
		Print(itemObj.itemQuantity);
		Print(itemObj.itemAmmoQuantity);
		Print(itemObj.itemHealth);
		Print(itemObj.itemLiquidType);
		Print(itemObj.itemFoodstage);
		Print(itemObj.itemTemp);
		Print(itemObj.itemWetness);
		Print(itemObj.itemInventoryType);
		Print(itemObj.itemIdx);
		Print(itemObj.itemSlotId);
		Print(itemObj.itemChildren);
		Print("----------------");
		*/
		
		return itemObj;
	}
	
	bool vrestore(tofuvStorageObj item, Object target_object, PlayerBase player)
	{
		
		
		bool itemFailed = false;
		bool indexError = false;
		
		EntityAI ntarget = EntityAI.Cast( target_object );
		ItemBase new_item;
	
		if(item.itemSlotId == -1) {
			
			if(g_Game.GetVSTConfig().Get_script_logging() == 1)
			{
				Print("[vStorage] Try Creating " + item.itemName + " in Parent " + ntarget.GetType());
				Print("[vStorage] Try " + item.itemIdx + " item.itemIdx ");
				Print("[vStorage] Try " + item.itemRow + " item.itemRow ");
				Print("[vStorage] Try " + item.itemCol + " item.itemCol ");
				Print("[vStorage] Try " + item.itemFliped + " item.itemFliped ");
			}
			
				
			GameInventory safty_iventory = GameInventory.Cast(ntarget.GetInventory());
			if(safty_iventory)
			{
				if(g_Game.GetVSTConfig().Get_script_logging() == 1)
					Print("[vStorage] safty_iventory okay");
				
				
				if(item.itemIdx == -1)
				{
					if(g_Game.GetVSTConfig().Get_script_logging() == 1)
						Print("[vStorage] !!!!!!!!!!!!!!item.itemIdx -1, Abort");
					
					indexError = true;
				}
				
				if(item.itemRow == -1)
				{
					if(g_Game.GetVSTConfig().Get_script_logging() == 1)
						Print("[vStorage] !!!!!!!!!!!!item.itemRow -1, Abort");
					
					indexError = true;
				}
				
				if(item.itemCol == -1)
				{
					if(g_Game.GetVSTConfig().Get_script_logging() == 1)
						Print("[vStorage] !!!!!!!!!!!!item.itemCol -1, Abort");
					
					indexError = true;
				}
				
				if(!indexError)				
				{	
					new_item = ItemBase.Cast(safty_iventory.CreateEntityInCargoEx(item.itemName,item.itemIdx,item.itemRow,item.itemCol,item.itemFliped));
				}
				else
				{
					new_item = ItemBase.Cast(GetGame().CreateObject(item.itemName, ntarget.GetPosition(),false,false,true));
					if(!new_item)
					{
						Print("[vStorage] !!!!!!!!Failed creating Item "+item.itemName+" "+new_item+" on ground.");
						return false;
					}
					else
					{
						if(g_Game.GetVSTConfig().Get_script_logging() == 1)
							Print("[vStorage] Item "+item.itemName+" "+new_item+" created on ground.");
					}
				}
			}
			else
			{
				if(g_Game.GetVSTConfig().Get_script_logging() == 1)
					Print("[vStorage] !!!!!!!!!!!!!! Failed casting Inventory");
				
				return false;
			}
			
			
			if(new_item) {
				if(g_Game.GetVSTConfig().Get_script_logging() == 1)
					Print("[vStorage] Item "+item.itemName+" "+new_item+" Created, preparing to set properties.");
			} else {
				if(g_Game.GetVSTConfig().Get_script_logging() == 1)
					Print("[vStorage] Item "+item.itemName+" "+new_item+" NOT created, try alternative way");
				
				
				GameInventory testinv = ntarget.GetInventory();
				if(!testinv) {
					if(g_Game.GetVSTConfig().Get_script_logging() == 1)
						Print("[vStorage] No Inventory found for Item "+item.itemName+" "+new_item+"");
				} else {
					testinv.UnlockInventory(HIDE_INV_FROM_SCRIPT);   
					new_item = ItemBase.Cast(testinv.CreateEntityInCargoEx(item.itemName,item.itemIdx,item.itemRow,item.itemCol,item.itemFliped));
					if(new_item) {
						if(g_Game.GetVSTConfig().Get_script_logging() == 1)
							Print("[vStorage] Item "+item.itemName+" "+new_item+" created in retry");
					} else {
						if(g_Game.GetVSTConfig().Get_script_logging() == 1)
							Print("[vStorage] Item "+item.itemName+" "+new_item+" NOT created in retry");
						
						new_item = ItemBase.Cast(GetGame().CreateObject(item.itemName, ntarget.GetPosition(),false,false,true));
						if(!new_item)
						{
							Print("[vStorage] !!!!!!!!Failed creating Item "+item.itemName+" "+new_item+" on ground.");
							return false;
						}
						else
						{
							if(g_Game.GetVSTConfig().Get_script_logging() == 1)
								Print("[vStorage] Item "+item.itemName+" "+new_item+" created on ground.");
						}
						
					}
				}
			}
		} else {
			if(ntarget.IsWeapon() && item.itemType == "magazine") {
				
				if(g_Game.GetVSTConfig().Get_script_logging() == 1)
					Print("[vStorage] Item "+item.itemName+" "+new_item+" is Magazine, parent is weapon.");
				
				Weapon_Base weapon = Weapon_Base.Cast(ntarget);
				
				
				weapon.SpawnAmmo( item.itemName, 0 );
				
				array<EntityAI> items = new array<EntityAI>;
				weapon.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, items);
				int count = items.Count();
				for (int i = 0; i < count; i++)
				{
					ItemBase item_in_storage = ItemBase.Cast(items.Get(i));
					if (item_in_storage)
					{
						Magazine magazine_check = Magazine.Cast(item_in_storage);
						Ammunition_Base ammo_check = Ammunition_Base.Cast(item_in_storage);
						if(item_in_storage.IsMagazine() && !(ammo_check && ammo_check.IsAmmoPile())) {
							magazine_check.ServerSetAmmoCount(0);
							
							array<string> itemMagInhalt3 = item.itemMagInhalt;
					
							for (int ae = 0; ae < itemMagInhalt3.Count(); ae++)
							{
								string part_itemMagInfo3 = itemMagInhalt3.Get(ae);
								if(part_itemMagInfo3 != "")
								{
									TStringArray a_part_itemMagInfo3 = new TStringArray;
									part_itemMagInfo3.Split(",", a_part_itemMagInfo3);
															
									if ( magazine_check.ServerStoreCartridge(0.0, a_part_itemMagInfo3[1]))
									{
										//Print("Stored Ammo in Mag");
									}
								}
							}
							magazine_check.SetSynchDirty();
							
							ItemBase mag_stats = ItemBase.Cast(item_in_storage);
							
							mag_stats.SetHealth(item.itemHealth);
							
							mag_stats.SetTemperature(item.itemTemp);
							
							
							mag_stats.SetWet(item.itemWetness);
						}
					}
				}
			} else {
				if(item.itemType == "magazine") {
					new_item = ItemBase.Cast(ntarget.GetInventory().CreateAttachmentEx(item.itemName,item.itemSlotId));
					Magazine magazine_check3 = Magazine.Cast(new_item);
					Ammunition_Base ammo_check2 = Ammunition_Base.Cast(new_item);
					if(new_item && new_item.IsMagazine() && !(ammo_check2 && ammo_check2.IsAmmoPile())) {
						magazine_check3.ServerSetAmmoCount(0);
						
						array<string> itemMagInhalt2 = item.itemMagInhalt;
					
						for (int ad = 0; ad < itemMagInhalt2.Count(); ad++)
						{
							string part_itemMagInfo2 = itemMagInhalt2.Get(ad);
							if(part_itemMagInfo2 != "")
							{
								TStringArray a_part_itemMagInfo2 = new TStringArray;
								part_itemMagInfo2.Split(",", a_part_itemMagInfo2);
														
								if ( magazine_check3.ServerStoreCartridge(0.0, a_part_itemMagInfo2[1]))
								{
									//Print("Stored Ammo in Mag");
								}
							}
						}
						magazine_check3.SetSynchDirty();
						
						ItemBase mag_stats2 = ItemBase.Cast(new_item);
						
						
						mag_stats2.SetHealth(item.itemHealth);
						
						
						mag_stats2.SetTemperature(item.itemTemp);
						
						
						mag_stats2.SetWet(item.itemWetness);
					}
					
				} else {
					new_item = ItemBase.Cast(ntarget.GetInventory().CreateAttachmentEx(item.itemName,item.itemSlotId));
				}
			}
		}
		
		if(new_item)
		{
			if(g_Game.GetVSTConfig().Get_script_logging() == 1)
				Print("[vStorage] Item "+item.itemName+" "+new_item+" setting properties.");
			
			if(new_item.IsWeapon())
			{
				if(g_Game.GetVSTConfig().Get_script_logging() == 1)
					Print("[vStorage] Item  "+item.itemName+" "+new_item+" is weapon.");
				
				//Print("Item is Weapon");
				bool didChamberAction = false;
				Weapon_Base wpn = Weapon_Base.Cast(new_item);
				
				string itemWpnBarrelInfo = item.itemWpnBarrelInfo;
				
				TStringArray a_itemWpnBarrelInfo = new TStringArray;
				itemWpnBarrelInfo.Split("|", a_itemWpnBarrelInfo);
				for (int ab = 0; ab < a_itemWpnBarrelInfo.Count(); ab++)
				{
					string part_itemWpnBarrelInfo = a_itemWpnBarrelInfo.Get(ab);
					if(part_itemWpnBarrelInfo != "")
					{
						TStringArray a_part_itemWpnBarrelInfo = new TStringArray;
						part_itemWpnBarrelInfo.Split(",", a_part_itemWpnBarrelInfo);
						if(wpn.PushCartridgeToChamber(a_part_itemWpnBarrelInfo[0].ToInt(), 0.0, a_part_itemWpnBarrelInfo[1]))
						{
							//Print("Loaded Ammo to Chamber");
							didChamberAction = true;
						}
						else
						{
							//Print("NOT Loaded Ammo to Chamber");
						}
					}
				}
				
				string itemWpnInternalMagInfo = item.itemWpnInternalMagInfo;
				
				TStringArray a_itemWpnInternalMagInfo = new TStringArray;
				itemWpnInternalMagInfo.Split("|", a_itemWpnInternalMagInfo);
				for (int ib = 0; ib < a_itemWpnInternalMagInfo.Count(); ib++)
				{
					string part_itemWpnInternalMagInfo = a_itemWpnInternalMagInfo.Get(ib);
					if(part_itemWpnInternalMagInfo != "")
					{
						TStringArray a_part_itemWpnInternalMagInfo = new TStringArray;
						part_itemWpnInternalMagInfo.Split(",", a_part_itemWpnInternalMagInfo);
						if(wpn.PushCartridgeToInternalMagazine(a_part_itemWpnInternalMagInfo[0].ToInt(), 0.0, a_part_itemWpnInternalMagInfo[1]))
						{
							//Print("Loaded Ammo to intenal Mag");
							didChamberAction = true;
						}
						else
						{
							//Print("NOT Loaded Ammo to intenal Mag");
						}
					}
				}
				
				if(didChamberAction)
				{
					if (g_Game.IsServer())
						wpn.RandomizeFSMState();
						
					wpn.Synchronize();
					
					/*
					RandomizeFSMState();
					Synchronize();
					*/
				}
				
				
				
				//PushCartridgeToChamber(int muzzleIndex, float ammoDamage, string ammoTypeName);
			} else {
				//Print("Item is NOT Weapon");
			}
			
			if(new_item.HasQuantity()) {
				new_item.SetQuantity( item.itemQuantity );
			}
			new_item.SetLiquidType(item.itemLiquidType);
			
			if(item.itemLiquidType >= 1 && item.itemLiquidType <= 128) {
				BloodContainerBase bloodBag = BloodContainerBase.Cast(new_item);
				bloodBag.SetBloodTypeVisible(true);
			}
			
			if(item.itemHasEnergy)
			{
				new_item.m_EM.SetEnergy(item.itemEnergy);
			}
			
			if(new_item.IsAmmoPile()) {
				Magazine mgzn = Magazine.Cast(new_item);
				mgzn.ServerSetAmmoCount(item.itemAmmoQuantity);
			}
			
			if(item.itemType == "magazine") {
				bool is_single_or_server = !GetGame().IsMultiplayer() || GetGame().IsServer();
				if (is_single_or_server)
				{
					Magazine magazine_check2 = Magazine.Cast(new_item);
					magazine_check2.ServerSetAmmoCount(0);
					
					
					array<string> itemMagInhalt = item.itemMagInhalt;
					
					for (int ac = 0; ac < itemMagInhalt.Count(); ac++)
					{
						string part_itemMagInfo = itemMagInhalt.Get(ac);
						if(part_itemMagInfo != "")
						{
							TStringArray a_part_itemMagInfo = new TStringArray;
							part_itemMagInfo.Split(",", a_part_itemMagInfo);
													
							if ( magazine_check2.ServerStoreCartridge(0.0, a_part_itemMagInfo[1]))
							{
								//Print("Stored Ammo in Mag");
							}
						}
					}
					magazine_check2.SetSynchDirty();
				}
			}
						
			if(new_item.IsFood()) {
				Edible_Base new_item_food = Edible_Base.Cast(new_item);
				new_item_food.ChangeFoodStage(item.itemFoodstage);
			}

			new_item.SetHealth(item.itemHealth);
			
			
			new_item.SetTemperature(item.itemTemp);
			
			
			if (item.itemEEIsEgg)
			{
				EasterEgg ee = EasterEgg.Cast(new_item);
				if (ee)
				{
					ee.vst_neo_SetCaptureState(item.itemEECaptureState);
					ee.vst_neo_SetCreatureType(item.itemEECreatureType);
					ee.vst_neo_SetParScale(item.itemWetness);
					ee.vst_neo_SetDangerSound(item.itemEEDangerSound);
					ee.vst_neo_SetCaptureSoundHash(item.itemEECapSoundHash);
					ee.vst_neo_SetReleaseSoundHash(item.itemEERelSoundHash);
					ee.SetSynchDirty();
				}				
				new_item.SetWet(0);
			}
			else // wetness used for ParScale on eastereggs
			{
				new_item.SetWet(item.itemWetness);
			}
			
			if (item.itemPprIsPaper)
			{
				Paper ppr = Paper.Cast(new_item);
				if (ppr)
				{
					WrittenNoteData wnd = ppr.GetWrittenNoteData();
					if (wnd)
					{
						wnd.SetNoteText(item.itemPprNoteData);
					}
				}
			}
						
			foreach(tofuvStorageObj childitem : item.itemChildren) {
				vrestore(childitem, new_item, player);
			}
		}
		else
		{
			
			if(!(ntarget.IsWeapon() && item.itemType == "magazine"))
			{
				Print("[vStorage] !!!!!!!!!!!!!! Failed creating item "+item.itemName+" :");
				Print("[vStorage] DEBUG " + item.itemName + " in Parent " + ntarget.GetType());
				Print("[vStorage] DEBUG " + item.itemIdx + " item.itemIdx ");
				Print("[vStorage] DEBUG " + item.itemRow + " item.itemRow ");
				Print("[vStorage] DEBUG " + item.itemCol + " item.itemCol ");
				Print("[vStorage] DEBUG " + item.itemFliped + " item.itemFliped ");
				itemFailed = true;
				
				if(player && player.GetIdentity())
				 	NotificationSystem.SendNotificationToPlayerIdentityExtended(player.GetIdentity(), 1.0, "WARNING", "WARNING "+ item.itemName+ " MIGHT NOT BE CREATED!", "set:dayz_inventory image:barrel");
				
				return false;
			}
		}
		
		return true;
		
	}
};

// make display name for written note clear why it can't go into storage
modded class Paper
{
	void Paper()
	{
		m_vst_neo_is_paper = true;
	}
}

modded class ItemBase
{
	bool m_vst_neo_is_easter_egg = false;
	bool m_vst_neo_is_paper = false;
}

modded class EasterEgg
{
	void EasterEgg()
	{
		m_vst_neo_is_easter_egg = true;
	}
	int vst_neo_GetCaptureState()
	{
		return m_CaptureState;
	}
	void vst_neo_SetCaptureState(int capstate)
	{
		m_CaptureState = capstate;
	}
	string vst_neo_GetCreatureType()
	{
		return m_CreatureType;
	}
	void vst_neo_SetCreatureType(string creaturetype)
	{
		m_CreatureType = creaturetype;
	}
	float vst_neo_GetParScale()
	{
		return m_ParScale;
	}
	void vst_neo_SetParScale(float parscale)
	{
		m_ParScale = parscale;
	}
	bool vst_neo_GetDangerSound()
	{
		return m_DangerSound;
	}
	void vst_neo_SetDangerSound(bool dangersound)
	{
		m_DangerSound = dangersound;
	}
	int vst_neo_GetCaptureSoundHash()
	{
		return m_CaptureSoundHash;
	}
	void vst_neo_SetCaptureSoundHash(int hash)
	{
		m_CaptureSoundHash = hash;
	}
	int vst_neo_GetReleaseSoundHash()
	{
		return m_ReleaseSoundHash;
	}
	void vst_neo_SetReleaseSoundHash(int hash)
	{
		m_ReleaseSoundHash = hash;
	}
}


