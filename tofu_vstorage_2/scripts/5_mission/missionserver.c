modded class MissionServer
{
	ref VST_Config m_VST_Config;
	
	override void OnInit()
    {
        super.OnInit();
               
        m_VST_Config = new VST_Config;
        g_Game.SetVSTConfig(m_VST_Config);
    }
	

}