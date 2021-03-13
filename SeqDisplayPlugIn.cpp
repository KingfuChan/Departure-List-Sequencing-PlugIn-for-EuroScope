#include "pch.h"
#include "SeqDisplayPlugIn.h"


#ifndef COPYRIGHTS
#define GITHUB_LINK "https://github.com/KingfuChan/Departure-List-Sequencing-PlugIn-for-EuroScope"
#endif // !COPYRIGHTS


// TAG ITEM TYPE
const int TAG_ITEM_TYPE_SEQ_STATUS = 1;

// TAG FUNCTIONS
const int TAG_FUNC_SEQ_POPUP = 1; // pop up item
const int TAG_FUNC_SEQ_EDIT_POPUP = 2; // edit
const int TAG_FUNC_SEQ_SEPERATOR = 10;
const int TAG_FUNC_SEQ_START = 11;
const int TAG_FUNC_SEQ_RESET = 12; // reset status
const int TAG_FUNC_SEQ_NEXT_STBY = 13; // next standby status
const int TAG_FUNC_SEQ_NEXT_CLRD = 14; // next cleared status
const int TAG_FUNC_SEQ_PREV = 15; // previous status
const int TAG_FUNC_SEQ_EDIT_FINISH = 16; // finish edit popup
const int TAG_FUNC_SEQ_DELETE = 17; // delete from database

// setting names
const char* SETTINGS_COLOR_STANDBY = "ColorStby";
const char* SETTINGS_COLOR_CLEARED = "ColorClrd";
const char* SETTINGS_INTERVAL = "RefreshIntv";
const char* SETTINGS_MAX_SPEED = "MaxSpeed";
const char* SETTINGS_SYNC = "SyncMode";

// tag texts
const char PLACE_HOLDER[] = "_______"; // place-holder, the text itself is useless
const char STATUS_DESCRIPTION[3][5] = { "CLRN","PUST","TKOF" };

// status constants DO NOT CHANGE
const int STATUS_STBY_CLEARANCE = 1;
const int STATUS_CLRD_CLEARANCE = 2;
const int STATUS_STBY_PUSHTAXI = 3;
const int STATUS_CLRD_PUSHTAXI = 4;
const int STATUS_STBY_TAKEOFF = 5;
const int STATUS_CLRD_TAKEOFF = 6;

using namespace EuroScopePlugIn;


SeqDisplayPlugIn::SeqDisplayPlugIn(void) {
	// custsom initialization
	AddAlias(".cjf", ".KingfuChan"); //just for fun

	//tag related
	RegisterTagItemType("Ground Sequence", TAG_ITEM_TYPE_SEQ_STATUS);
	RegisterTagItemFunction("GND SEQ Popup List", TAG_FUNC_SEQ_POPUP);

	// load settings, note that parse color will return a bool
	CustomColorStby = ParseColorFromText(GetDataFromSettings(SETTINGS_COLOR_STANDBY), CurrentColorStby);
	CustomColorClrd = ParseColorFromText(GetDataFromSettings(SETTINGS_COLOR_CLEARED), CurrentColorClrd);
	// load settings for data class
	const char* itvchar = GetDataFromSettings(SETTINGS_INTERVAL);
	const char* spdchar = GetDataFromSettings(SETTINGS_MAX_SPEED);
	const char* synchar = GetDataFromSettings(SETTINGS_SYNC);
	int itv = itvchar == nullptr ? 0 : atoi(itvchar);
	int spd = spdchar == nullptr ? 0 : atoi(spdchar);
	ConfigurePlugin(&itv, &spd, synchar);

	DisplayMessage("Departure List Sequencing Plugin Loaded!");
	DisplayMessage(CString("For help please refer to ") + GITHUB_LINK);
}


SeqDisplayPlugIn::~SeqDisplayPlugIn(void) {

}


bool SeqDisplayPlugIn::ParseColorFromText(const char* text, COLORREF& color) {
	// set color according to text, if unsuccessful will set color to black (in case mis-used)
	if (text == nullptr) {
		color = RGB(0, 0, 0);
		return false;
	}

	int r, g, b;
	if (sscanf_s(text, "%d,%d,%d", &r, &g, &b) == 3)
		if (r >= 0 && r <= 255 &&
			g >= 0 && r <= 255 &&
			b >= 0 && b <= 255) { // valid color value
			color = RGB(r, g, b);
			return true;
		}

	color = RGB(0, 0, 0);
	return false;
}


void SeqDisplayPlugIn::DisplayMessage(const char* msg) {
	// unified display method with fixed parameters
	DisplayUserMessage("Message", "DLSPlugin",
		msg,
		false, true, true, false, false);
}


// below are inherited methods


void SeqDisplayPlugIn::OnGetTagItem(CFlightPlan FlightPlan, CRadarTarget RadarTarget,
	int ItemCode, int TagData, char sItemString[16],
	int* pColorCode, COLORREF* pRGB, double* pFontSize)
{
	if (!(FlightPlan.IsValid() || RadarTarget.IsValid()))
		return;

	if (ItemCode != TAG_ITEM_TYPE_SEQ_STATUS)  return;
	SequenceData sd;

	sd = GetSequence(FlightPlan);
	int sts = sd.m_status;
	int synsts = sd.m_syncstatus;
	int seq = sd.m_sequence;
	int atv = sd.m_active;
	if (!atv) return; // not active
	else if (sts < 0) { // not found
		if (synsts < STATUS_STBY_CLEARANCE || synsts > STATUS_CLRD_TAKEOFF)
			return;
		// there is a sync status
		if (synsts % 2) { // stby
			sprintf_s(sItemString, strlen(PLACE_HOLDER) + 1, "%s_??",
				STATUS_DESCRIPTION[(synsts - 1) / 2]);
			if (CustomColorStby) {
				*pColorCode = TAG_COLOR_RGB_DEFINED;
				*pRGB = CurrentColorStby;
			}
		}
		else { // clrd
			sprintf_s(sItemString, strlen(PLACE_HOLDER) + 1, "%s__x",
				STATUS_DESCRIPTION[(synsts - 1) / 2]);
			if (CustomColorClrd) {
				*pColorCode = TAG_COLOR_RGB_DEFINED;
				*pRGB = CurrentColorClrd;
			}
		}
	}
	else {
		if (seq) { // stby
			sprintf_s(sItemString, strlen(PLACE_HOLDER) + 1, "%s_%.2d",
				STATUS_DESCRIPTION[(sts - 1) / 2], seq);
			if (CustomColorStby) {
				*pColorCode = TAG_COLOR_RGB_DEFINED;
				*pRGB = CurrentColorStby;
			}
		}
		else { // clrd
			sprintf_s(sItemString, strlen(PLACE_HOLDER) + 1, "%s___",
				STATUS_DESCRIPTION[(sts - 1) / 2]);
			if (CustomColorClrd) {
				*pColorCode = TAG_COLOR_RGB_DEFINED;
				*pRGB = CurrentColorClrd;
			}
		}
	}
}


void SeqDisplayPlugIn::OnFunctionCall(int FunctionId, const char* sItemString, POINT Pt, RECT Area) {
	EuroScopePlugIn::CFlightPlan fp;
	SequenceData sd;

	fp = FlightPlanSelectASEL();
	if (!fp.IsValid())
		return;

	sd = GetSequence(fp);
	if (sd.m_status < 0
		&& FunctionId != TAG_FUNC_SEQ_POPUP && FunctionId != TAG_FUNC_SEQ_START)
		return;

	switch (FunctionId)
	{

	case TAG_FUNC_SEQ_POPUP:

		OpenPopupList(Area, "SEQ Menu", 2);

		if (sd.m_status < 0)
			AddPopupListElement("Start", "SEQ", TAG_FUNC_SEQ_START);
		else {

			// little bit tricky :D
			int sts = sd.m_status;
			if (sts < STATUS_CLRD_TAKEOFF)
				AddPopupListElement(STATUS_DESCRIPTION[sts / 2], "clrd", TAG_FUNC_SEQ_NEXT_CLRD);

			if (sts == STATUS_CLRD_CLEARANCE || sts == STATUS_CLRD_PUSHTAXI)
				AddPopupListElement(STATUS_DESCRIPTION[sts / 2], "stby", TAG_FUNC_SEQ_NEXT_STBY);

			AddPopupListElement("----", "", TAG_FUNC_SEQ_SEPERATOR);

			if (sts > STATUS_STBY_CLEARANCE)
				AddPopupListElement(STATUS_DESCRIPTION[sts / 2 - 1], sts % 2 ? "clrd" : "stby", TAG_FUNC_SEQ_PREV);

			AddPopupListElement("----", "", TAG_FUNC_SEQ_SEPERATOR);

			if (sts % 2) // stby status
				AddPopupListElement("Edit", "SEQ", TAG_FUNC_SEQ_EDIT_POPUP);

			AddPopupListElement("Reset", "STS", TAG_FUNC_SEQ_RESET); // TODO: SYNC related
			AddPopupListElement("Delete", "STS", TAG_FUNC_SEQ_DELETE);
		}

		break;
	case TAG_FUNC_SEQ_START: {
		int synsts = sd.m_syncstatus;
		AddSequence(fp);
		if (synsts >= STATUS_STBY_CLEARANCE && synsts <= STATUS_CLRD_TAKEOFF)
			for (int t = 1; t < synsts; t++)
				EditStatus(fp, t % 2 + 1);
		break;
	}
	case TAG_FUNC_SEQ_RESET:
		EditStatus(fp, 0);
		break;

	case TAG_FUNC_SEQ_NEXT_STBY:
		EditStatus(fp, 1);
		break;

	case TAG_FUNC_SEQ_NEXT_CLRD:
		EditStatus(fp, 2);
		break;

	case TAG_FUNC_SEQ_PREV:
		EditStatus(fp, -1);
		break;

	case TAG_FUNC_SEQ_EDIT_POPUP:
		OpenPopupEdit(Area, TAG_FUNC_SEQ_EDIT_FINISH, "");
		break;

	case TAG_FUNC_SEQ_EDIT_FINISH:
		int ns; // new sequence
		if (sscanf_s(sItemString, "%d", &ns) > 0)
			EditSequence(sd, ns);
		break;

	case TAG_FUNC_SEQ_DELETE:
		PopSequence(fp);
		break;

	default:
		break;
	}
}


bool SeqDisplayPlugIn::OnCompileCommand(const char* sCommandLine) {
	CString cmd = sCommandLine;
	cmd.Trim();
	cmd.MakeUpper();

	if (cmd == ".DLS" || cmd == ".CJF" || cmd == ".KINGFUCHAN") { // for fun
		DisplayMessage(GITHUB_LINK);
		return true;
	}

	if (cmd.Left(4) == ".DLS " || cmd.GetLength() < 5) // undefined
		return false;

	cmd = cmd.Mid(5);
	CString msg; // for display

	if (cmd == "REMOVE ALL") {
		RemoveAll();
		DisplayMessage("All sequences have been removed!");
		return true;
	}

	if (cmd == "REMOVE OFFLINE") {
		RemoveOffline();
		DisplayMessage("All offline sequences have been removed!");
		return true;
	}

	// color format: rrr,ggg,bbb (no space, English comma), note that cmd is trimmed
	if (cmd.Left(14) == "COLOR STANDBY ") { // 14==strlen
		const char* colordata = (LPCTSTR)cmd + 14;
		CustomColorStby = ParseColorFromText(colordata, CurrentColorStby);
		if (CustomColorStby) {
			SaveDataToSettings(SETTINGS_COLOR_STANDBY, "custom color for standby tags", colordata);
			msg.Format("Custom color for standby tags have been set to (%s)!", colordata);
			DisplayMessage(msg);
		}
		else {
			SaveDataToSettings(SETTINGS_COLOR_STANDBY, "reset color for standby tags", nullptr);
			DisplayMessage("Default colors for standby tags have been set!");
		}
		return true;
	}

	if (cmd.Left(14) == "COLOR CLEARED ") { // 14==strlen
		const char* colordata = (LPCTSTR)cmd + 14;
		CustomColorClrd = ParseColorFromText(colordata, CurrentColorClrd);
		if (CustomColorClrd) {
			SaveDataToSettings(SETTINGS_COLOR_CLEARED, "custom color for cleared tags", colordata);
			msg.Format("Custom color for cleared tags have been set to (%s)!", colordata);
			DisplayMessage(msg);
		}
		else {
			SaveDataToSettings(SETTINGS_COLOR_CLEARED, "reset color for cleared tags", nullptr);
			DisplayMessage("Default colors for cleared tags have been set!");
		}
		return true;
	}

	if (cmd == "COLOR RESET") {
		CustomColorStby = ParseColorFromText(nullptr, CurrentColorStby);
		CustomColorClrd = ParseColorFromText(nullptr, CurrentColorClrd);
		SaveDataToSettings(SETTINGS_COLOR_STANDBY, "reset color for standby tags", nullptr);
		SaveDataToSettings(SETTINGS_COLOR_CLEARED, "reset color for cleared tags", nullptr);
		DisplayMessage("Default colors for tags have been set!");
		return true;
	}

	if (cmd.Left(9) == "INTERVAL ") { // 9==strlen
		CString itvstr = cmd.Mid(9);
		int itv = atoi(itvstr);
		if (itv >= 1) {
			ConfigurePlugin(&itv, nullptr, nullptr);
			SaveDataToSettings(SETTINGS_INTERVAL, "custom refresh interval", itvstr);
			msg.Format("Refresh interval has been set to %d seconds!", itv);
			DisplayMessage(msg);
			return true;
		}
	}

	if (cmd == "INTERVAL RESET") {
		ConfigurePlugin(0, nullptr, nullptr);
		SaveDataToSettings(SETTINGS_INTERVAL, "reset refresh interval", nullptr);
		DisplayMessage("Default refresh interval has been set!");
		return true;
	}

	if (cmd.Left(6) == "SPEED ") { // 6==strlen
		CString spdstr = cmd.Mid(6);
		int spd = atoi(spdstr);
		if (spd >= 30) {
			ConfigurePlugin(nullptr, &spd, nullptr);
			SaveDataToSettings(SETTINGS_MAX_SPEED, "custom maximum speed", spdstr);
			msg.Format("Maximum speed has been set to %d knots!", spd);
			DisplayMessage(msg);
			return true;
		}
	}

	if (cmd == "SPEED RESET") {
		ConfigurePlugin(nullptr, 0, nullptr);
		SaveDataToSettings(SETTINGS_MAX_SPEED, "reset maximum speed", nullptr);
		DisplayMessage("Default maximum speed has been set!");
		return true;
	}

	if (cmd.Left(5) == "SYNC ") { // 5==strlen
		CString method = cmd.Mid(5);
		if (method == "SPEED" || method == "MACH" || method == "RATE" || method == "OFF") {
			ConfigurePlugin(nullptr, nullptr, method);
			SaveDataToSettings(SETTINGS_SYNC, "synchronizing method", method);
			DisplayMessage("Synchronize method has been set!");
			return true;
		}
	}

	return false;
}
