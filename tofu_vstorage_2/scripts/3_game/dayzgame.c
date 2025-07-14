modded class DayZGame
{	
	protected ref VST_Config m_VST_Config;
	protected ref VST_Config_v2 m_VST_Config_v2;
	
    ref VST_Config GetVSTConfig()
    {
        if (!m_VST_Config)
        {
			SetVSTConfig(new VST_Config);
        }
        
	    return m_VST_Config;
    }
	
	ref VST_Config_v2 GetVSTConfig_v2()
    {
        if (!m_VST_Config_v2)
        {
			SetVSTConfig_v2(new VST_Config_v2);
        }
        
	    return m_VST_Config_v2;
    }

    void SetVSTConfig(VST_Config config)
	{
		//Print("[vStorage] Setting config: "+config);
		m_VST_Config = config;
	}
	
	void SetVSTConfig_v2(VST_Config_v2 config)
	{
		//Print("[vStorage] Setting config: "+config);
		m_VST_Config_v2 = config;
	}
    
};
