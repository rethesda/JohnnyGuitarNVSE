#pragma once


//All thanks to jazzisparis for the code, since it's way faster than tanf.

inline float tan_p(float angle)
{
	angle *= kDbl4dPI;
	double ang2 = angle * angle;
	return angle * (211.849369664121 - 12.5288887278448 * ang2) / (269.7350131214121 + ang2 * (ang2 - 71.4145309347748));
}


 
 __forceinline float dTan(float angle) {
	 {
		 while (angle > kDblPIx2)
			 angle -= kDblPIx2;

		 int octant = int(angle * kDbl4dPI);
		 switch (octant)
		 {
		 case 0:
			 return tan_p(angle);
		 case 1:
			 return 1.0 / tan_p(kDblPId2 - angle);
		 case 2:
			 return -1.0 / tan_p(angle - kDblPId2);
		 case 3:
			 return -tan_p(kDblPI - angle);
		 case 4:
			 return tan_p(angle - kDblPI);
		 case 5:
			 return 1.0 / tan_p(kDblPIx3d2 - angle);
		 case 6:
			 return -1.0 / tan_p(angle - kDblPIx3d2);
		 default:
			 return -tan_p(kDblPIx2 - angle);
		 }
	 }
 }

float fastDTan(float value) {
	bool sign = (value < 0);
	if (sign) value = -value;
	float tempRes = dTan(value * kDblPId180);
	return (sign ? -tempRes : tempRes);
}


//As defined in NVSE.


#define PASS_VARARGS  scriptObj, eventList
#define VARARGS Script *scriptObj, ScriptEventList *eventList



void setVarByName(VARARGS, const char *var_name, float value)
{
	ListNode<VariableInfo>* traverse = scriptObj->varList.Head();
	VariableInfo *varInfo;
	do
	{
		varInfo = traverse->data;
		if (!strcmp(((char*)varInfo->name.CStr()), var_name))
		{
			eventList->GetVariable(varInfo->idx)->data = value;
			break;
		}
	} while (traverse = traverse->next);

}

class LevelUpMenu : public Menu {
public:
	LevelUpMenu();
	~LevelUpMenu();
	UInt32 isPerkMenu;
	TileText *tile2C;
	TileImage *tile30;
	TileImage *tile34;
	TileImage *tile38;
	TileText *tile3C;
	TileText *tile40;
	TileImage *tile44;
	TileImage *tile48;
	TileImage *tileBackBtn;
	TileImage *tile50;
	UInt32 unk54;
	UInt32 unk58;
	UInt32 unk5C;
	UInt32 unk60;
	UInt32 listBoxActorValue[12];
	UInt32 listBoxPerk[12];
	UInt32 unkC4[2];
};
struct MediaSetData
{
	String filepath; // NAM2 NAM3 NAM4 NAM5 NAM6 NAM7
	float dB; // NAM8 NAM9 NAM0 ANAM BNAM CNAM
	float boundary; // JNAM KNAM LNAM MNAM NNAM ONAM
};

class MediaSet : public TESForm {
public:
	MediaSet();
	~MediaSet();
	TESFullName	fullName;
	UInt32 unk24[8];
	UInt32 type; // NAM1
	MediaSetData data[6];
	UInt32 flags; //PNAM
	float DNAM;
	float ENAM;
	float FNAM;
	float GNAM;
	TESSound *HNAM;
	TESSound *INAM;
};
STATIC_ASSERT(sizeof(MediaSet) == 0xC4);
struct ItemEntryData
{
	TESForm				*type;
	ContChangesEntry	*entry;
	ExtraDataList		*xData;

	ItemEntryData() {}
	ItemEntryData(TESForm *_type, ContChangesEntry *_entry, ExtraDataList *_xData) : type(_type), entry(_entry), xData(_xData) {}
};
class InventoryRef
{
public:
	ItemEntryData	data;
	TESObjectREFR	*containerRef;
	TESObjectREFR	*tempRef;
	UInt32			deferredActions[6];
	bool			doValidation;
	bool			removed;

	bool CreateExtraData(BSExtraData *xBSData);
};

bool InventoryRef::CreateExtraData(BSExtraData *xBSData)
{
	ExtraContainerChanges::EntryDataList *entryList = containerRef->GetContainerChangesList();
	if (!entryList) return false;
	ContChangesEntry *entry = entryList->FindForItem(data.type);
	if (!entry) return false;
	data.xData = ExtraDataList::Create(xBSData);
	if (!entry->extendData)
	{
		entry->extendData = (ExtraContainerChanges::ExtendDataList*)GameHeapAlloc(8);
		entry->extendData->Init();
	}
	entry->extendData->Insert(data.xData);
	return true;
}

InventoryRef* (*InventoryRefCreate)(TESObjectREFR *container, const ItemEntryData &data, bool bValidate);

//Only ready for a 24-bit BMP, will check for non-24 bit later.
//Also currently doesn't handle negative height/width BMPs, will fix later
bool ReadBMP24(char* filename, unsigned long& R, unsigned long& G, unsigned long& B, unsigned long PixelW, unsigned long PixelH)
{
	FILE* f = fopen(filename, "rb");

	if (f == NULL)
		return false;
	char info[54];
	fread(info, sizeof(char), 54, f);
	int width = *(int*)& info[18];
	int height = *(int*)& info[22];
	if (width < PixelW || height < PixelH) return false;
	int XPadding = (width * 3 + 3) & (~3);
	char* data = new char[XPadding];
	PixelH = height - PixelH;
	fseek(f, XPadding * PixelH, SEEK_CUR);
	fread(data, sizeof(char), XPadding, f);
	UInt32 PosX = PixelW * 3;
	B = data[PosX];
	G = data[PosX + 1];
	R = data[PosX + 2];
	fclose(f);
	delete data;
	return true;
}

bool AlchemyItem::IsPoison()
{
	EffectItem *effItem;
	EffectSetting *effSetting = NULL;
	ListNode<EffectItem> *iter = magicItem.list.list.Head();
	do
	{
		if (!(effItem = iter->data)) continue;
		effSetting = effItem->setting;
		if (effSetting && !(effSetting->effectFlags & 4)) return false;
	} while (iter = iter->next);
	return effSetting != NULL;
}

const uintptr_t TESForm_Vtables[] =
{
	0x010939EC,
	0x0103195C,
	0x0103168C,
	0x0103140C,
	0x0104D5B4,
	0x0104D19C,
	0x01044FFC,
	0x0104CC0C,
	0x0104BA24,
	0x0102397C,
	0x010369DC,
	0x01036B2C,
	0x0104B4BC,
	0x0104AC44,
	0x0106847C,
	0x0102C51C,
	0x0102BC94,
	0x0102BA2C,
	0x0102F55C,
	0x0102B844,
	0x01028EE4,
	0x0102DCD4,
	0x0102B5AC,
	0x0102B1FC,
	0x0102AEB4,
	0x0102AC0C,
	0x0102E9B4,
	0x0102A9C4,
	0x0102A62C,
	0x0102A31C,
	0x0102A0A4,
	0x01029D5C,
	0x01029B1C,
	0x0104A2F4,
	0x01036854,
	0x010366CC,
	0x01028C5C,
	0x01028A64,
	0x0102886C,
	0x0102864C,
	0x0102E6C4,
	0x01028444,
	0x0102D97C,
	0x0102D7F4,
	0x01049D0C,
	0x01049B9C,
	0x0102814C,
	0x01036524,
	0x01026D0C,
	0x01026A4C,
	0x01026BD0,
	0x010498DC,
	0x0104973C,
	0x0102685C,
	0x01048F5C,
	0x010266E4,
	0x0102D5C4,
	0x01048BB4,
	0x0104891C,
	0x010263DC,
	0x01026574,
	0x010349B4,
	0x01034B4C,
	0x0103478C,
	0x01029994,
	0x0102960C,
	0x0102980C,
	0x0103449C,
	0x01026064,
	0x010492AC,
	0x01013F8C,
	0x0109381C,
	0x01093674,
	0x01037094,
	0x010900DC,
	0x0108AA3C,
	0x0106BB8C,
	0x0106A0B4,
	0x0108A49C,
	0x0108FA44,
	0x010342EC,
	0x010340C4,
	0x0107B8C4,
	0x0107B394,
	0x01011B4C,
	0x01013044,
	0x0107AD84,
	0x0107A8F4,
	0x0107A554,
	0x01013284,
	0x0108F674,
	0x010931E4,
	0x0108F2F4,
	0x0108EE04,
	0x01012EA4,
	0x01012834,
	0x01092F6C,
	0x010870AC,
	0x0108EA64,
	0x0108CE74,
	0x01086A6C,
	0x0108C3C4,
	0x01092D84,
	0x0104733C,
	0x01033D1C,
	0x01025914,
	0x01025594,
	0x0102535C,
	0x010116FC,
	0x010470EC,
	0x01033B34,
	0x010251AC,
	0x01024F4C,
	0x01046EC4,
	0x01046874,
	0x0103397C,
	0x01024CEC,
	0x01024E70,
	0x01024E84,
	0x010337C4,
	0x01033654,
	0x010334B4,
	0x0102CD94,
	0x0103323C,
	0x01032F6C,
	0x0104664C,
	0x010115B4,
	0x010464B4,
	0x01024A94,
	0x0102CBBC,
	0x0101144C,
	0x01032DC4,
	0x01024834,
	0x010245A4,
	0x010327F4,
	0x0103245C,
	0x01045504,
	0x01024214,
	0x010320FC,
	0x01085954,
	0x01011964,
	0x01092BDC,
	0x01067A2C,
	0x01084254,
};