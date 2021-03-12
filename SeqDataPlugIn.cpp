#include "pch.h"
#include "SeqDataPlugin.h"


#ifndef COPYRIGHTS
#define PLUGIN_NAME "DLSPlugin"
#define PLUGIN_VERSION "1.1.0"
#define PLUGIN_AUTHOR "Kingfu Chan"
#define PLUGIN_COPYRIGHT "MIT License, Copyright (c) 2020 Kingfu Chan"
#define GITHUB_LINK "https://github.com/KingfuChan/Departure-List-Sequencing-PlugIn-for-EuroScope"
#endif // !COPYRIGHTS


// default settings
const int DEFAULT_REFRESH_INTERVAL = 5;
const int DEFAULT_MAX_SPEED = 80; // maximum speed before delete from memory

// status constants DO NOT CHANGE
const int STATUS_STBY_CLEARANCE = 1;
const int STATUS_CLRD_CLEARANCE = 2;
const int STATUS_STBY_PUSHTAXI = 3;
const int STATUS_CLRD_PUSHTAXI = 4;
const int STATUS_STBY_TAKEOFF = 5;
const int STATUS_CLRD_TAKEOFF = 6;

using namespace EuroScopePlugIn;


SeqDataPlugIn::SeqDataPlugIn(void)
	: CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
		PLUGIN_NAME,
		PLUGIN_VERSION,
		PLUGIN_AUTHOR,
		PLUGIN_COPYRIGHT)
{
	activeConfig = PluginCfg{ 0,0,"" };
}


SeqDataPlugIn::~SeqDataPlugIn(void) {

}


void SeqDataPlugIn::ConfigurePlugin(const int* interval, const int* speed, const char* sync) {
	// only changes config if param isn't nullptr
	if (interval != nullptr)
		activeConfig.interval = *interval > 0 ? *interval : DEFAULT_REFRESH_INTERVAL;
	if (speed != nullptr)
		activeConfig.speed = *speed >= 30 ? *speed : DEFAULT_MAX_SPEED;
	if (sync != nullptr)
		activeConfig.sync = sync;
}


SequenceData SeqDataPlugIn::GetSequence(CFlightPlan FlightPlan) {
	// if unable to find, return a dummy SequenceData of -1
	int ary, idx;
	int seq[3] = { 0,0,0 };
	SequenceData sd;
	sd.m_syncstatus = GetSyncStatus(FlightPlan);

	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++) {
		ary = (m_SequenceArray[idx].m_status - 1) / 2; // array index
		if (m_SequenceArray[idx].m_active && m_SequenceArray[idx].m_status % 2) // stby status
			m_SequenceArray[idx].m_sequence = ++seq[ary];
		else
			m_SequenceArray[idx].m_sequence = 0;
		if (!strcmp(m_SequenceArray[idx].m_callsign, FlightPlan.GetCallsign())) {
			sd = m_SequenceArray[idx];
			return sd;
		}
	}

	sd.m_callsign = FlightPlan.GetCallsign();
	sd.m_status = -1;
	sd.m_active = -1;
	sd.m_sequence = -1;
	return sd;
}


bool SeqDataPlugIn::IsCallsignOnline(const char* callsign) {
	CFlightPlan fp = FlightPlanSelect(callsign);
	return fp.IsValid();
}


void SeqDataPlugIn::AddSequence(CFlightPlan FlightPlan) {
	SequenceData seq = {
		FlightPlan.GetCallsign(),
		STATUS_STBY_CLEARANCE,
		STATUS_STBY_CLEARANCE,
		true };
	m_SequenceArray.Add(seq);
}


SequenceData SeqDataPlugIn::PopSequence(CFlightPlan FlightPlan) {
	// remove a SequenceData and return
	// TODO: sync related
	int idx;
	SequenceData sd;
	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++)
		if (!strcmp(m_SequenceArray[idx].m_callsign, FlightPlan.GetCallsign())) {
			sd = m_SequenceArray[idx];
			m_SequenceArray.RemoveAt(idx);
			break;
		}

	PutSyncStatus(FlightPlan, 0); // delete sync
	return sd;
}


void SeqDataPlugIn::EditStatus(CFlightPlan FlightPlan, const int method) {
	// method: 0-reset, 1=next standby, 2=next cleared, -1=previous
	int idx;
	for (idx = 0;
		idx < m_SequenceArray.GetCount() && strcmp(FlightPlan.GetCallsign(), m_SequenceArray[idx].m_callsign);
		idx++);
	if (idx == m_SequenceArray.GetCount()) return;

	SequenceData sd;
	switch (method)
	{
	case 0: // reset
		m_SequenceArray.RemoveAt(idx);
		AddSequence(FlightPlan);
		PutSyncStatus(FlightPlan, STATUS_STBY_CLEARANCE);
		break;

	case 1: // next standby
		sd = m_SequenceArray[idx];
		sd.m_status += sd.m_status % 2 ? 2 : 1;
		m_SequenceArray.RemoveAt(idx);
		m_SequenceArray.Add(sd);
		break;

	case 2: // next cleared
		sd = m_SequenceArray[idx];
		sd.m_status += sd.m_status % 2 ? 1 : 2;
		m_SequenceArray.RemoveAt(idx);
		m_SequenceArray.Add(sd);
		break;

	case -1: // previous
		sd = m_SequenceArray[idx];
		sd.m_status--;
		m_SequenceArray.RemoveAt(idx);
		m_SequenceArray.Add(sd);
		break;

	default:
		break;
	}
}


void SeqDataPlugIn::EditSequence(const SequenceData seqdata, const int target_seq) {
	if (!seqdata.m_status % 2 || seqdata.m_sequence == target_seq) return;
	// cleared status has no sequence || no need to adjust

	CFlightPlan fp = FlightPlanSelect(seqdata.m_callsign);
	SequenceData sd = PopSequence(fp); // remove original, delete sync
	PutSyncStatus(fp, seqdata.m_status); // set sync
	int idx, seq;
	for (idx = seq = 0; idx < m_SequenceArray.GetCount(); idx++)
		if (seqdata.m_status == m_SequenceArray[idx].m_status)
			if (++seq == target_seq)
				m_SequenceArray.InsertAt(idx, sd);
}


void SeqDataPlugIn::RemoveAll(void) {
	for (int idx = 0; idx < m_SequenceArray.GetCount(); idx++) // delete sync status
		PutSyncStatus(FlightPlanSelect(m_SequenceArray[idx].m_callsign), 0);
	m_SequenceArray.RemoveAll();
}


void SeqDataPlugIn::RemoveOffline(void) {
	int idx;
	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++)
		if (!IsCallsignOnline(m_SequenceArray[idx].m_callsign))
			m_SequenceArray.RemoveAt(idx);
}


int SeqDataPlugIn::GetSyncStatus(CFlightPlan FlightPlan) {
	// status as above, -1 if not find or not set
	CRadarTarget rt = FlightPlan.GetCorrelatedRadarTarget();
	if (rt.GetGS() > activeConfig.speed) return -1;
	CString mode = activeConfig.sync;
	if (mode == "SPEED")
		return FlightPlan.GetControllerAssignedData().GetAssignedSpeed() - 2000;
	if (mode == "MACH")
		return FlightPlan.GetControllerAssignedData().GetAssignedMach() - 300;
	if (mode == "RATE")
		return FlightPlan.GetControllerAssignedData().GetAssignedRate() - 9000;
	return -1;
}


void SeqDataPlugIn::PutSyncStatus(CFlightPlan FlightPlan, int status) {
	// status as above, 0 to delete
	CString mode = activeConfig.sync;
	// 0 judgement for invalidate
	if (mode == "SPEED")
		FlightPlan.GetControllerAssignedData().SetAssignedSpeed(status ? status + 2000 : 0);
	if (mode == "MACH")
		FlightPlan.GetControllerAssignedData().SetAssignedMach(status ? status + 300 : 0);
	if (mode == "RATE")
		FlightPlan.GetControllerAssignedData().SetAssignedRate(status ? status + 9000 : 0);
}


// below are inherited functions


void SeqDataPlugIn::OnTimer(int Counter) {
	// update active information for all aircrafts
	if (Counter % activeConfig.interval) // once every n seconds
		return;

	int idx;
	bool online;
	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++) {
		online = IsCallsignOnline(m_SequenceArray[idx].m_callsign);
		m_SequenceArray[idx].m_active = online;
		if (!online) { // avoid potential errors
			TRACE("%s\tinactive\n", m_SequenceArray[idx].m_callsign);
			continue;
		}
		CFlightPlan fp;
		fp = FlightPlanSelect(m_SequenceArray[idx].m_callsign);
		if (fp.GetCorrelatedRadarTarget().GetGS() > activeConfig.speed) { // criterion for taking off
			TRACE("%s\tremoved\n", m_SequenceArray[idx].m_callsign);
			m_SequenceArray.RemoveAt(idx);
			PutSyncStatus(fp, 0); // invalidate status
		}
		else if (GetSyncStatus(fp) <= m_SequenceArray[idx].m_status)
			PutSyncStatus(fp, m_SequenceArray[idx].m_status);
	}
}
