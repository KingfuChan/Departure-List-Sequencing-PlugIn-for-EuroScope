#pragma once

#include <iostream>
#include <string>

/*
namespace Status {
	const int INACTIVE = 0;
	const int STBY_CLEARANCE = 1;
	const int CLRD_CLEARANCE = 2;
	const int STBY_PUSHTAXI = 3;
	const int CLRD_PUSHTAXI = 4;
	const int STBY_TAKEOFF = 5;
	const int CLRD_TAKEOFF = 6;
	const int COMPLETED = 7;
};
*/

struct LinkedFlights {
	std::string callsign;
	int status;
	LinkedFlights* next;
};

namespace FlightRecorder {

	LinkedFlights* ConstructList(void);

	void DestroyList(LinkedFlights*);

	void AddFlight(LinkedFlights*, std::string);

	LinkedFlights* FindFlight(LinkedFlights*, std::string);

	void DeleteFlight(LinkedFlights*, std::string);

	void SetStatus(LinkedFlights*, std::string, int);

};