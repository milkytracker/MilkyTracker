/*
 *  Copyright 2012 Julian Harnath
 *
 *  This file is part of Milkytracker.
 *
 *  Milkytracker is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Milkytracker is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Milkytracker.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __HAIKU_MILKYMIDICONSUMER_H__
#define __HAIKU_MILKYMIDICONSUMER_H__

#include <MidiConsumer.h>

class BMidiProducer;
class Tracker;

class MilkyMidiConsumer : public BMidiLocalConsumer
{
public:
						MilkyMidiConsumer(Tracker* tracker, sem_id trackerLock);
	virtual				~MilkyMidiConsumer();

	virtual	void		NoteOn(uchar channel, uchar note, uchar velocity,
							bigtime_t time);
	virtual	void		NoteOff(uchar channel, uchar note, uchar velocity,
							bigtime_t time);

			void		ConnectProducer(int32 id);

			void		SetRecordVelocity(bool recordVelocity)
							{ fRecordVelocity = recordVelocity; }
			void		SetVelocityAmplify(int amplification)
							{ fVelocityAmplify = amplification; }

private:
			sem_id		fTrackerLock;
			Tracker*	fTracker;
			bool		fRecordVelocity;
			int			fVelocityAmplify;
			BMidiProducer*	fConnectedProducer;

};

#endif // __HAIKU_MILKYMIDICONSUMER_H__
