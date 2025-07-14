const string barrel_nolock_flagfile = "$profile:BARRELS_NO_LOCKS.TXT";
const string barrel_nolog_cftools_flagfile = "$profile:BARRELS_NO_LOG_CFTOOLS.txt";


void NEO_check_barrel_flag_files(VST_Config_v2 config)
{
	if (FileExist(barrel_nolock_flagfile))
	{
		Print("[TofuBarrelsNEO] disabling barrel locks");
		config.disable_barrel_locks = true;
	}
	else
	{
		Print("[TofuBarrelsNEO] enabling barrel locks");
		config.disable_barrel_locks = false;
	}
	
	if (FileExist(barrel_nolog_cftools_flagfile))
	{
		Print("[TofuBarrelsNEO] disabling logging to CFTools");
		config.log_to_cftools = false;
	}
	else
	{
		Print("[TofuBarrelsNEO] enabling logging to CFTools");
		config.log_to_cftools = true;
	}
}

modded class MissionServer
{
	ref VST_Config m_VST_Config;
	ref VST_Config_v2 m_VST_Config_v2;
	
	override void OnInit()
    {
        super.OnInit();
               
        m_VST_Config = new VST_Config;
        g_Game.SetVSTConfig(m_VST_Config);
		
		m_VST_Config_v2 = new VST_Config_v2;
		
		NEO_check_barrel_flag_files(m_VST_Config_v2);

		g_Game.SetVSTConfig_v2(m_VST_Config_v2);
    }
	
	override void OnEvent(EventType eventTypeId, Param params)
	{
		switch(eventTypeId)
		{
			// Handle user command
			case ChatMessageEventTypeID:

				ChatMessageEventParams chatParams;

			    chatParams = ChatMessageEventParams.Cast(params);
			
				if (!chatParams)
				{
					Print("[TofuBarrelsNEO] chat msg params were null");
					break;
				}
			
				// Check that input was a command (contains forward slash)
				string cmd = chatParams.param3;
			
				if ( cmd.Get(0) != "/" ) break;
				
				if (cmd == "/checkbarrelflagfiles")
				{
					NEO_check_barrel_flag_files(m_VST_Config_v2);
				}
				break;
		}
		super.OnEvent(eventTypeId, params);
	}
	

}