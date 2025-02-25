// Change the name of this file to init.c to test neo tofu vstorage barrels

// starting with base init.c, adding a notification and chat command to run tests


ref array<Barrel_ColorBase> g_barrelArray;

void barrel_test_stage1 (PlayerBase player)
{
	if(!player)
	{
		return;
	}
	
	string test_string = "The virtual storage barrel constructor sets an auto close feature for non-empty barrels more than 10 meters from a player. ";
	test_string = test_string + "Four open red and one closed blue barrels with rags and four open green and one closed yellow empty barrels will spawn to your north, those further than ten meters should auto-close randomly after a minute from now. Empty barrels should stay open.";
	test_string = test_string + "PLEASE DO NOT MOVE!!! after 1 minute 5 seconds we will expect certain barrels to be closed";
	NotificationSystem.SendNotificationToPlayerIdentityExtended(
		player.GetIdentity(), 
		60.0,
		"Test Stage 1",
		test_string,
		"set:dayz_inventory image:barrel");
		
	// setup
	int old_min = g_Game.GetVSTConfig().Get_auto_close_random_seconds_min();
	int old_max = g_Game.GetVSTConfig().Get_auto_close_random_seconds_max();
	g_Game.GetVSTConfig().Set_auto_close_random_seconds_min(1);
	g_Game.GetVSTConfig().Set_auto_close_random_seconds_max(5);

	g_barrelArray = new array<Barrel_ColorBase>;
	int i;
	vector playerPos = player.GetPosition();
	vector pos;
	Barrel_ColorBase barrel;
	
	// rag barrels
	for (i = 1; i < 6; i++)
	{
		pos = playerPos;
		pos[2] = pos[2] + (i*3);
		barrel = Barrel_ColorBase.Cast(GetGame().CreateObject("Barrel_Red", pos));
		barrel.Open();
		barrel.GetInventory().CreateEntityInCargo("Rag");
		g_barrelArray.Insert(barrel);
	}
	
	pos = playerPos;
	pos[2] = pos[2] + 18;
	barrel = Barrel_ColorBase.Cast(GetGame().CreateObject("Barrel_Blue", pos));
	barrel.Close();
	barrel.GetInventory().CreateEntityInCargo("Rag");
	g_barrelArray.Insert(barrel);
	
	// empty barrels
	for (i = 1; i < 6; i++)
	{
		pos = playerPos;
		pos[1] = pos[1] + 1;
		pos[2] = pos[2] + (i*3);
		barrel = Barrel_ColorBase.Cast(GetGame().CreateObject("Barrel_Green", pos));
		barrel.Open();
		g_barrelArray.Insert(barrel);
	}
	
	pos = playerPos;
	pos[1] = pos[1] + 1;
	pos[2] = pos[2] + 18;
	barrel = Barrel_ColorBase.Cast(GetGame().CreateObject("Barrel_Yellow", pos));
	barrel.Close();
	g_barrelArray.Insert(barrel);
	GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( barrel_test_stage1_check_and_clean, 65000, false, player, old_min, old_max);
}

void barrel_test_stage1_check_and_clean (PlayerBase player, int old_min, int old_max)
{
	//check

	string test_string = "If you did not move, the furthest 2 red barrels should have closed, no others should have changed";
	NotificationSystem.SendNotificationToPlayerIdentityExtended(
		player.GetIdentity(), 
		7.0,
		"Test Stage 1 Check",
		test_string,
		"set:dayz_inventory image:barrel");	
	//cleanup
	g_Game.GetVSTConfig().Set_auto_close_random_seconds_min(old_min);
	g_Game.GetVSTConfig().Set_auto_close_random_seconds_max(old_max);

	GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( barrel_test_stage2, 10000, false, player);
}

void barrel_test_stage2 (PlayerBase player)
{
	foreach (Barrel_ColorBase barrel: g_barrelArray)
	{
		barrel.Delete();
	}
	
	g_barrelArray.Clear();
	string test_string = "Open your inventory and observe inventory save and restore";
	NotificationSystem.SendNotificationToPlayerIdentityExtended(
		player.GetIdentity(), 
		7.0,
		"Test Stage 2",
		test_string,
		"set:dayz_inventory image:barrel");	
	
	vector pos = player.GetPosition();
	pos[2] = pos[2] + 0.5;
	
	Barrel_ColorBase b = Barrel_ColorBase.Cast(GetGame().CreateObject("Barrel_Red", pos));
    b.Open();	
	b.GetInventory().CreateEntityInCargo("SparkPlug");
	b.GetInventory().CreateEntityInCargo("TireRepairKit");
	b.GetInventory().CreateEntityInCargo("GPSReceiver");

	g_barrelArray.Insert(b);	
	GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( barrel_test_stage2_save, 10000, false, player);	
}

void barrel_test_stage2_save (PlayerBase player)
{
	string test_string = "Items are being saved, they should disappear and be replaced with a rag that keeps the barrel from being moved or turned into a fire barrel when closed";
	NotificationSystem.SendNotificationToPlayerIdentityExtended(
		player.GetIdentity(), 
		7.0,
		"Test Stage 2 Save",
		test_string,
		"set:dayz_inventory image:barrel");	
    Barrel_ColorBase b;
	
	b = g_barrelArray.Get(0);
	b.vclose();
	GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( barrel_test_stage2_restore, 10000, false, player);	
}

void barrel_test_stage2_restore (PlayerBase player)
{
	string test_string = "Items are being restored, the rag should disappear";
	NotificationSystem.SendNotificationToPlayerIdentityExtended(
		player.GetIdentity(), 
		7.0,
		"Test Stage 2 Restore",
		test_string,
		"set:dayz_inventory image:barrel");	
    Barrel_ColorBase b;
	
	b = g_barrelArray.Get(0);
	b.vopen(player, "");
	
	test_string = "Remaining tests are manual";
	NotificationSystem.SendNotificationToPlayerIdentityExtended(
		player.GetIdentity(), 
		7.0,
		"Auto tests complete",
		test_string,
		"set:dayz_inventory image:barrel");	
 
	int x;
	for (x = 3; x < 21; x=x+3)
	{
		vector pos = player.GetPosition();
		pos[2] = pos[2] + x;
		GetGame().CreateObject("Barrel_Red", pos);
	}
}

void run_barrel_test(string playerName)
{
	autoptr array<Man> players = new array<Man>;
	GetGame().GetPlayers(players);
	
	PlayerBase pb;
	PlayerBase found_player;
	PlayerIdentity pi;
	foreach (Man m: players)
	{
		pb = PlayerBase.Cast(m);
		if(pb)
		{
			pi = pb.GetIdentity();
			if (pi)
			{
				if (pi.GetName() == playerName)
				{
					found_player = pb;
					break;
				}
			}
		}
	}
	
	GetGame().GetCallQueue( CALL_CATEGORY_SYSTEM ).CallLater( barrel_test_stage1, 1000, false, found_player);
}

void main()
{
	//INIT ECONOMY--------------------------------------
	Hive ce = CreateHive();
	if ( ce )
		ce.InitOffline();

	//DATE RESET AFTER ECONOMY INIT-------------------------
	int year, month, day, hour, minute;
	int reset_month = 9, reset_day = 20;
	GetGame().GetWorld().GetDate(year, month, day, hour, minute);

	if ((month == reset_month) && (day < reset_day))
	{
		GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
	}
	else
	{
		if ((month == reset_month + 1) && (day > reset_day))
		{
			GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
		}
		else
		{
			if ((month < reset_month) || (month > reset_month + 1))
			{
				GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
			}
		}
	}
}

class CustomMission: MissionServer
{
	void SetRandomHealth(EntityAI itemEnt)
	{
		if ( itemEnt )
		{
			float rndHlt = Math.RandomFloat( 0.45, 0.65 );
			itemEnt.SetHealth01( "", "", rndHlt );
		}
	}

	override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName)
	{
		Entity playerEnt;
		playerEnt = GetGame().CreatePlayer( identity, characterName, pos, 0, "NONE" );
		Class.CastTo( m_player, playerEnt );

		GetGame().SelectPlayer( identity, m_player );

		return m_player;
	}

	override void StartingEquipSetup(PlayerBase player, bool clothesChosen)
	{
		EntityAI itemClothing;
		EntityAI itemEnt;
		ItemBase itemBs;
		float rand;

		itemClothing = player.FindAttachmentBySlotName( "Body" );
		if ( itemClothing )
		{
			SetRandomHealth( itemClothing );
			
			itemEnt = itemClothing.GetInventory().CreateInInventory( "BandageDressing" );
			player.SetQuickBarEntityShortcut(itemEnt, 2);
			
			string chemlightArray[] = { "Chemlight_White", "Chemlight_Yellow", "Chemlight_Green", "Chemlight_Red" };
			int rndIndex = Math.RandomInt( 0, 4 );
			itemEnt = itemClothing.GetInventory().CreateInInventory( chemlightArray[rndIndex] );
			SetRandomHealth( itemEnt );
			player.SetQuickBarEntityShortcut(itemEnt, 1);

			rand = Math.RandomFloatInclusive( 0.0, 1.0 );
			if ( rand < 0.35 )
				itemEnt = player.GetInventory().CreateInInventory( "Apple" );
			else if ( rand > 0.65 )
				itemEnt = player.GetInventory().CreateInInventory( "Pear" );
			else
				itemEnt = player.GetInventory().CreateInInventory( "Plum" );
			player.SetQuickBarEntityShortcut(itemEnt, 3);
			SetRandomHealth( itemEnt );
		}
		
		itemClothing = player.FindAttachmentBySlotName( "Legs" );
		if ( itemClothing )
			SetRandomHealth( itemClothing );
		
		itemClothing = player.FindAttachmentBySlotName( "Feet" );
	}
	
	override void OnEvent(EventType eventTypeId, Param params)
	{
		super.OnEvent(eventTypeId, params);
		
		string sender_name;
		string message;
		PlayerIdentity identity;
		
		switch (eventTypeId)
		{
			case ChatMessageEventTypeID:
				ChatMessageEventParams chat_params = ChatMessageEventParams.Cast( params );
				sender_name = chat_params.param2;
				message = chat_params.param3;
				message.ToLower();
				if (message == "/testbarrels")
				{
					run_barrel_test(sender_name);
				}
				break;
				
			case ClientReadyEventTypeID:
				ClientReadyEventParams readyParams;
				Class.CastTo(readyParams, params);
				identity = readyParams.param1;
				if (!identity)
				{
					break;
				}
				NotificationSystem.SendNotificationToPlayerIdentityExtended(
					identity, 
					15.0,
					string.Format("Welcome back, %1", identity.GetName()), 
					"To run a barrel test just chat the command \"/testbarrels\" (without the quotes) make sure there is pleny of space to your north.",
					"set:dayz_inventory image:barrel");
				break;
			case ClientNewEventTypeID:
				ClientNewEventParams newParams;
				Class.CastTo(newParams, params);
				identity = newParams.param1;
				if (!identity)
				{
					break;
				}
				NotificationSystem.SendNotificationToPlayerIdentityExtended(
					identity, 
					15.0,
					string.Format("Welcome fresh spawn, %1", identity.GetName()), 
					"To run a barrel test just chat the command \"/testbarrels\" (without the quotes). You will be teleported to the SouthEast coast and the tests will run. Please read the instructions as they proceed.",
					"set:dayz_inventory image:barrel");
				break;
		}
	}
};

Mission CreateCustomMission(string path)
{
	return new CustomMission();
}

