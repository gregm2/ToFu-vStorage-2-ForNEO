class tofuvStorageObj 
{
	string itemName = "";
	string itemType = "";
	int itemRow = 0;
	int itemCol = 0;
	bool itemFliped = false;
	int itemQuantity = 0;
	int itemAmmoQuantity = 0;
	float itemHealth = 0;
	int itemLiquidType = 0;
	int itemFoodstage = 0;
	float itemTemp = 0;
	float itemWetness = 0; // also used for EasterEgg m_ParScale and eggs will have no wetness stored/loaded
	int itemInventoryType = 0;
	int itemIdx = 0;
	int itemSlotId = 0;
	int itemTimeStored = 0;
	string itemID = "";
	string itemWpnBarrelInfo = "";
	string itemWpnInternalMagInfo = "";
	string itemMagInfo = "";
	
	autoptr array<string> itemMagInhalt = {};
	
	string itemEECreatureType = "";
	string itemPprNoteData = "";
	string itemUnusedString3 = "";
	string itemUnusedString4 = "";
	string itemUnusedString5 = "";
	string itemUnusedString6 = "";
	
	int itemEnergy = 0;
	int itemEECaptureState = 0; // easter egg capture state
	int itemEERelSoundHash = 0; // easter egg release sound hash
	int itemEECapSoundHash = 0; // easter egg capture sound hash
	int itemUnusedInt5 = 0;
	int itemUnusedInt6 = 0;
	
	bool itemHasEnergy = false;
	bool itemEEDangerSound = false; // easter egg danger sound flag
	bool itemEEIsEgg = false; // flag for faster easter egg detection on load
	bool itemPprIsPaper = false;
	bool itemUnusedBool5 = false;
	bool itemUnusedBool6 = false;
	
	ref array<ref tofuvStorageObj> itemChildren = {};
};


class tofuvStorageContainer 
{
	string persistentId = "";
	ref array<ref tofuvStorageObj> storedItems = {};
};

class tofuvStorageContainerMeta
{
	bool m_vst_hasitems;
	bool m_vst_wasplaced;
	ref array<string> m_vst_owner_names = {};
	ref array<string> m_vst_owner_steamids = {};
};
