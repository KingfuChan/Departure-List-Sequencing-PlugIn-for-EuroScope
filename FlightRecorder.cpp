#include "pch.h"
#include "FlightRecorder.h"

LinkedFlights* FlightRecorder::ConstructList(void) {
	LinkedFlights* head = new LinkedFlights;
	head->next = nullptr;
	return head;
}


void FlightRecorder::DestroyList(LinkedFlights* head) {
	LinkedFlights* p, * q;
	p = head;
	while (p != nullptr) {
		q = p->next;
		delete p;
		p = q;
	}
}

void FlightRecorder::AddFlight(LinkedFlights* head, std::string	callsign) {
	LinkedFlights* p = head;
	for (; p->next != nullptr; p = p->next); //locate to tail
	LinkedFlights* add = new LinkedFlights;
	add->callsign = callsign;
	add->status = 0; // may be edited
	add->next = nullptr;
	p->next = add;
}

LinkedFlights* FlightRecorder::FindFlight(LinkedFlights* head, std::string callsign) {
	LinkedFlights* p = head->next;
	for (; p != nullptr && p->callsign != callsign; p = p->next);
	return p; //returns nullptr if not found
}

void FlightRecorder::DeleteFlight(LinkedFlights* head, std::string callsign) {
	LinkedFlights* p, * q;
	p = head; //previous node
	q = head->next; //node to remove
	while (q != nullptr) {
		if (!callsign.compare(p->callsign)) {
			p->next = q->next;
			delete q;
			return;
		}
		p = q;
		q = p->next;
	}
}

void FlightRecorder::SetStatus(LinkedFlights* head, std::string callsign ,int status) {
	LinkedFlights* p = FlightRecorder::FindFlight(head, callsign);
	if (p != nullptr)
		p->status = status;
}