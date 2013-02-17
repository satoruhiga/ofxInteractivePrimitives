#include "ofxIPPatcher.h"

using namespace ofxInteractivePrimitives;

// PatchCord

PatchCord::PatchCord(Port *upstream_port, Port *downstream_port) : upstream(upstream_port), downstream(downstream_port)
{
	getUpstream()->addCord(this);
	getDownstream()->addCord(this);
}

void PatchCord::disconnect()
{
	getUpstream()->removeCord(this);
	getDownstream()->removeCord(this);
	
	delayedDelete();
}

void PatchCord::draw()
{
	if (!isValid()) return;
	
	const ofVec3f p0 = getUpstream()->getPos();
	const ofVec3f p1 = getUpstream()->getPatcher()->globalToLocalPos(getDownstream()->getGlobalPos());
	
	// cout << p0 << " / " << p1 << endl;
	cout << getDownstream()->getPatcher()->getPosition() << endl;
	ofLine(p0, p1);
}

// Port

Port::Port(BasePatcher *patcher, int index, PortIdentifer::Direction direction) : patcher(patcher), index(index), direction(direction)
{
}

void Port::execute(MessageRef message)
{
	if (direction == PortIdentifer::INPUT)
	{
		data = message;
		patcher->inputDataUpdated(index);
	}
	else if (direction == PortIdentifer::OUTPUT)
	{
		CordContainerType::iterator it = cords.begin();
		while (it != cords.end())
		{
			PatchCord *cord = *it;
			Port *port = cord->getDownstream();
			if (port) port->execute(message);
			
			it++;
		}
	}
}

ofVec3f Port::getGlobalPos() const
{
	return patcher->localToGlobalPos(getPos());
}

bool Port::hasConnectTo(Port *port)
{
	CordContainerType::iterator it = cords.begin();
	while (it != cords.end())
	{
		PatchCord *cord = *it;
		Port *p = cord->getDownstream();
		if (p == port) return true;
		
		it++;
	}
	
	return false;
}
