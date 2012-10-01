/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */

#include "IPathManager.h"
#include "Default/PathManager.h"
#include "QTPFS/PathManager.hpp"
#include "Game/GlobalUnsynced.h"
#include "System/Log/ILog.h"

IPathManager* pathManager = NULL;
boost::thread* IPathManager::pathBatchThread = NULL;

IPathManager* IPathManager::GetInstance(unsigned int type) {
	static IPathManager* pm = NULL;

	if (pm == NULL) {
		const char* fmtStr = "[IPathManager::GetInstance] using %s path-manager";
		const char* typeStr = "";

		switch (type) {
			case PFS_TYPE_DEFAULT: { typeStr = "DEFAULT"; pm = new       CPathManager(); } break;
			case PFS_TYPE_QTPFS:   { typeStr = "QTPFS";   pm = new QTPFS::PathManager(); } break;
		}
#if THREADED_PATH
		pathBatchThread = new boost::thread(boost::bind<void, IPathManager, IPathManager*>(&IPathManager::ThreadFunc, pm));
#endif

		LOG(fmtStr, typeStr);
	}

	return pm;
}

IPathManager::IPathManager() : pathRequestID(0), wait(false), stopThread(false) {
}


IPathManager::~IPathManager() {
#if THREADED_PATH
	if (pathBatchThread != NULL) {
		{
			boost::mutex::scoped_lock preqLock(preqMutex);
			stopThread = true;
			if (wait) {
				wait = false;
				cond.notify_one();
			}
		}
		pathBatchThread->join();
		pathBatchThread = NULL;
	}
#endif
}


int IPathManager::GetPathID(int cid) {
	std::map<int, unsigned int>::iterator it = newPathCache.find(cid);
	if (it != newPathCache.end())
		return it->second;

	boost::mutex::scoped_lock preqLock(preqMutex);

	PathData* p = GetPathData(cid);
	return (p == NULL) ? -1 : p->pathID;
}


bool IPathManager::PathUpdated(MT_WRAP unsigned int pathID) {
	if (!Threading::threadedPath) {
		PathData* p = GetPathData(pathID);
		return (p != NULL && p->pathID >= 0) ? PathUpdated(ST_CALL p->pathID) : false;
	}
	boost::mutex::scoped_lock preqLock(preqMutex);
	PathData* p = GetPathData(pathID);
	pathOps.push_back(PathOpData(PATH_UPDATED, pathID));
	if (wait) {
		wait = false;
		cond.notify_one();
	}
	return (p != NULL && p->pathID >= 0) ? p->updated : false;
}


void IPathManager::UpdatePath(MT_WRAP const CSolidObject* owner, unsigned int pathID) {
	if (!Threading::threadedPath) {
		PathData* p = GetPathData(pathID);
		if (p != NULL && p->pathID >= 0)
			UpdatePath(ST_CALL owner, p->pathID);
		return;
	}
	boost::mutex::scoped_lock preqLock(preqMutex);
	pathOps.push_back(PathOpData(UPDATE_PATH, owner, pathID));
	if (wait) {
		wait = false;
		cond.notify_one();
	}
}


void IPathManager::DeletePath(MT_WRAP unsigned int pathID) {
	if (!Threading::threadedPath) {
		PathData* p = GetPathData(pathID);
		if (p != NULL && p->pathID >= 0)
			DeletePath(ST_CALL p->pathID);
		pathInfos.erase(pathID);
		return;
	}
	boost::mutex::scoped_lock preqLock(preqMutex);
	pathOps.push_back(PathOpData(DELETE_PATH, pathID));
	if (wait) {
		wait = false;
		cond.notify_one();
	}
}


float3 IPathManager::NextWayPoint(
	MT_WRAP
	unsigned int pathID,
	float3 callerPos,
	float minDistance,
	int numRetries,
	const CSolidObject *owner,
	bool synced
	) {
		if (!Threading::threadedPath) {
			PathData* p = GetPathData(pathID);
			if (p == NULL || p->pathID < 0)
				return callerPos;
			p->nextWayPoint = NextWayPoint(ST_CALL p->pathID, callerPos, minDistance, numRetries, owner, synced);
			return p->nextWayPoint;
		}
		boost::mutex::scoped_lock preqLock(preqMutex);
		PathData* p = GetPathData(pathID);
		pathOps.push_back(PathOpData(NEXT_WAYPOINT, pathID, callerPos, minDistance, numRetries, owner, synced));
		if (wait) {
			wait = false;
			cond.notify_one();
		}
		if (p == NULL || p->pathID < 0)
			return callerPos;
		return p->nextWayPoint;
}


void IPathManager::GetPathWayPoints(
	MT_WRAP
	unsigned int pathID,
	std::vector<float3>& points,
	std::vector<int>& starts
	) {
		ScopedDisableThreading sdt;
		boost::mutex::scoped_lock preqLock(preqMutex);
		PathData* p = GetPathData(pathID);
		if (p == NULL || p->pathID < 0)
			return;
		return GetPathWayPoints(ST_CALL pathID, points, starts);
}


unsigned int IPathManager::RequestPath(
	MT_WRAP
	const MoveDef* moveDef,
	const float3& startPos,
	const float3& goalPos,
	float goalRadius,
	CSolidObject* caller,
	bool synced
	) {
		if (!Threading::threadedPath) {
			int cid = ++pathRequestID;
			pathInfos[cid] = PathData(RequestPath(ST_CALL moveDef, startPos, goalPos, goalRadius, caller, synced), startPos);
			return cid;
		}
		boost::mutex::scoped_lock preqLock(preqMutex);
		int cid = ++pathRequestID;
		pathOps.push_back(PathOpData(REQUEST_PATH, cid, moveDef, startPos, goalPos, goalRadius, caller, synced));
		pathInfos[cid] = PathData(-1, startPos);
		if (wait) {
			wait = false;
			cond.notify_one();
		}
		return cid;
}


void IPathManager::ThreadFunc() {
	streflop::streflop_init<streflop::Simple>();

	while(true) {
		std::vector<PathOpData> pops;
		{
			boost::mutex::scoped_lock preqLock(preqMutex);
			if (stopThread)
				return;
			if (pathOps.empty()) {
				if (wait)
					cond.notify_one();
				wait = true;
				cond.wait(preqLock);
			}
			pathOps.swap(pops);
		}

		for (std::vector<PathOpData>::iterator i = pops.begin(); i != pops.end(); ++i) {
			PathOpData &cid = *i;
			unsigned int pid;
			switch(cid.type) {
				case REQUEST_PATH:
					pid = RequestPath(ST_CALL cid.moveDef, cid.startPos, cid.goalPos, cid.goalRadius, const_cast<CSolidObject*>(cid.owner), cid.synced);
					newPathCache[cid.pathID] = pid;
					pathUpdates[cid.pathID].push_back(PathUpdateData(REQUEST_PATH, pid));
					break;
				case NEXT_WAYPOINT:
					pid = GetPathID(cid.pathID);
					if (pid >= 0)
						pathUpdates[cid.pathID].push_back(PathUpdateData(NEXT_WAYPOINT, NextWayPoint(ST_CALL pid, cid.startPos, cid.minDistance, cid.numRetries, cid.owner, cid.synced)));
					break;
				case UPDATE_PATH:
					pid = GetPathID(cid.pathID);
					if (pid >= 0)
						UpdatePath(ST_CALL cid.owner, pid);
					break;
				case PATH_UPDATED:
					pid = GetPathID(cid.pathID);
					if (pid >= 0)
						pathUpdates[cid.pathID].push_back(PathUpdateData(PATH_UPDATED, PathUpdated(ST_CALL pid)));
					break;
					/*			case TERRAIN_CHANGE:
					TerrainChange(ST_CALL cid.cx1, cid.cz1, cid.cx2, cid.cz2);
					break;*/
				case DELETE_PATH:
					pid = GetPathID(cid.pathID);
					if (pid >= 0) {
						DeletePath(ST_CALL pid);
						pathUpdates[cid.pathID].push_back(PathUpdateData(DELETE_PATH));
					}
					newPathCache.erase(cid.pathID);
					break;
				default:
					LOG_L(L_ERROR,"Invalid path request %d", cid.type);
			}
		}
	}
}

#include "System/Platform/CrashHandler.h"

void IPathManager::SynchronizeThread() {
	ASSERT_SINGLETHREADED_SIM();
	if (!Threading::threadedPath)
		return;

	boost::mutex::scoped_lock preqLock(preqMutex);
	if (!wait) {
		wait = true;
		cond.wait(preqLock);
	}

	for (std::map<unsigned int, std::vector<PathUpdateData> >::iterator i  = pathUpdates.begin(); i != pathUpdates.end(); ++i) {
		for (std::vector<PathUpdateData>::iterator v  = i->second.begin(); v != i->second.end(); ++v) {
			PathUpdateData &u = *v;
			switch(u.type) {
				case REQUEST_PATH:
					pathInfos[i->first].pathID = u.pathID;
					break;
				case NEXT_WAYPOINT:
					pathInfos[i->first].nextWayPoint = u.wayPoint;
					break;
				case PATH_UPDATED:
					pathInfos[i->first].updated = u.updated;
					break;
				case DELETE_PATH:
					pathInfos.erase(i->first);
					break;
				default:
					LOG_L(L_ERROR,"Invalid path update %d", u.type);
			}
		}
	}

	newPathCache.clear();
	pathUpdates.clear();
}
