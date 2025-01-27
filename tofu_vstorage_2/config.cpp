class CfgPatches
{
	class ToFuVStorage
	{
		requiredVersion = 0.1;
		requiredAddons[] = {
			"DZ_Data",
			"DZ_Scripts",
		};
	};
};
class CfgMods
{
	class ToFuVStorage
	{
		dir = "tofu_vstorage_2";
		picture = "";
		action = "";
		hideName = 0;
		hidePicture = 0;
		name = "ToFuVirtualStorage2";
		credits = "Funatic / inkihh";
		author = "";
		authorID = "0";
		version = "2.0";
		extra = 0;
		type = "mod";
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"tofu_vstorage_2/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"tofu_vstorage_2/Scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"tofu_vstorage_2/Scripts/5_Mission"};
			};
		};
	};
};

