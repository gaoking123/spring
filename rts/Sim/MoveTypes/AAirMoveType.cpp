/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include "StdAfx.h"
#include "AAirMoveType.h"
#include "MoveMath/MoveMath.h"

#include "Sim/Units/Unit.h"
#include "Sim/Units/UnitDef.h"
#include "Sim/Units/CommandAI/CommandAI.h"

CR_BIND_DERIVED_INTERFACE(AAirMoveType, AMoveType);

CR_REG_METADATA(AAirMoveType, (
		CR_MEMBER(oldGoalPos),
		CR_MEMBER(oldpos),
		CR_MEMBER(reservedLandingPos),

		CR_MEMBER(wantedHeight),

		CR_MEMBER(collide),
		CR_MEMBER(useSmoothMesh),
		CR_MEMBER(lastColWarning),
		CR_MEMBER(lastColWarningType),

		CR_MEMBER(autoLand),

		CR_RESERVED(16)
		));

AAirMoveType::AAirMoveType(CUnit* unit) :
	AMoveType(unit),
	aircraftState(AIRCRAFT_LANDED),
	oldGoalPos(owner? owner->pos : ZeroVector),
	oldpos(ZeroVector),
	reservedLandingPos(-1.0f, -1.0f, -1.0f),
	wantedHeight(80.0f),
	collide(true),
	useSmoothMesh(false),
	lastColWarning(NULL),
	lastColWarningType(0),
	autoLand(true)
{
	useHeading = false;
}

AAirMoveType::~AAirMoveType()
{
	if (reservedPad) {
		airBaseHandler->LeaveLandingPad(reservedPad);
		reservedPad = NULL;
	}
}

bool AAirMoveType::UseSmoothMesh() const {
	if (useSmoothMesh) {
		const bool onTransportMission =
			!owner->commandAI->commandQue.empty() &&
			((owner->commandAI->commandQue.front().id == CMD_LOAD_UNITS) || (owner->commandAI->commandQue.front().id == CMD_UNLOAD_UNIT));
		const bool repairing = reservedPad ? padStatus >= 1 : false;
		const bool forceDisableSmooth =
			repairing ||
			aircraftState == AIRCRAFT_LANDING ||
			aircraftState == AIRCRAFT_LANDED ||
			onTransportMission;
		return !forceDisableSmooth;
	} else {
		return false;
	}
}

void AAirMoveType::ReservePad(CAirBaseHandler::LandingPad* lp) {
	oldGoalPos = goalPos;
	AMoveType::ReservePad(lp);
	Takeoff();
}

void AAirMoveType::DependentDied(CObject* o) {
	AMoveType::DependentDied(o);

	if (o == reservedPad) {
		SetState(AIRCRAFT_FLYING);
		goalPos = oldGoalPos;
	}
}
