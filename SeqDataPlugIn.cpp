#include "pch.h"
#include "SeqDataPlugin.h"


#ifndef COPYRIGHTS
#define PLUGIN_NAME "Departure List Sequencing"
#define PLUGIN_VERSION "1.0.0"
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
	ConfigurePlugin(0, 0);
}


SeqDataPlugIn::~SeqDataPlugIn(void) {

}


void SeqDataPlugIn::ConfigurePlugin(int* interval, int* speed) {
	//TODO
	if (interval != nullptr)
		CustomRefreshIntv = *interval > 0 ? *interval : DEFAULT_REFRESH_INTERVAL;
	if (speed != nullptr)
		CustomMaxSpeed = *speed >= 30 ? *speed : DEFAULT_MAX_SPEED;
}


SequenceData SeqDataPlugIn::GetSequence(const char* callsign) {
	// if unable to find, return a dummy SequenceData of -1
	int ary, idx;
	int seq[3] = { 0,0,0 };
	SequenceData sd;

	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++) {
		ary = (m_SequenceArray[idx].m_status - 1) / 2; // array index

		if (!m_SequenceArray[idx].m_active) // double check online, avoid potential bugs on refresh
			m_SequenceArray[idx].m_active = IsCallsignOnline(callsign);

		if (m_SequenceArray[idx].m_active && m_SequenceArray[idx].m_status % 2) // stby status
			m_SequenceArray[idx].m_sequence = ++seq[ary];
		else
			m_SequenceArray[idx].m_sequence = 0;

		if (!strcmp(m_SequenceArray[idx].m_callsign, callsign)) {
			sd = m_SequenceArray[idx];
			return sd;
		}
	}

	sd.m_callsign = callsign;
	sd.m_status = -1;
	sd.m_active = -1;
	sd.m_sequence = -1;
	return sd;
}


bool SeqDataPlugIn::IsCallsignOnline(const char* callsign) {
	CRadarTarget rt;
	for (rt = RadarTargetSelectFirst(); rt.IsValid(); rt = RadarTargetSelectNext(rt))
		if (!strcmp(callsign, rt.GetCallsign()))
			return true;
	return false;
}


void SeqDataPlugIn::AddSequence(const char* callsign) {
	SequenceData seq = {
		callsign,
		STATUS_STBY_CLEARANCE,
		true };
	m_SequenceArray.Add(seq);
}


SequenceData SeqDataPlugIn::PopSequence(const char* callsign) {
	// remove a SequenceData and return
	int idx;
	SequenceData sd;
	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++)
		if (!strcmp(m_SequenceArray[idx].m_callsign, callsign)) {
			sd = m_SequenceArray[idx];
			m_SequenceArray.RemoveAt(idx);
			break;
		}
	return sd;
}


void SeqDataPlugIn::EditStatus(const char* callsign, const int method) {
	// method: 0-reset, 1=next standby, 2=next cleared, -1=previous
	int idx;
	for (idx = 0;
		idx < m_SequenceArray.GetCount() && strcmp(callsign, m_SequenceArray[idx].m_callsign);
		idx++);
	if (idx == m_SequenceArray.GetCount()) return;

	SequenceData sd;
	switch (method)
	{
	case 0: // reset
		m_SequenceArray.RemoveAt(idx);
		AddSequence(callsign);
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

	SequenceData sd = PopSequence(seqdata.m_callsign); // remove original
	int idx, seq;
	for (idx = seq = 0; idx < m_SequenceArray.GetCount(); idx++)
		if (seqdata.m_status == m_SequenceArray[idx].m_status)
			if (++seq == target_seq)
				m_SequenceArray.InsertAt(idx, sd);
}



void SeqDataPlugIn::RemoveAll(void) {
	m_SequenceArray.RemoveAll();
}


void SeqDataPlugIn::RemoveOffline(void) {
	int idx;
	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++)
		if (!IsCallsignOnline(m_SequenceArray[idx].m_callsign))
			m_SequenceArray.RemoveAt(idx);
}


// below are inherited functions


void SeqDataPlugIn::OnTimer(int Counter) {
	// update active information for all aircrafts
	if (Counter % CustomRefreshIntv) // once every n seconds
		return;

	CRadarTarget rt;
	int idx;
	bool online;
	for (idx = 0; idx < m_SequenceArray.GetCount(); idx++) {
		online = IsCallsignOnline(m_SequenceArray[idx].m_callsign);
		m_SequenceArray[idx].m_active = online;
		if (!online) { // avoid potential errors
			TRACE("%s\tinactive\n", m_SequenceArray[idx].m_callsign);
			continue;
		}
		rt = RadarTargetSelect(m_SequenceArray[idx].m_callsign);
		if (rt.GetGS() > CustomMaxSpeed) { // criterion for taking off
			TRACE("%s\tremoved\n", m_SequenceArray[idx].m_callsign);
			m_SequenceArray.RemoveAt(idx);
		}
	}
}
