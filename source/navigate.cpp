//
// Copyright (c) 2003-2009, by Yet Another POD-Bot Development Team.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.ebot_aim_spring_stiffness_y
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// $Id:$
//

#include <core.h>

ConVar ebot_aimbot("ebot_aimbot", "0");
ConVar ebot_zombies_as_path_cost("ebot_zombie_count_as_path_cost", "1");
ConVar ebot_ping_affects_aim("ebot_ping_affects_aim", "0");
ConVar ebot_aim_type("ebot_aim_type", "1");
ConVar ebot_use_old_jump_method("ebot_use_old_jump_method", "0");
ConVar ebot_has_semiclip("ebot_has_semiclip", "0");
ConVar ebot_breakable_health_limit("ebot_breakable_health_limit", "3000.0");
ConVar ebot_path_smoothing("ebot_path_smoothing", "0");
ConVar ebot_stuck_detect_height("ebot_stuck_detect_height", "72.0");
ConVar ebot_force_shortest_path("ebot_force_shortest_path", "0");
ConVar ebot_pathfinder_seed_min("ebot_pathfinder_seed_min", "0.5");
ConVar ebot_pathfinder_seed_max("ebot_pathfinder_seed_max", "5.0");

static bool ohShit;
int Bot::FindGoal(void)
{
	if (IsZombieMode())
	{
		if (m_isZombieBot)
		{
			if (g_waypoint->m_terrorPoints.IsEmpty())
				return m_chosenGoalIndex = crandomint(0, g_numWaypoints - 1);

			return m_chosenGoalIndex = g_waypoint->m_terrorPoints.Random();
		}

		if (IsValidWaypoint(m_myMeshWaypoint))
			return m_chosenGoalIndex = m_myMeshWaypoint;

		if (IsValidWaypoint(m_zhCampPointIndex))
			return m_chosenGoalIndex = m_zhCampPointIndex;

		if (!g_waypoint->m_zmHmPoints.IsEmpty())
			return m_chosenGoalIndex = g_waypoint->m_zmHmPoints.Random();

		return m_chosenGoalIndex = crandomint(0, g_numWaypoints - 1);
	}
	else if (GetGameMode() == MODE_BASE)
	{
		if (g_mapType & MAP_CS)
		{
			if (m_team == TEAM_COUNTER)
			{
				
				if (HasHostage())
				{
					ohShit = true;
					if (!g_waypoint->m_rescuePoints.IsEmpty())
					{
						if (!IsValidWaypoint(m_chosenGoalIndex) || !(g_waypoint->GetPath(m_chosenGoalIndex)->flags & WAYPOINT_RESCUE))
							return m_chosenGoalIndex = g_waypoint->m_rescuePoints.Random();

						return m_chosenGoalIndex;
					}
				}
				else
				{
					ohShit = false;
					if (m_personality == PERSONALITY_CAREFUL && !g_waypoint->m_goalPoints.IsEmpty())
						return m_chosenGoalIndex = g_waypoint->m_goalPoints.Random();

					if (!g_waypoint->m_ctPoints.IsEmpty() && crandomint(1, 10) == 1)
						return m_chosenGoalIndex = g_waypoint->m_ctPoints.Random();

					if (!g_waypoint->m_goalPoints.IsEmpty())
						return m_chosenGoalIndex = g_waypoint->m_goalPoints.Random();
				}
			}
			else
			{
				if (!g_waypoint->m_rescuePoints.IsEmpty() && (ohShit || crandomint(1, 11) == 1))
					return m_chosenGoalIndex = g_waypoint->m_rescuePoints.Random();

				if (!g_waypoint->m_goalPoints.IsEmpty() && crandomint(1, 5) == 1)
					return m_chosenGoalIndex = g_waypoint->m_goalPoints.Random();

				if (!g_waypoint->m_terrorPoints.IsEmpty())
					return m_chosenGoalIndex = g_waypoint->m_terrorPoints.Random();
			}
		}
		else if (g_mapType & MAP_DE)
		{
			if (g_bombPlanted)
			{
				const bool noTimeLeft = OutOfBombTimer();

				if (GetCurrentTaskID() != TASK_ESCAPEFROMBOMB)
				{
					if (noTimeLeft)
					{
						TaskComplete();
						PushTask(TASK_ESCAPEFROMBOMB, TASKPRI_ESCAPEFROMBOMB, -1, 2.0f, true);
					}
					else if (m_team == TEAM_COUNTER)
					{
						const Vector bombOrigin = g_waypoint->GetBombPosition();
						if (bombOrigin != nullvec)
						{
							if (IsBombDefusing(bombOrigin))
							{
								if (GetCurrentTaskID() != TASK_CAMP && GetCurrentTaskID() != TASK_GOINGFORCAMP)
								{
									m_chosenGoalIndex = FindDefendWaypoint(bombOrigin);
									if (IsValidWaypoint(m_chosenGoalIndex))
									{
										m_campposition = g_waypoint->GetPath(m_chosenGoalIndex)->origin;
										PushTask(TASK_GOINGFORCAMP, TASKPRI_GOINGFORCAMP, m_chosenGoalIndex, GetBombTimeleft(), true);
										return m_chosenGoalIndex;
									}
								}
								else
									return m_chosenGoalIndex;
							}
							else
							{
								if (GetCurrentTaskID() == TASK_CAMP || GetCurrentTaskID() == TASK_GOINGFORCAMP)
									TaskComplete();
								else if (g_bombSayString)
								{
									ChatMessage(CHAT_PLANTBOMB);
									g_bombSayString = false;
								}

								m_chosenGoalIndex = g_waypoint->FindNearest(bombOrigin, 999999.0f);
								if (IsValidWaypoint(m_chosenGoalIndex))
									return m_chosenGoalIndex;
							}
						}
					}
					else
					{
						const Vector bombOrigin = g_waypoint->GetBombPosition();
						if (bombOrigin != nullvec)
						{
							if (IsBombDefusing(bombOrigin))
							{
								if (GetCurrentTaskID() == TASK_CAMP || GetCurrentTaskID() == TASK_GOINGFORCAMP)
									TaskComplete();

								m_chosenGoalIndex = g_waypoint->FindNearest(bombOrigin, 999999.0f);
								if (IsValidWaypoint(m_chosenGoalIndex))
									return m_chosenGoalIndex;
							}
							else
							{
								if (GetCurrentTaskID() != TASK_CAMP && GetCurrentTaskID() != TASK_GOINGFORCAMP)
								{
									m_chosenGoalIndex = FindDefendWaypoint(bombOrigin);
									if (IsValidWaypoint(m_chosenGoalIndex))
									{
										m_campposition = g_waypoint->GetPath(m_chosenGoalIndex)->origin;
										PushTask(TASK_GOINGFORCAMP, TASKPRI_GOINGFORCAMP, m_chosenGoalIndex, GetBombTimeleft(), true);
										return m_chosenGoalIndex;
									}
								}
								else
									return m_chosenGoalIndex;
							}
						}
					}
				}
			}
			else
			{
				if (m_isSlowThink)
					m_loosedBombWptIndex = FindLoosedBomb();

				if (IsValidWaypoint(m_loosedBombWptIndex))
				{
					if (m_team == TEAM_COUNTER)
					{
						if (GetCurrentTaskID() != TASK_CAMP && GetCurrentTaskID() != TASK_GOINGFORCAMP)
						{
							m_chosenGoalIndex = FindDefendWaypoint(g_waypoint->GetPath(m_loosedBombWptIndex)->origin);
							if (IsValidWaypoint(m_chosenGoalIndex))
							{
								m_campposition = g_waypoint->GetPath(m_chosenGoalIndex)->origin;
								PushTask(TASK_GOINGFORCAMP, TASKPRI_GOINGFORCAMP, m_chosenGoalIndex, GetBombTimeleft(), true);
								return m_chosenGoalIndex;
							}
						}
						else
							return m_chosenGoalIndex;
					}
					else
						return m_chosenGoalIndex = m_loosedBombWptIndex;
				}
				else
				{
					if (m_team == TEAM_COUNTER)
					{
						if (!g_waypoint->m_ctPoints.IsEmpty())
						{
							m_chosenGoalIndex = g_waypoint->m_ctPoints.Random();
							if (IsValidWaypoint(m_chosenGoalIndex))
								return m_chosenGoalIndex;
						}
					}
					else
					{
						if (m_isBomber)
						{
							m_loosedBombWptIndex = -1;
							if (!IsValidWaypoint(m_chosenGoalIndex) || !(g_waypoint->GetPath(m_chosenGoalIndex)->flags & WAYPOINT_GOAL))
							{
								if (!g_waypoint->m_goalPoints.IsEmpty())
									return m_chosenGoalIndex = g_waypoint->m_goalPoints.Random();
							}
						}
						else if (!g_waypoint->m_terrorPoints.IsEmpty())
						{
							m_chosenGoalIndex = g_waypoint->m_terrorPoints.Random();
							if (IsValidWaypoint(m_chosenGoalIndex))
								return m_chosenGoalIndex;
						}
					}
				}
			}
		}
		else if (g_mapType & MAP_AS)
		{
			if (m_team == TEAM_COUNTER)
			{
				if (m_isVIP && !g_waypoint->m_goalPoints.IsEmpty())
					return m_chosenGoalIndex = g_waypoint->m_goalPoints.Random();
				else
				{
					if (!g_waypoint->m_goalPoints.IsEmpty() && crandomint(1, 2) == 1)
						return m_chosenGoalIndex = g_waypoint->m_goalPoints.Random();
					else if (!g_waypoint->m_ctPoints.IsEmpty())
						return m_chosenGoalIndex = g_waypoint->m_ctPoints.Random();
				}
			}
			else
			{
				if (!g_waypoint->m_goalPoints.IsEmpty() && crandomint(1, 11) == 1)
					return m_chosenGoalIndex = g_waypoint->m_goalPoints.Random();
				else if (!g_waypoint->m_terrorPoints.IsEmpty())
					return m_chosenGoalIndex = g_waypoint->m_terrorPoints.Random();
			}
		}
	}

	if (!FNullEnt(m_lastEnemy) && IsAlive(m_lastEnemy))
	{
		const Vector origin = GetEntityOrigin(m_lastEnemy);
		if (origin != nullvec)
		{
			m_chosenGoalIndex = g_waypoint->FindNearest(origin, 9999999.0f, -1, m_lastEnemy);
			if (IsValidWaypoint(m_chosenGoalIndex))
				return m_chosenGoalIndex;
		}
	}

	return m_chosenGoalIndex = crandomint(0, g_numWaypoints - 1);
}

bool Bot::GoalIsValid(void)
{
	const int goal = GetCurrentGoalID();
	if (!IsValidWaypoint(goal)) // not decided about a goal
		return false;

	if (goal == m_currentWaypointIndex) // no nodes needed
		return true;

	if (m_navNode.IsEmpty()) // no path calculated
		return false;

	if (goal == m_navNode.Last())
		return true;

	return false;
}

// this function is a main path navigation
bool Bot::DoWaypointNav(void)
{
	// check if we need to find a waypoint...
	if (!IsValidWaypoint(m_currentWaypointIndex))
		GetValidWaypoint();
	else if (m_waypointOrigin != nullvec)
	{
		m_cachedWaypointIndex = m_currentWaypointIndex;
		m_destOrigin = m_waypointOrigin;

		if (!IsOnLadder() && !(m_waypoint.flags & WAYPOINT_LADDER) && pev->origin.z < m_destOrigin.z && cabsf(m_destOrigin.z - pev->origin.z) > ebot_stuck_detect_height.GetFloat())
		{
			m_isStuck = true;
			DeleteSearchNodes();
			m_currentWaypointIndex = -1;
			FindWaypoint(false);
			SetWaypointOrigin();
			return false;
		}
	}

	if (m_waypoint.flags & WAYPOINT_FALLCHECK)
	{
		TraceResult tr{};
		const Vector origin = g_waypoint->m_paths[m_currentWaypointIndex].origin;
		TraceLine(origin, origin - Vector(0.0f, 0.0f, 60.0f), false, false, GetEntity(), &tr);
		if (tr.flFraction == 1.0f)
		{
			DeleteSearchNodes();
			m_currentWaypointIndex = -1;
			FindWaypoint(false);
			SetWaypointOrigin();
			IgnoreCollisionShortly();
			m_moveSpeed = 0.0f;
			m_strafeSpeed = 0.0f;
			m_jumpFinished = false;
			m_checkTerrain = false;
			return false;
		}
	}
	else if (m_waypoint.flags & WAYPOINT_WAITUNTIL)
	{
		TraceResult tr{};
		const Vector origin = g_waypoint->m_paths[m_currentWaypointIndex].origin;
		TraceLine(origin, origin - Vector(0.0f, 0.0f, 60.0f), false, false, GetEntity(), &tr);
		if (tr.flFraction == 1.0f)
		{
			IgnoreCollisionShortly();
			m_moveSpeed = 0.0f;
			m_strafeSpeed = 0.0f;
			m_jumpFinished = false;
			m_checkTerrain = false;
			return false;
		}
	}

	// this waypoint has additional travel flags - care about them
	if (!m_jumpFinished && m_currentTravelFlags & PATHFLAG_JUMP && !m_isSlowThink)
	{
		// cheating for jump, bots cannot do some hard jumps and double jumps too
		// who cares about double jump for bots? :)

		auto jump = [&](void)
		{
			IgnoreCollisionShortly();
			if (ebot_use_old_jump_method.GetBool())
			{
				pev->velocity.x = (m_destOrigin.x - pev->origin.x) * (1.0f + (pev->maxspeed / 500.0f) + pev->gravity);
				pev->velocity.y = (m_destOrigin.y - pev->origin.y) * (1.0f + (pev->maxspeed / 500.0f) + pev->gravity);
			}
			else
			{
				const Vector myOrigin = GetBottomOrigin(GetEntity());
				Vector waypointOrigin = m_destOrigin;
				Vector walkableOrigin = GetWalkablePosition(waypointOrigin + pev->velocity * m_frameInterval, GetEntity(), true);

				if (m_waypoint.flags & WAYPOINT_CROUCH)
					waypointOrigin.z -= 18.0f;
				else
					waypointOrigin.z -= 36.0f;

				if (walkableOrigin != nullvec && (waypointOrigin - walkableOrigin).GetLengthSquared() < squaredf(8.0f))
					waypointOrigin = walkableOrigin;
				else
				{
					walkableOrigin = GetWalkablePosition(waypointOrigin + pev->velocity * -m_frameInterval, GetEntity(), true);
					if (walkableOrigin != nullvec && (waypointOrigin - walkableOrigin).GetLengthSquared() < squaredf(8.0f))
						waypointOrigin = walkableOrigin;
				}

				const float heightDifference = waypointOrigin.z - myOrigin.z;
				const float timeToReachWaypoint = csqrtf(squaredf(waypointOrigin.x - myOrigin.x) + squaredf(waypointOrigin.y - myOrigin.y) + squaredf(waypointOrigin.z - myOrigin.z)) / pev->maxspeed;
				pev->velocity.x = (waypointOrigin.x - myOrigin.x) / timeToReachWaypoint;
				pev->velocity.y = (waypointOrigin.y - myOrigin.y) / timeToReachWaypoint;
				pev->velocity.z = (heightDifference * pev->gravity * squaredf(timeToReachWaypoint)) / timeToReachWaypoint;
			}
		};

		jump();
		pev->button |= (IN_DUCK | IN_JUMP);
		jump();

		m_jumpFinished = true;
		m_checkTerrain = false;
		m_duckTime = 0.0f;
	}
	else if (m_waypoint.flags & WAYPOINT_CROUCH && !(m_waypoint.flags & WAYPOINT_CAMP))
		m_duckTime = engine->GetTime() + 1.0f;

	const float inter = (m_frameInterval + g_pGlobals->frametime) * 0.54f;
	float waypointDistance = 0.0f;
	float waypointDistance2 = 0.0f;
	const Vector origin = pev->origin + pev->velocity * inter;
	const Vector wpOrigin = m_destOrigin + pev->velocity * -inter;

	if (m_waypoint.flags & WAYPOINT_LADDER || IsOnLadder())
	{
		waypointDistance = (origin - wpOrigin).GetLengthSquared();
		waypointDistance2 = (pev->origin - m_destOrigin).GetLengthSquared();
		m_aimStopTime = 0.0f;
		if (m_destOrigin.z > (origin.z + 16.0f))
			m_destOrigin = m_waypoint.origin + Vector(0, 0, 16.0f);
		else if (m_destOrigin.z < origin.z + 16.0f && !IsOnLadder() && IsOnFloor())
		{
			m_moveSpeed = csqrtf(waypointDistance);
			if (m_moveSpeed < 150.0f)
				m_moveSpeed = 150.0f;
			else if (m_moveSpeed > pev->maxspeed)
				m_moveSpeed = pev->maxspeed;

			// never stop/stuck in ladder...
			pev->speed = m_moveSpeed;
		}
	}
	else if (!(pev->flags & FL_ONGROUND) && m_jumpTime > engine->GetTime()) // jumping
	{
		waypointDistance = (origin - wpOrigin).GetLengthSquared();
		waypointDistance2 = (pev->origin - m_destOrigin).GetLengthSquared();
	}
	else
	{
		waypointDistance = (origin - wpOrigin).GetLengthSquared2D();
		waypointDistance2 = (pev->origin - m_destOrigin).GetLengthSquared2D();
	}

	float desiredDistance;

	// initialize the radius for a special waypoint type, where the wpt is considered to be reached
	if (m_currentTravelFlags & PATHFLAG_JUMP)
		desiredDistance = squaredf(4.0f);
	else if (m_waypoint.flags & WAYPOINT_LIFT)
		desiredDistance = squaredf(48.0f);
	else if (m_waypoint.flags & WAYPOINT_LADDER)
		desiredDistance = squaredf(24.0f);
	else if (m_waypoint.radius > 4)
		desiredDistance = squaredi(m_waypoint.radius);
	else
		desiredDistance = squaredf(4.0f);

	if (m_jumpTime + 0.5f > engine->GetTime() && !IsOnFloor() && !IsOnLadder() && !IsInWater())
		waypointDistance = 9999999999.0f;
	else
		desiredDistance += cmaxf(inter, 68.0f - static_cast<float>(m_waypoint.radius)) / inter;

	if (waypointDistance < desiredDistance || waypointDistance2 < desiredDistance)
	{
		// use button waypoints
		if (m_waypoint.flags & WAYPOINT_USEBUTTON)
		{
			if (g_isXash)
			{
				if (!(pev->button & IN_USE) && !(pev->oldbuttons & IN_USE))
					pev->button |= IN_USE;
			}
			else
			{
				edict_t* button = FindButton();
				if (button != nullptr)
					MDLL_Use(button, GetEntity());
				else if (!(pev->button & IN_USE) && !(pev->oldbuttons & IN_USE))
					pev->button |= IN_USE;
			}
		}

		// did we reach a destination waypoint?
		if (GetCurrentGoalID() == m_currentWaypointIndex)
			return true;
		else if (m_navNode.IsEmpty())
			return false;

		m_navNode.Shift(); // advance in list
		m_currentTravelFlags = 0; // reset travel flags (jumping etc)

		// we're not at the end of the list?
		if (!m_navNode.IsEmpty())
		{
			const int destIndex = m_navNode.First();

			// find out about connection flags
			int8_t i;
			for (i = 0; i < Const_MaxPathIndex; i++)
			{
				if (m_waypoint.index[i] == destIndex)
				{
					m_currentTravelFlags = m_waypoint.connectionFlags[i];
					m_jumpFinished = false;
					break;
				}
			}

			ChangeWptIndex(destIndex);
			SetWaypointOrigin();
		}

		m_navTimeset = engine->GetTime();
		return false;
	}

	return false;
}

class PriorityQueue
{
public:
	PriorityQueue(void);
	~PriorityQueue(void);
	inline bool IsEmpty(void) const { return !m_size; }
	inline int16_t Size(void) const { return m_size; }
	inline void InsertLowest(const int16_t value, const float priority);
	//inline void InsertHighest(const int16_t value, const float priority);
	inline int16_t RemoveLowest(void);
	//inline int16_t RemoveHighest(void);
private:
	struct HeapNode
	{
		int16_t id;
		float priority;
	} *m_heap;
	int16_t m_size;
	int16_t m_heapSize;
};

PriorityQueue::PriorityQueue(void)
{
	m_size = 0;
	m_heapSize = static_cast<int16_t>((g_numWaypoints / 2) + 2);
	m_heap = static_cast<HeapNode*>(malloc(sizeof(HeapNode) * m_heapSize));
}

PriorityQueue::~PriorityQueue(void)
{
	if (m_heap != nullptr)
		free(m_heap);

	m_size = 0;
	m_heapSize = 0;
}

// inserts a value into the priority queue
void PriorityQueue::InsertLowest(const int16_t value, const float priority)
{
	if (m_size >= m_heapSize)
	{
		m_heapSize += static_cast<int16_t>(50);
		if (m_heap == nullptr)
			m_heap = static_cast<HeapNode*>(malloc(sizeof(HeapNode) * m_heapSize));
		else
		{
			HeapNode* hp = static_cast<HeapNode*>(realloc(m_heap, m_size));
			m_heap = hp;
			hp = nullptr;
		}
	}

	if (m_heap == nullptr)
		return;

	m_heap[m_size].priority = priority;
	m_heap[m_size].id = value;

	static int16_t child;
	static int16_t parent;
	static HeapNode temp;

	child = ++m_size - 1;
	while (child)
	{
		parent = (child - 1) / 2;
		if (m_heap[parent].priority < m_heap[child].priority)
			break;

		temp = m_heap[child];
		m_heap[child] = m_heap[parent];
		m_heap[parent] = temp;
		child = parent;
	}
}

// inserts a value into the priority queue
/*void PriorityQueue::InsertHighest(const int16_t value, const float priority)
{
	if (m_size >= m_heapSize)
	{
		m_heapSize += static_cast<int16_t>(50);
		if (m_heap == nullptr)
			m_heap = static_cast<HeapNode*>(malloc(sizeof(HeapNode) * m_heapSize));
		else
		{
			HeapNode* hp = static_cast<HeapNode*>(realloc(m_heap, m_size));
			m_heap = hp;
			hp = nullptr;
		}
	}

	if (m_heap == nullptr)
		return;

	m_heap[m_size].priority = priority;
	m_heap[m_size].id = value;

	static int16_t child;
	static int16_t parent;
	static HeapNode temp;

	child = ++m_size - 1;
	while (child)
	{
		parent = (child - 1) / 2;
		if (m_heap[parent].priority > m_heap[child].priority)
			break;

		temp = m_heap[child];
		m_heap[child] = m_heap[parent];
		m_heap[parent] = temp;
		child = parent;
	}
}*/

// removes the smallest item from the priority queue
int16_t PriorityQueue::RemoveLowest(void)
{
	if (m_heap == nullptr)
		return -1;

	static int16_t retID;
	retID = m_heap[0].id;

	m_size--;
	m_heap[0] = m_heap[m_size];

	static int16_t parent;
	static int16_t child;
	static int16_t rightChild;
	static HeapNode ref;

	parent = 0;
	child = (2 * parent) + 1;
	ref = m_heap[parent];
	while (child < m_size)
	{
		rightChild = (2 * parent) + 2;
		if (rightChild < m_size)
		{
			if (m_heap[rightChild].priority < m_heap[child].priority)
				child = rightChild;
		}

		if (ref.priority < m_heap[child].priority)
			break;

		m_heap[parent] = m_heap[child];
		parent = child;
		child = (2 * parent) + 1;
	}

	m_heap[parent] = ref;
	return retID;
}

// removes the largest item from the priority queue
/*int16_t PriorityQueue::RemoveHighest(void)
{
	if (m_heap == nullptr)
		return -1;

	static int16_t retID;
	retID = m_heap[0].id;

	m_size--;
	m_heap[0] = m_heap[m_size];

	static int16_t parent;
	static int16_t child;
	static int16_t rightChild;
	static HeapNode ref;

	parent = 0;
	child = (2 * parent) + 1;
	ref = m_heap[parent];
	while (child < m_size)
	{
		rightChild = (2 * parent) + 2;
		if (rightChild < m_size)
		{
			if (m_heap[rightChild].priority < m_heap[child].priority)
				child = rightChild;
		}

		if (ref.priority > m_heap[child].priority)
			break;

		m_heap[parent] = m_heap[child];
		parent = child;
		child = (2 * parent) + 1;
	}

	m_heap[parent] = ref;
	return retID;
}*/

inline const float HF_Distance(const int16_t& start, const int16_t& goal)
{
	return (g_waypoint->m_paths[start].origin - g_waypoint->m_paths[goal].origin).GetLength();
}

inline const float HF_Distance2D(const int16_t& start, const int16_t& goal)
{
	return (g_waypoint->m_paths[start].origin - g_waypoint->m_paths[goal].origin).GetLength2D();
}

inline const float HF_DistanceSquared(const int16_t& start, const int16_t& goal)
{
	return (g_waypoint->m_paths[start].origin - g_waypoint->m_paths[goal].origin).GetLengthSquared();
}

static Path pathCache;
static int8_t countCache;
inline const float GF_CostHuman(const int16_t& index, const int16_t& parent, const int32_t& parentFlags, const int8_t& team, const float& gravity, const bool& isZombie)
{
	if (!parentFlags)
		return HF_Distance2D(index, parent);

	if (parentFlags & WAYPOINT_AVOID)
		return 65355.0f;

	if (isZombie)
	{
		if (parentFlags & WAYPOINT_HUMANONLY)
			return 65355.0f;
	}
	else
	{
		if (parentFlags & WAYPOINT_ZOMBIEONLY)
			return 65355.0f;

		if (parentFlags & WAYPOINT_DJUMP)
			return 65355.0f;
	}

	pathCache = g_waypoint->m_paths[parent];
	if (parentFlags & WAYPOINT_ONLYONE)
	{
		for (const auto& client : g_clients)
		{
			if (!(client.flags & CFLAG_USED) || !(client.flags & CFLAG_ALIVE) || team != client.team)
				continue;

			if ((client.origin - pathCache.origin).GetLengthSquared() < squaredi(static_cast<int>(pathCache.radius) + 64))
				return 65355.0f;
		}
	}

	static float distance;
	static float totalDistance;
	totalDistance = 0.0f;
	countCache = 0;
	for (const auto& client : g_clients)
	{
		if (!(client.flags & CFLAG_USED) || !(client.flags & CFLAG_ALIVE) || team == client.team || !IsZombieEntity(client.ent))
			continue;

		distance = ((client.ent->v.origin + client.ent->v.velocity * g_pGlobals->frametime) - pathCache.origin).GetLengthSquared();
		if (distance < squaredi(static_cast<int>(pathCache.radius) + 128))
			countCache++;

		totalDistance += distance;
	}

	if (countCache > 0 && totalDistance > 0.0f)
		return (HF_Distance(index, parent) * static_cast<float>(countCache)) + totalDistance;

	return HF_Distance(index, parent);
}

inline const float GF_CostCareful(const int16_t& index, const int16_t& parent, const int32_t& parentFlags, const int8_t& team, const float& gravity, const bool& isZombie)
{
	if (!parentFlags)
		return HF_Distance2D(index, parent);

	if (parentFlags & WAYPOINT_AVOID)
		return 65355.0f;

	if (isZombie)
	{
		if (parentFlags & WAYPOINT_HUMANONLY)
			return 65355.0f;
	}
	else
	{
		if (parentFlags & WAYPOINT_ZOMBIEONLY)
			return 65355.0f;

		if (parentFlags & WAYPOINT_DJUMP)
			return 65355.0f;
	}

	if (parentFlags & WAYPOINT_ONLYONE)
	{
		pathCache = g_waypoint->m_paths[parent];
		for (const auto& client : g_clients)
		{
			if (!(client.flags & CFLAG_USED) || !(client.flags & CFLAG_ALIVE) || team != client.team)
				continue;

			if ((client.origin - pathCache.origin).GetLengthSquared() < squaredi(static_cast<int>(pathCache.radius) + 64))
				return 65355.0f;
		}
	}

	if (isZombie)
	{
		if (parentFlags & WAYPOINT_DJUMP)
		{
			pathCache = g_waypoint->m_paths[parent];
			countCache = 0;
			for (const auto& client : g_clients)
			{
				if (!(client.flags & CFLAG_USED) || !(client.flags & CFLAG_ALIVE) || client.team != team)
					continue;

				if ((client.origin - pathCache.origin).GetLengthSquared() < squaredi(static_cast<int>(pathCache.radius) + 512))
					countCache++;
				else if (IsVisible(pathCache.origin, client.ent))
					countCache++;
			}

			// don't count me
			if (countCache < static_cast<int8_t>(2))
				return 65355.0f;

			return HF_Distance2D(index, parent) / static_cast<float>(countCache);
		}
	}

	return HF_Distance2D(index, parent);
}

inline const float GF_CostNormal(const int16_t& index, const int16_t& parent, const int32_t& parentFlags, const int8_t& team, const float& gravity, const bool& isZombie)
{
	if (!parentFlags)
		return HF_Distance(index, parent);

	if (parentFlags & WAYPOINT_AVOID)
		return 65355.0f;

	if (isZombie)
	{
		if (parentFlags & WAYPOINT_HUMANONLY)
			return 65355.0f;
	}
	else
	{
		if (parentFlags & WAYPOINT_ZOMBIEONLY)
			return 65355.0f;

		if (parentFlags & WAYPOINT_DJUMP)
			return 65355.0f;
	}

	if (parentFlags & WAYPOINT_ONLYONE)
	{
		pathCache = g_waypoint->m_paths[parent];
		for (const auto& client : g_clients)
		{
			if (!(client.flags & CFLAG_USED) || !(client.flags & CFLAG_ALIVE) || team != client.team)
				continue;

			if ((client.origin - pathCache.origin).GetLengthSquared() < squaredi(static_cast<int>(pathCache.radius) + 64))
				return 65355.0f;
		}
	}

	if (isZombie)
	{
		if (parentFlags & WAYPOINT_DJUMP)
		{
			pathCache = g_waypoint->m_paths[parent];
			countCache = 0;
			for (const auto& client : g_clients)
			{
				if (!(client.flags & CFLAG_USED) || !(client.flags & CFLAG_ALIVE) || client.team != team)
					continue;

				if ((client.origin - pathCache.origin).GetLengthSquared() < squaredi(static_cast<int>(pathCache.radius) + 512))
					countCache++;
				else if (IsVisible(pathCache.origin, client.ent))
					countCache++;
			}

			// don't count me
			if (countCache < static_cast<int8_t>(2))
				return 65355.0f;

			return HF_Distance(index, parent) / static_cast<float>(countCache);
		}
	}

	if (parentFlags & WAYPOINT_LADDER)
		return HF_Distance(index, parent) * 2.0f;

	return HF_Distance(index, parent);
}

inline const float GF_CostRusher(const int16_t& index, const int16_t& parent, const int32_t& parentFlags, const int8_t& team, const float& gravity, const bool& isZombie)
{
	if (!parentFlags)
		return HF_Distance(index, parent);

	if (parentFlags & WAYPOINT_AVOID)
		return 65355.0f;

	if (isZombie)
	{
		if (parentFlags & WAYPOINT_HUMANONLY)
			return 65355.0f;
	}
	else
	{
		if (parentFlags & WAYPOINT_ZOMBIEONLY)
			return 65355.0f;

		if (parentFlags & WAYPOINT_DJUMP)
			return 65355.0f;
	}

	if (parentFlags & WAYPOINT_ONLYONE)
	{
		pathCache = g_waypoint->m_paths[parent];
		for (const auto& client : g_clients)
		{
			if (!(client.flags & CFLAG_USED) || !(client.flags & CFLAG_ALIVE) || team != client.team)
				continue;

			if ((client.origin - pathCache.origin).GetLengthSquared() < squaredi(static_cast<int>(pathCache.radius) + 64))
				return 65355.0f;
		}
	}

	// rusher bots never wait for boosting
	if (parentFlags & WAYPOINT_DJUMP)
		return 65355.0f;

	if (parentFlags & WAYPOINT_CROUCH)
		return HF_Distance(index, parent) * 2.0f;

	return HF_Distance(index, parent);
}

inline const float GF_CostNoHostage(const int16_t& index, const int16_t& parent, const int32_t& parentFlags, const int8_t& team, const float& gravity, const bool& isZombie)
{
	if (parentFlags & WAYPOINT_CROUCH)
		return 65355.0f;

	if (parentFlags & WAYPOINT_LADDER)
		return 65355.0f;

	int16_t neighbour;
	pathCache = g_waypoint->m_paths[index];
	for (countCache = 0; countCache < Const_MaxPathIndex; countCache++)
	{
		neighbour = pathCache.index[countCache];
		if (neighbour == parent)
		{
			if (pathCache.connectionFlags[countCache] & PATHFLAG_JUMP || pathCache.connectionFlags[countCache] & PATHFLAG_DOUBLE)
				return 65355.0f;
		}
	}

	return HF_Distance2D(index, parent);
}

// this function finds a path from srcIndex to destIndex
void Bot::FindPath(int& srcIndex, int& destIndex)
{
	if (g_pathTimer > engine->GetTime() && !m_navNode.IsEmpty())
		return;

	if (g_gameVersion == HALFLIFE || ebot_force_shortest_path.GetBool())
	{
		FindShortestPath(srcIndex, destIndex);
		return;
	}

	int16_t i;
	if (!IsValidWaypoint(srcIndex))
	{
		i = FindWaypoint(false);
		if (IsValidWaypoint(i))
			srcIndex = i;
		else
		{
			i = g_waypoint->FindNearest(pev->origin + pev->velocity * m_frameInterval, 1024.0f, -1, GetEntity());
			if (!IsValidWaypoint(i))
				return;

			srcIndex = i;
		}
	}

	if (!IsValidWaypoint(destIndex))
		return;

	if (srcIndex == destIndex)
	{
		DeleteSearchNodes();
		m_navNode.Add(destIndex);
		return;
	}

	const float (*gcalc) (const int16_t&, const int16_t&, const int32_t&, const int8_t&, const float&, const bool&) = nullptr;

	if (IsZombieMode() && ebot_zombies_as_path_cost.GetBool() && !m_isZombieBot)
		gcalc = GF_CostHuman;
	else if (g_bombPlanted && m_team == TEAM_COUNTER)
		gcalc = GF_CostRusher;
	else if (HasHostage())
		gcalc = GF_CostNoHostage;
	else if (m_isBomber || m_isVIP || (g_bombPlanted && m_inBombZone))
	{
		// move faster...
		if (g_timeRoundMid < engine->GetTime())
			gcalc = GF_CostRusher;
		else
			gcalc = GF_CostCareful;
	}
	else if (m_personality == PERSONALITY_CAREFUL)
		gcalc = GF_CostCareful;
	else if (m_personality == PERSONALITY_RUSHER)
		gcalc = GF_CostRusher;
	else
		gcalc = GF_CostNormal;

	if (gcalc == nullptr)
		return;

	int seed = m_index + m_numSpawns + m_currentWeapon;

	// if we stuck always randomize paths to better unstuck
	if (m_isStuck)
		seed += static_cast<int>(engine->GetTime() * 0.1f);

	float min = ebot_pathfinder_seed_min.GetFloat();
	float max = ebot_pathfinder_seed_max.GetFloat();

	struct AStar
	{
		float g;
		float f;
		int16_t parent;
		bool is_closed;
	} waypoints[g_numWaypoints];

	for (i = 0; i < g_numWaypoints; i++)
	{
		waypoints[i].g = 0.0f;
		waypoints[i].f = 0.0f;
		waypoints[i].parent = -1;
		waypoints[i].is_closed = false;
	}

	AStar& srcWaypoint = waypoints[srcIndex];
	srcWaypoint.g = gcalc(srcIndex, destIndex, 0, m_team, pev->gravity, m_isZombieBot);
	srcWaypoint.f = srcWaypoint.g + HF_Distance(srcIndex, destIndex); // we hate that first waypoint

	// loop cache
	AStar* currWaypoint;
	AStar* childWaypoint;
	int16_t currentIndex, self;
	int32_t flags;
	float g, f;

	// do not allow to search whole map
	const int16_t limit = static_cast<int16_t>((g_numWaypoints / 2) + 2);

	PriorityQueue openList;
	openList.InsertLowest(srcIndex, srcWaypoint.f);
	while (!openList.IsEmpty())
	{
		// remove the first waypoint from the open list
		currentIndex = openList.RemoveLowest();
		if (!IsValidWaypoint(currentIndex))
			break;

		// is the current waypoint the goal waypoint?
		if (currentIndex == destIndex || openList.Size() > limit)
		{
			// delete path for new one
			DeleteSearchNodes();

			// set the chosen goal value
			m_chosenGoalIndex = destIndex;

			do
			{
				m_navNode.Add(currentIndex);
				currentIndex = waypoints[currentIndex].parent;
			} while (IsValidWaypoint(currentIndex));

			m_navNode.Reverse();
			ChangeWptIndex(m_navNode.First());
			m_destOrigin = m_waypointOrigin;
			m_jumpFinished = false;
			g_pathTimer = engine->GetTime() + 0.25f;
			IgnoreCollisionShortly();
			return;
		}

		currWaypoint = &waypoints[currentIndex];
		if (currWaypoint->is_closed)
			continue;

		// set current waypoint as closed
		currWaypoint->is_closed = true;

		// now expand the waypoint
		pathCache = g_waypoint->m_paths[currentIndex];
		for (i = 0; i < Const_MaxPathIndex; i++)
		{
			self = pathCache.index[i];
			if (!IsValidWaypoint(self))
				continue;

			if (flags = g_waypoint->m_paths[self].flags)
			{
				if (flags & WAYPOINT_FALLCHECK)
				{
					TraceResult tr{};
					const Vector origin = g_waypoint->m_paths[self].origin;
					TraceLine(origin, origin - Vector(0.0f, 0.0f, 60.0f), false, false, GetEntity(), &tr);
					if (tr.flFraction == 1.0f)
						continue;
				}
				else if (flags & WAYPOINT_SPECIFICGRAVITY)
				{
					if ((pev->gravity * engine->GetGravity()) > g_waypoint->m_paths[self].gravity)
						continue;
				}
			}

			// calculate the F value as F = G + H
			g = currWaypoint->g + (gcalc(currentIndex, self, flags, m_team, pev->gravity, m_isZombieBot) * crandomfloatfast(seed, min, max));
			f = g + HF_Distance(self, destIndex);

			childWaypoint = &waypoints[self];
			if (!childWaypoint->is_closed || childWaypoint->f > f)
			{
				// put the current child into open list
				childWaypoint->parent = currentIndex;
				childWaypoint->g = g;
				childWaypoint->f = f;
				openList.InsertLowest(self, f);
			}
		}
	}

	// roam around poorly :(
	MiniArray <int16_t> PossiblePath;
	for (i = 0; i < g_numWaypoints; i++)
	{
		if (waypoints[i].is_closed)
			PossiblePath.Push(i);
	}

	if (!PossiblePath.IsEmpty())
	{
		int ref = PossiblePath.Random();
		FindShortestPath(srcIndex, ref);
		return;
	}

	FindShortestPath(srcIndex, destIndex);
}

void Bot::FindShortestPath(int& srcIndex, int& destIndex)
{
	int16_t i;
	if (!IsValidWaypoint(srcIndex))
	{
		i = FindWaypoint(false);
		if (IsValidWaypoint(i))
			srcIndex = i;
		else
		{
			i = g_waypoint->FindNearest(pev->origin + pev->velocity * m_frameInterval, 1024.0f, -1, GetEntity());
			if (!IsValidWaypoint(i))
				return;

			srcIndex = i;
		}
	}

	if (!IsValidWaypoint(destIndex))
		return;

	if (srcIndex == destIndex)
	{
		DeleteSearchNodes();
		m_navNode.Add(destIndex);
		return;
	}

	struct AStar
	{
		float g;
		float f;
		int16_t parent;
		bool is_closed;
	} waypoints[g_numWaypoints];

	for (i = 0; i < g_numWaypoints; i++)
	{
		waypoints[i].g = 0.0f;
		waypoints[i].f = 0.0f;
		waypoints[i].parent = -1;
		waypoints[i].is_closed = false;
	}

	AStar& srcWaypoint = waypoints[srcIndex];
	srcWaypoint.f = HF_DistanceSquared(srcIndex, destIndex);

	// loop cache
	AStar* currWaypoint;
	AStar* childWaypoint;
	int16_t currentIndex, self;
	int32_t flags;
	float f;

	// do not allow to search whole map
	const int16_t limit = static_cast<int16_t>((g_numWaypoints / 2) + 2);

	PriorityQueue openList;
	openList.InsertLowest(srcIndex, srcWaypoint.f);
	while (!openList.IsEmpty())
	{
		// remove the first waypoint from the open list
		currentIndex = openList.RemoveLowest();
		if (!IsValidWaypoint(currentIndex))
			break;

		// is the current waypoint the goal waypoint?
		if (currentIndex == destIndex || openList.Size() > limit)
		{
			// delete path for new one
			DeleteSearchNodes();

			// set the chosen goal value
			m_chosenGoalIndex = destIndex;

			do
			{
				m_navNode.Add(currentIndex);
				currentIndex = waypoints[currentIndex].parent;
			} while (IsValidWaypoint(currentIndex));

			m_navNode.Reverse();
			ChangeWptIndex(m_navNode.First());
			m_destOrigin = m_waypointOrigin;
			m_jumpFinished = false;
			g_pathTimer = engine->GetTime() + 0.25f;
			IgnoreCollisionShortly();
			return;
		}

		currWaypoint = &waypoints[currentIndex];
		if (currWaypoint->is_closed)
			continue;

		// set current waypoint as closed
		currWaypoint->is_closed = true;

		// now expand the waypoint
		pathCache = g_waypoint->m_paths[currentIndex];
		for (i = 0; i < Const_MaxPathIndex; i++)
		{
			self = pathCache.index[i];
			if (!IsValidWaypoint(self))
				continue;

			if (flags = g_waypoint->m_paths[self].flags)
			{
				if (flags & WAYPOINT_FALLCHECK)
				{
					TraceResult tr{};
					const Vector origin = g_waypoint->m_paths[self].origin;
					TraceLine(origin, origin - Vector(0.0f, 0.0f, 60.0f), false, false, GetEntity(), &tr);
					if (tr.flFraction == 1.0f)
						continue;
				}
				else if (flags & WAYPOINT_SPECIFICGRAVITY)
				{
					if ((pev->gravity * engine->GetGravity()) > g_waypoint->m_paths[self].gravity)
						continue;
				}
			}

			f = HF_DistanceSquared(self, destIndex);
			childWaypoint = &waypoints[self];
			if (!childWaypoint->is_closed || childWaypoint->f > f)
			{
				// put the current child into open list
				childWaypoint->parent = currentIndex;
				childWaypoint->f = f;
				openList.InsertLowest(self, f);
			}
		}
	}
}

void Bot::DeleteSearchNodes(void)
{
	m_navNode.Clear();
}

void Bot::CheckTouchEntity(edict_t* entity)
{
	if (FNullEnt(entity))
		return;

	// if we won't be able to break it, don't try
	if (entity->v.takedamage == DAMAGE_NO)
	{
		// defuse bomb
		if (m_team == TEAM_COUNTER && cstrcmp(STRING(entity->v.model) + 9, "c4.mdl") == 0)
		{
			if (GetCurrentTaskID() != TASK_DEFUSEBOMB)
			{
				// notify team of defusing
				if (m_numFriendsLeft > 0)
					RadioMessage(Radio_CoverMe);

				m_moveToGoal = false;
				m_checkTerrain = false;

				m_moveSpeed = 0.0f;
				m_strafeSpeed = 0.0f;

				PushTask(TASK_DEFUSEBOMB, TASKPRI_DEFUSEBOMB, -1, 0.0f, false);
			}
		}

		return;
	}

	// see if it's breakable
	if ((FClassnameIs(entity, "func_breakable") || (FClassnameIs(entity, "func_pushable") && (entity->v.spawnflags & SF_PUSH_BREAKABLE)) || FClassnameIs(entity, "func_wall")) && entity->v.health > 0.0f && entity->v.health < ebot_breakable_health_limit.GetFloat())
	{
		edict_t* me = pev->pContainingEntity;
		TraceResult tr{};
		TraceHull(pev->origin, m_destOrigin, false, point_hull, me, &tr);

		TraceResult tr2{};
		TraceHull(EyePosition(), m_destOrigin, false, head_hull, me, &tr2);

		// double check
		if ((!FNullEnt(tr.pHit) && tr.pHit == entity) || (!FNullEnt(tr2.pHit) && tr2.pHit == entity))
		{
			m_breakableEntity = entity;
			m_breakable = (!FNullEnt(tr.pHit) && tr.pHit == entity) ? tr.vecEndPos : ((GetEntityOrigin(entity) * 0.5f) + (tr2.vecEndPos * 0.5f));
			m_destOrigin = m_breakable;

			PushTask(TASK_DESTROYBREAKABLE, TASKPRI_SHOOTBREAKABLE, -1, 1.0f, false);
			edict_t* ent;

			if (pev->origin.z > m_breakable.z) // make bots smarter
			{
				// tell my enemies to destroy it, so i will fall
				for (const auto& enemy : g_botManager->m_bots)
				{
					if (enemy == nullptr)
						continue;

					if (m_team == enemy->m_team)
						continue;

					if (!enemy->m_isAlive)
						continue;

					if (enemy->m_isZombieBot)
						continue;

					if (enemy->m_currentWeapon == WEAPON_KNIFE)
						continue;

					ent = enemy->pev->pContainingEntity;
					if (ent == nullptr)
						continue;

					TraceHull(enemy->EyePosition(), m_breakable, false, point_hull, ent, &tr);
					TraceHull(enemy->EyePosition(), m_breakable, false, head_hull, ent, &tr2);

					if ((!FNullEnt(tr.pHit) && tr.pHit == entity) || (!FNullEnt(tr2.pHit) && tr2.pHit == entity))
					{
						enemy->m_breakableEntity = entity;
						enemy->m_breakable = (!FNullEnt(tr.pHit) && tr.pHit == entity) ? tr.vecEndPos : ((GetEntityOrigin(entity) * 0.5f) + (tr2.vecEndPos * 0.5f));
						enemy->SelectBestWeapon();
						enemy->PushTask(TASK_DESTROYBREAKABLE, TASKPRI_SHOOTBREAKABLE, -1, 1.0f, false);
					}
				}
			}
			else if (!m_isZombieBot)
			{
				for (const auto& bot : g_botManager->m_bots)
				{
					if (bot == nullptr)
						continue;

					if (m_team != bot->m_team)
						continue;

					if (!bot->m_isAlive)
						continue;

					if (bot->m_isZombieBot)
						continue;

					ent = bot->pev->pContainingEntity;
					if (ent == nullptr)
						continue;

					if (me == ent)
						continue;

					TraceHull(bot->EyePosition(), m_breakable, false, point_hull, ent, &tr);
					TraceHull(bot->EyePosition(), m_breakable, false, head_hull, ent, &tr2);

					if ((!FNullEnt(tr.pHit) && tr.pHit == entity) || (!FNullEnt(tr2.pHit) && tr2.pHit == entity))
					{
						bot->m_breakableEntity = entity;
						bot->m_breakable = (!FNullEnt(tr.pHit) && tr.pHit == entity) ? tr.vecEndPos : ((GetEntityOrigin(entity) * 0.5f) + (tr2.vecEndPos * 0.5f));
						bot->SelectBestWeapon();
						bot->PushTask(TASK_DESTROYBREAKABLE, TASKPRI_SHOOTBREAKABLE, -1, 1.0f, false);
					}
				}
			}
		}
	}
}

void Bot::SetEnemy(edict_t* entity)
{
	if (FNullEnt(entity))
	{
		m_enemy = nullptr;
		m_enemyOrigin = nullvec;
		return;
	}

	if (!IsAlive(entity))
		return;

	if (m_team == GetTeam(entity))
		return;

	m_enemy = entity;
}

void Bot::SetLastEnemy(edict_t* entity)
{
	if (FNullEnt(entity))
	{
		m_lastEnemy = nullptr;
		m_lastEnemyOrigin = nullvec;
		return;
	}

	if (!IsAlive(entity))
		return;

	if (m_team == GetTeam(entity))
		return;

	m_lastEnemy = entity;
	m_lastEnemyOrigin = GetEntityOrigin(entity);
}

void Bot::SetMoveTarget(edict_t* entity)
{
	if (FNullEnt(entity) || !IsAlive(entity) || m_team == GetTeam(entity))
	{
		m_moveTargetEntity = nullptr;
		m_moveTargetOrigin = nullvec;

		if (GetCurrentTaskID() == TASK_MOVETOTARGET)
		{
			RemoveCertainTask(TASK_MOVETOTARGET);
			m_prevGoalIndex = -1;
			GetCurrentTask()->data = -1;
		}

		return;
	}

	m_moveTargetOrigin = GetEntityOrigin(entity);
	m_states &= ~STATE_SEEINGENEMY;
	SetEnemy(nullptr);
	SetLastEnemy(nullptr);
	m_enemyUpdateTime = 0.0f;
	m_aimFlags &= ~AIM_ENEMY;

	if (m_moveTargetEntity == entity)
		return;

	SetEntityWaypoint(entity);
	SetEntityWaypoint(GetEntity(), GetEntityWaypoint(entity));

	m_moveTargetEntity = entity;
	PushTask(TASK_MOVETOTARGET, TASKPRI_MOVETOTARGET, -1, 0.0, true);
}

// this function find a node in the near of the bot if bot had lost his path of pathfinder needs to be restarted over again
int Bot::FindWaypoint(const bool skipLag)
{
	if (skipLag && !m_isSlowThink && IsValidWaypoint(m_cachedWaypointIndex))
		return m_cachedWaypointIndex;

	int busy = -1;
	float lessDist[3] = {65355.0f, 65355.0f, 65355.0f};
	float distance;
	bool skip;
	int lessIndex[3] = {-1, -1, -1};
	int at;
	for (at = 0; at < g_numWaypoints; at++)
	{
		pathCache = g_waypoint->m_paths[at];
		if (m_team == TEAM_COUNTER && pathCache.flags & WAYPOINT_ZOMBIEONLY)
			continue;
		else if (m_team == TEAM_TERRORIST && pathCache.flags & WAYPOINT_HUMANONLY)
			continue;

		skip = false;

		// skip current and recent previous nodes
		if (at == m_prevWptIndex || at == m_currentWaypointIndex)
			skip = true;

		// skip the current node, if any
		if (skip)
			continue;

		// skip if isn't visible
		if (!IsVisible(pathCache.origin, GetEntity()))
			continue;

		// cts with hostages should not pick
		if (m_team == TEAM_COUNTER && HasHostage())
			continue;

		// check we're have link to it
		if (IsValidWaypoint(m_currentWaypointIndex) && !g_waypoint->IsConnected(m_currentWaypointIndex, at))
			continue;

		// ignore non-reacheable nodes...
		if (!g_waypoint->IsNodeReachable(m_waypoint.origin, pathCache.origin))
			continue;

		// check if node is already used by another bot...
		if (IsWaypointOccupied(at))
		{
			busy = at;
			continue;
		}

		// if we're still here, find some close nodes
		distance = (pev->origin - pathCache.origin).GetLengthSquared();
		if (distance < lessDist[0])
		{
			lessDist[2] = lessDist[1];
			lessIndex[2] = lessIndex[1];

			lessDist[1] = lessDist[0];
			lessIndex[1] = lessIndex[0];

			lessDist[0] = distance;
			lessIndex[0] = at;
		}
		else if (distance < lessDist[1])
		{
			lessDist[2] = lessDist[1];
			lessIndex[2] = lessIndex[1];

			lessDist[1] = distance;
			lessIndex[1] = at;
		}
		else if (distance < lessDist[2])
		{
			lessDist[2] = distance;
			lessIndex[2] = at;
		}
	}

	int selected = -1;

	// now pick random one from choosen
	int index = 0;

	// choice from found
	if (IsValidWaypoint(lessIndex[2]))
		index = crandomint(0, 2);
	else if (IsValidWaypoint(lessIndex[1]))
		index = crandomint(0, 1);
	else if (IsValidWaypoint(lessIndex[0])) 
		index = 0;

	selected = lessIndex[index];

	// if we're still have no node and have busy one (by other bot) pick it up
	if (!IsValidWaypoint(selected) && IsValidWaypoint(busy)) 
		selected = busy;

	// worst case... find atleast something
	if (!IsValidWaypoint(selected))
		selected = g_waypoint->FindNearest(pev->origin + pev->velocity, 9999.0f, -1, GetEntity());

	ChangeWptIndex(selected);
	return selected;
}

void Bot::IgnoreCollisionShortly(void)
{
	m_lastCollTime = engine->GetTime() + 1.0f;
	m_isStuck = false;
	m_checkTerrain = false;
}

Vector Smooth(const Vector& one, const Vector& second)
{
	return (one + second) * 0.5f;
}

Vector SmoothMax(const Vector& one, const Vector& second, const Vector& third, const Vector& last)
{
	return (one + second + third + last) * 0.25f;
}

Vector SmoothMaxMax(const Vector& one, const Vector& second, const Vector& third, const Vector& last, const Vector& onemore)
{
	return (one + second + third + last + onemore) * 0.20f;
}

void Bot::SetWaypointOrigin(void)
{
	m_waypointOrigin = m_waypoint.origin;
	if (ebot_path_smoothing.GetBool())
	{
		bool isSmooth = false;
		if (IsOnLadder())
		{
			m_aimStopTime = 0.0f;
			TraceResult tr{};
			TraceLine(Vector(pev->origin.x, pev->origin.y, pev->absmin.z), m_waypointOrigin, true, true, GetEntity(), &tr);

			if (tr.flFraction < 1.0f)
				m_waypointOrigin = m_waypointOrigin + (pev->origin - m_waypointOrigin) * 0.5f + Vector(0.0f, 0.0f, 32.0f);
		}
		else
		{
			const auto myFlags = m_waypoint.flags;
			const int length = m_navNode.Length();
			if (length > 2 && !(myFlags & WAYPOINT_JUMP) && !(myFlags & WAYPOINT_LADDER) && !(myFlags & WAYPOINT_FALLCHECK) && !(myFlags & WAYPOINT_FALLRISK))
			{
				if (length > 3)
				{
					const Vector origin = g_waypoint->GetPath(m_navNode.Get(0))->origin;
					if (IsVisible(origin, GetEntity()))
					{
						const Vector origin2 = g_waypoint->GetPath(m_navNode.Get(1))->origin;
						if (IsVisible(origin2, GetEntity()))
						{
							Vector origin3;
							if (length > 3 && IsVisible((origin3 = g_waypoint->GetPath(m_navNode.Get(2))->origin), GetEntity()))
							{
								isSmooth = true;
								m_waypointOrigin = SmoothMaxMax(m_prevOrigin, m_waypointOrigin, origin, origin2, origin3);
							}
							else
							{
								isSmooth = true;
								m_waypointOrigin = SmoothMax(m_prevOrigin, m_waypointOrigin, origin, origin2);
							}
						}
						else
						{
							isSmooth = true;
							m_waypointOrigin = Smooth(m_waypointOrigin, origin);
						}
					}
				}
				else
				{
					const Vector& origin = g_waypoint->GetPath(m_navNode.Next())->origin;
					if (IsVisible(origin, GetEntity()))
					{
						isSmooth = true;
						m_waypointOrigin = Smooth(m_waypointOrigin, origin);
					}
				}
			}
		}

		if (isSmooth)
			return;
	}

	const float radius = static_cast<float>(m_waypoint.radius);
	if (radius > 0.0f)
	{
		MakeVectors(Vector(pev->angles.x, AngleNormalize(pev->angles.y + crandomfloat(-90.0f, 90.0f)), 0.0f));
		int sPoint = -1;

		if (m_navNode.HasNext())
		{
			Vector waypointOrigin[5];
			int i;
			for (i = 0; i < 5; i++)
			{
				waypointOrigin[i] = m_waypointOrigin;
				waypointOrigin[i] += Vector(crandomfloat(-radius, radius), crandomfloat(-radius, radius), 0.0f);
			}

			float sDistance = 65355.0f;
			float distance;
			for (i = 0; i < 5; i++)
			{
				distance = (pev->origin - waypointOrigin[i]).GetLengthSquared();
				if (distance < sDistance)
				{
					sPoint = i;
					sDistance = distance;
				}
			}

			if (sPoint != -1)
				m_waypointOrigin = waypointOrigin[sPoint];
		}

		if (sPoint == -1)
			m_waypointOrigin = m_waypointOrigin + pev->view_ofs + g_pGlobals->v_forward;
	}

	if (IsOnLadder())
	{
		m_aimStopTime = 0.0f;
		TraceResult tr{};
		TraceLine(Vector(pev->origin.x, pev->origin.y, pev->absmin.z), m_waypointOrigin, false, false, GetEntity(), &tr);
		if (tr.flFraction < 1.0f)
		{
			if (m_waypointOrigin.z >= pev->origin.z)
				m_waypointOrigin += tr.vecPlaneNormal;
			else
				m_waypointOrigin -= tr.vecPlaneNormal;
		}
	}
}

// checks if the last waypoint the bot was heading for is still valid
void Bot::GetValidWaypoint(void)
{
	bool needFindWaypont = false;
	if (!IsValidWaypoint(m_currentWaypointIndex))
		needFindWaypont = true;
	else if ((m_navTimeset + GetEstimatedReachTime() < engine->GetTime()))
		needFindWaypont = true;
	else
	{
		const int client = ENTINDEX(GetEntity()) - 1;
		if (m_currentWaypointIndex != g_clients[client].wpIndex && m_currentWaypointIndex != g_clients[client].wpIndex2 && (m_waypoint.origin - pev->origin).GetLengthSquared() > squaredf(600.0f) && !g_waypoint->Reachable(GetEntity(), m_currentWaypointIndex))
			needFindWaypont = true;
	}

	if (needFindWaypont)
	{
		DeleteSearchNodes();
		m_currentWaypointIndex = -1;
		FindWaypoint(false);
		SetWaypointOrigin();
	}
}

void Bot::ChangeWptIndex(const int waypointIndex)
{
	bool badPrevWpt = true;
	for (const auto& link : m_waypoint.index)
	{
		if (link == waypointIndex)
			badPrevWpt = false;
	}

	if (badPrevWpt)
		m_prevWptIndex = -1;
	else
		m_prevWptIndex = m_currentWaypointIndex;

	m_currentWaypointIndex = waypointIndex;
	m_navTimeset = engine->GetTime();
	m_waypoint = g_waypoint->m_paths[waypointIndex];
}

int Bot::FindDefendWaypoint(const Vector origin)
{
	// where to defend?
	if (origin == nullvec)
		return -1;

	// no camp waypoints
	if (g_waypoint->m_campPoints.IsEmpty())
		return -1;

	// invalid index
	if (!IsValidWaypoint(m_currentWaypointIndex))
		return -1;

	MiniArray <int16_t> BestSpots;
	MiniArray <int16_t> OkSpots;
	MiniArray <int16_t> WorstSpots;

	const int16_t size = g_waypoint->m_campPoints.Size();

	TraceResult tr{};
	int16_t i;
	int16_t index;
	for (i = 0; i < size; i++)
	{
		index = g_waypoint->m_campPoints.Get(i);
		if (g_waypoint->GetPath(index)->flags & WAYPOINT_LADDER)
			continue;

		if (m_team == TEAM_COUNTER && g_waypoint->GetPath(index)->flags & WAYPOINT_TERRORIST)
			continue;
		else if (g_waypoint->GetPath(index)->flags & WAYPOINT_COUNTER)
			continue;

		if (!IsWaypointOccupied(index))
		{
			TraceLine(g_waypoint->GetPath(index)->origin, origin, true, true, GetEntity(), &tr);

			if (tr.flFraction == 1.0f) // distance isn't matter
				BestSpots.Push(index);
			else if ((g_waypoint->GetPath(index)->origin - origin).GetLengthSquared() < squaredf(1024.0f))
				OkSpots.Push(index);
			else
				WorstSpots.Push(index);
		}
	}

	if (!BestSpots.IsEmpty())
		return BestSpots.Random();

	if (!OkSpots.IsEmpty())
		return OkSpots.Random();

	if (!WorstSpots.IsEmpty())
		return WorstSpots.Random();

	return -1;
}

// checks if bot is blocked in his movement direction (excluding doors)
bool Bot::CantMoveForward(const Vector normal, TraceResult* tr)
{
	// first do a trace from the bot's eyes forward...
	Vector src = EyePosition();
	Vector forward = src + normal * 24.0f;

	MakeVectors(Vector(0.0f, pev->angles.y, 0.0f));

	// trace from the bot's eyes straight forward...
	TraceLine(src, forward, true, pev->pContainingEntity, tr);

	// check if the trace hit something...
	if (tr->flFraction < 1.0f)
	{
		if (cstrncmp("func_door", STRING(tr->pHit->v.classname), 9) == 0)
			return false;

		return true;  // bot's head will hit something
	}

	// bot's head is clear, check at shoulder level...
	// trace from the bot's shoulder left diagonal forward to the right shoulder...
	src = EyePosition() + Vector(0.0f, 0.0f, -16.0f) - g_pGlobals->v_right * 16.0f;
	forward = EyePosition() + Vector(0.0f, 0.0f, -16.0f) + g_pGlobals->v_right * 16.0f + normal * 24.0f;

	TraceLine(src, forward, true, pev->pContainingEntity, tr);

	// check if the trace hit something...
	if (tr->flFraction < 1.0f && cstrncmp("func_door", STRING(tr->pHit->v.classname), 9) != 0)
		return true;  // bot's body will hit something

	// bot's head is clear, check at shoulder level...
	// trace from the bot's shoulder right diagonal forward to the left shoulder...
	src = EyePosition() + Vector(0.0f, 0.0f, -16.0f) + g_pGlobals->v_right * 16.0f;
	forward = EyePosition() + Vector(0.0f, 0.0f, -16.0f) - g_pGlobals->v_right * 16.0f + normal * 24.0f;

	TraceLine(src, forward, true, pev->pContainingEntity, tr);

	// check if the trace hit something...
	if (tr->flFraction < 1.0f && cstrncmp("func_door", STRING(tr->pHit->v.classname), 9) != 0)
		return true;  // bot's body will hit something

	// now check below waist
	if (pev->flags & FL_DUCKING)
	{
		src = pev->origin + Vector(0.0f, 0.0f, -19.0f + 19.0f);
		forward = src + Vector(0.0f, 0.0f, 10.0f) + normal * 24.0f;

		TraceLine(src, forward, true, pev->pContainingEntity, tr);

		// check if the trace hit something...
		if (tr->flFraction < 1.0f && cstrncmp("func_door", STRING(tr->pHit->v.classname), 9) != 0)
			return true;  // bot's body will hit something

		src = pev->origin;
		forward = src + normal * 24.0f;

		TraceLine(src, forward, true, pev->pContainingEntity, tr);

		// check if the trace hit something...
		if (tr->flFraction < 1.0f && cstrncmp("func_door", STRING(tr->pHit->v.classname), 9) != 0)
			return true;  // bot's body will hit something
	}
	else
	{
		// trace from the left waist to the right forward waist pos
		src = pev->origin + Vector(0.0f, 0.0f, -17.0f) - g_pGlobals->v_right * 16.0f;
		forward = pev->origin + Vector(0.0f, 0.0f, -17.0f) + g_pGlobals->v_right * 16.0f + normal * 24.0f;

		// trace from the bot's waist straight forward...
		TraceLine(src, forward, true, pev->pContainingEntity, tr);

		// check if the trace hit something...
		if (tr->flFraction < 1.0f && cstrncmp("func_door", STRING(tr->pHit->v.classname), 9) != 0)
			return true;  // bot's body will hit something

		// trace from the left waist to the right forward waist pos
		src = pev->origin + Vector(0.0f, 0.0f, -17.0f) + g_pGlobals->v_right * 16.0f;
		forward = pev->origin + Vector(0.0f, 0.0f, -17.0f) - g_pGlobals->v_right * 16.0f + normal * 24.0f;

		TraceLine(src, forward, true, pev->pContainingEntity, tr);

		// check if the trace hit something...
		if (tr->flFraction < 1.0f && cstrncmp("func_door", STRING(tr->pHit->v.classname), 9) != 0)
			return true;  // bot's body will hit something
	}

	return false;  // bot can move forward, return false
}

// this function check if bot can jump over some obstacle
bool Bot::CanJumpUp(const Vector normal)
{
	// can't jump if not on ground and not on ladder/swimming
	if (!IsOnFloor() && (IsOnLadder() || !IsInWater()))
		return false;

	TraceResult tr{};
	MakeVectors(Vector(0.0f, pev->angles.y, 0.0f));

	// check for normal jump height first...
	Vector src = pev->origin + Vector(0.0f, 0.0f, -36.0f + 45.0f);
	Vector dest = src + normal * 32.0f;

	// trace a line forward at maximum jump height...
	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	if (tr.flFraction < 1.0f)
		goto CheckDuckJump;
	else
	{
		// now trace from jump height upward to check for obstructions...
		src = dest;
		dest.z = dest.z + 37.0f;

		TraceLine(src, dest, true, pev->pContainingEntity, &tr);

		if (tr.flFraction < 1.0f)
			return false;
	}

	// now check same height to one side of the bot...
	src = pev->origin + g_pGlobals->v_right * 16.0f + Vector(0.0f, 0.0f, -36.0f + 45.0f);
	dest = src + normal * 32.0f;

	// trace a line forward at maximum jump height...
	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	// if trace hit something, return false
	if (tr.flFraction < 1.0f)
		goto CheckDuckJump;

	// now trace from jump height upward to check for obstructions...
	src = dest;
	dest.z = dest.z + 37.0f;

	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	// if trace hit something, return false
	if (tr.flFraction < 1.0f)
		return false;

	// now check same height on the other side of the bot...
	src = pev->origin + (-g_pGlobals->v_right * 16.0f) + Vector(0.0f, 0.0f, -36.0f + 45.0f);
	dest = src + normal * 32.0f;

	// trace a line forward at maximum jump height...
	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	// if trace hit something, return false
	if (tr.flFraction < 1.0f)
		goto CheckDuckJump;

	// now trace from jump height upward to check for obstructions...
	src = dest;
	dest.z = dest.z + 37.0f;

	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	// if trace hit something, return false
	return tr.flFraction > 1.0f;

	// here we check if a duck jump would work...
CheckDuckJump:
	// use center of the body first... maximum duck jump height is 62, so check one unit above that (63)
	src = pev->origin + Vector(0.0f, 0.0f, -36.0f + 63.0f);
	dest = src + normal * 32.0f;

	// trace a line forward at maximum jump height...
	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	if (tr.flFraction < 1.0f)
		return false;
	else
	{
		// now trace from jump height upward to check for obstructions...
		src = dest;
		dest.z = dest.z + 37.0f;

		TraceLine(src, dest, true, pev->pContainingEntity, &tr);

		// if trace hit something, check duckjump
		if (tr.flFraction < 1.0f)
			return false;
	}

	// now check same height to one side of the bot...
	src = pev->origin + g_pGlobals->v_right * 16.0f + Vector(0.0f, 0.0f, -36.0f + 63.0f);
	dest = src + normal * 32.0f;

	// trace a line forward at maximum jump height...
	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	// if trace hit something, return false
	if (tr.flFraction < 1.0f)
		return false;

	// now trace from jump height upward to check for obstructions...
	src = dest;
	dest.z = dest.z + 37.0f;

	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	// if trace hit something, return false
	if (tr.flFraction < 1.0f)
		return false;

	// now check same height on the other side of the bot...
	src = pev->origin + (-g_pGlobals->v_right * 16.0f) + Vector(0.0f, 0.0f, -36.0f + 63.0f);
	dest = src + normal * 32.0f;

	// trace a line forward at maximum jump height...
	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	// if trace hit something, return false
	if (tr.flFraction < 1.0f)
		return false;

	// now trace from jump height upward to check for obstructions...
	src = dest;
	dest.z = dest.z + 37.0f;

	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	// if trace hit something, return false
	return tr.flFraction > 1.0f;
}

// this function check if bot can duck under obstacle
bool Bot::CanDuckUnder(const Vector normal)
{
	TraceResult tr{};
	Vector baseHeight;

	// use center of the body first...
	if (pev->flags & FL_DUCKING)
		baseHeight = pev->origin + Vector(0.0f, 0.0f, -17.0f);
	else
		baseHeight = pev->origin;

	Vector src = baseHeight;
	Vector dest = src + normal * 32.0f;

	// trace a line forward at duck height...
	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	// if trace hit something, return false
	if (tr.flFraction < 1.0f)
		return false;

	MakeVectors(Vector(0.0f, pev->angles.y, 0.0f));

	// now check same height to one side of the bot...
	src = baseHeight + g_pGlobals->v_right * 16.0f;
	dest = src + normal * 32.0f;

	// trace a line forward at duck height...
	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	// if trace hit something, return false
	if (tr.flFraction < 1.0f)
		return false;

	// now check same height on the other side of the bot...
	src = baseHeight + (-g_pGlobals->v_right * 16.0f);
	dest = src + normal * 32.0f;

	// trace a line forward at duck height...
	TraceLine(src, dest, true, pev->pContainingEntity, &tr);

	// if trace hit something, return false
	return tr.flFraction > 1.0f;
}

bool Bot::CheckWallOnBehind(void)
{
	TraceResult tr{};
	MakeVectors(pev->angles);

	// do a trace to the left...
	TraceLine(pev->origin, pev->origin - g_pGlobals->v_forward * 54.0f, false, false, GetEntity(), &tr);

	// check if the trace hit something...
	if (tr.flFraction != 1.0f)
	{
		m_lastWallOrigin = tr.flFraction;
		return true;
	}
	else
	{
		TraceLine(tr.vecEndPos, tr.vecEndPos - g_pGlobals->v_up * 54.0f, false, false, GetEntity(), &tr);

		// we don't want fall
		if (tr.flFraction == 1.0f)
		{
			m_lastWallOrigin = pev->origin;
			return true;
		}
	}

	return false;
}

bool Bot::CheckWallOnLeft(void)
{
	TraceResult tr{};
	MakeVectors(pev->angles);

	// do a trace to the left...
	TraceLine(pev->origin, pev->origin - g_pGlobals->v_right * 54.0f, false, false, GetEntity(), &tr);

	// check if the trace hit something...
	if (tr.flFraction != 1.0f)
	{
		m_lastWallOrigin = tr.flFraction;
		return true;
	}
	else
	{
		TraceLine(tr.vecEndPos, tr.vecEndPos - g_pGlobals->v_up * 54.0f, false, false, GetEntity(), &tr);

		// we don't want fall
		if (tr.flFraction == 1.0f)
		{
			m_lastWallOrigin = pev->origin;
			return true;
		}
	}

	return false;
}

bool Bot::CheckWallOnRight(void)
{
	TraceResult tr{};
	MakeVectors(pev->angles);

	// do a trace to the right...
	TraceLine(pev->origin, pev->origin + g_pGlobals->v_right * 54.0f, false, false, GetEntity(), &tr);

	// check if the trace hit something...
	if (tr.flFraction != 1.0f)
	{
		m_lastWallOrigin = tr.flFraction;
		return true;
	}
	else
	{
		TraceLine(tr.vecEndPos, tr.vecEndPos - g_pGlobals->v_up * 54.0f, false, false, GetEntity(), &tr);

		// we don't want fall
		if (tr.flFraction == 1.0f)
		{
			m_lastWallOrigin = pev->origin;
			return true;
		}
	}

	return false;
}

// this function eturns if given location would hurt Bot with falling damage
bool Bot::IsDeadlyDrop(const Vector targetOriginPos)
{
	Vector botPos = pev->origin;
	TraceResult tr{};

	const Vector move((targetOriginPos - botPos).ToYaw(), 0.0f, 0.0f);
	MakeVectors(move);

	Vector direction = (targetOriginPos - botPos).Normalize();  // 1 unit long
	Vector check = botPos;
	Vector down = botPos;

	down.z = down.z - 1000.0f;  // straight down 1000 units

	TraceHull(check, down, true, head_hull, GetEntity(), &tr);

	if (tr.flFraction > 0.036f) // We're not on ground anymore?
		tr.flFraction = 0.036f;

	float height;
	float lastHeight = tr.flFraction * 1000.0f;  // height from ground

	float distance = (targetOriginPos - check).GetLengthSquared();  // distance from goal

	while (distance > squaredf(16.0f))
	{
		check = check + direction * 16.0f; // move 10 units closer to the goal...

		down = check;
		down.z = down.z - 1000.0f;  // straight down 1000 units

		TraceHull(check, down, true, head_hull, GetEntity(), &tr);

		if (tr.fStartSolid) // Wall blocking?
			return false;

		height = tr.flFraction * 1000.0f;  // height from ground

		if (lastHeight < height - 100.0f) // Drops more than 100 Units?
			return true;

		lastHeight = height;
		distance = (targetOriginPos - check).GetLengthSquared();  // distance from goal
	}

	return false;
}

void Bot::CheckCloseAvoidance(const Vector& dirNormal)
{
	if (ebot_has_semiclip.GetBool())
		return;

	if (GetGameMode() == MODE_DM)
		return;

	if (GetGameMode() == MODE_NOTEAM)
		return;

	if (pev->solid == SOLID_NOT)
		return;

	float distance = squaredf(512.0f);
	float maxSpeed = squaredf(pev->maxspeed);
	m_avoid = nullptr;

	// get our priority
	const unsigned int myPri = GetPlayerPriority(pev->pContainingEntity);
	unsigned int otherPri;

	float nearest;

	// find nearest player to bot
	for (const auto& client : g_clients)
	{
		// only valid meat
		if (FNullEnt(client.ent))
			continue;

		// need only good meat
		if (!(client.flags & CFLAG_USED))
			continue;

		// and still alive meat
		if (!(client.flags & CFLAG_ALIVE))
			continue;

		// our team, alive and not myself?
		if (client.team != m_team || client.ent == pev->pContainingEntity)
			continue;

		// get priority of other player
		otherPri = GetPlayerPriority(client.ent);

		// if our priority is better, don't budge
		if (myPri < otherPri)
			continue;

		// they are higher priority - make way, unless we're already making way for someone more important
		if (!FNullEnt(m_avoid) && m_avoid != client.ent)
		{
			if (GetPlayerPriority(m_avoid) < otherPri) // ignore because we're already avoiding someone better
				continue;
		}

		nearest = (pev->origin - client.ent->v.origin).GetLengthSquared();
		if (nearest < maxSpeed && nearest < distance)
		{
			m_avoid = client.ent;
			distance = nearest;
		}
	}

	// found somebody?
	if (FNullEnt(m_avoid))
		return;

	// don't get stuck in small areas, follow the same goal
	if (m_isStuck)
	{
		Bot* otherBot = g_botManager->GetBot(m_avoid);
		if (otherBot != nullptr)
		{
			m_prevGoalIndex = otherBot->m_prevGoalIndex;
			m_chosenGoalIndex = otherBot->m_chosenGoalIndex;

			int index = m_chosenGoalIndex;
			if (index == -1)
				index = m_prevGoalIndex;

			if (index != -1)
			{
				FindPath(m_currentWaypointIndex, index);
				otherBot->FindPath(m_currentWaypointIndex, index);
			}
		}
	}

	const float interval = (m_frameInterval * 4.0f) + g_pGlobals->frametime;

	// use our movement angles, try to predict where we should be next frame
	Vector right, forward;
	m_moveAngles.BuildVectors(&forward, &right, nullptr);

	Vector predict = pev->origin + forward * m_moveSpeed * interval;

	predict += right * m_strafeSpeed * interval;
	predict += pev->velocity * interval;

	nearest = (predict - m_avoid->v.origin).GetLengthSquared();
	maxSpeed = (pev->origin - (m_avoid->v.origin + m_avoid->v.velocity * interval)).GetLengthSquared();

	// is player that near now or in future that we need to steer away?
	if (nearest < squaredf(72.0f) || (distance < squaredf(80.0f) && maxSpeed < distance))
	{
		const Vector dir = (pev->origin - m_avoid->v.origin).Normalize2D();

		// to start strafing, we have to first figure out if the target is on the left side or right side
		if ((dir | right.Normalize2D()) > 0.0f)
			SetStrafeSpeed(dirNormal, pev->maxspeed);
		else
			SetStrafeSpeed(dirNormal, -pev->maxspeed);

		if (distance < squaredf(72.0f))
		{
			if ((dir | forward.Normalize2D()) < 0.0f)
				m_moveSpeed = -pev->maxspeed;
			else
				m_moveSpeed = pev->maxspeed;
		}
	}
}

int Bot::GetCampAimingWaypoint(void)
{
	MiniArray <int16_t> BestWaypoints;
	MiniArray <int16_t> OkWaypoints;

	int16_t currentWay = static_cast<int16_t>(m_currentWaypointIndex);
	if (!IsValidWaypoint(currentWay))
		currentWay = static_cast<int16_t>(g_waypoint->FindNearest(pev->origin));

	int16_t i;
	for (i = 0; i < g_numWaypoints; i++)
	{
		if (!IsValidWaypoint(i))
			continue;

		if (currentWay == i)
			continue;

		if (!IsVisible(g_waypoint->GetPath(i)->origin, GetEntity()))
			continue;

		if ((g_waypoint->GetPath(i)->origin - pev->origin).GetLengthSquared() > squaredf(512.0f))
			BestWaypoints.Push(i);
		else
			OkWaypoints.Push(i);
	}

	if (!BestWaypoints.IsEmpty())
		return BestWaypoints.Random();

	if (!OkWaypoints.IsEmpty())
		return OkWaypoints.Random();

	return crandomint(0, g_numWaypoints - 1);
}

void Bot::FacePosition(void)
{
	if (!m_isAlive)
		return;

	// predict enemy
	if (m_aimFlags & AIM_ENEMY && !FNullEnt(m_enemy))
	{
		extern ConVar ebot_ping;
		if (ebot_ping.GetBool() && ebot_ping_affects_aim.GetBool())
		{
			if (m_trackTime < engine->GetTime())
			{
				m_tempAim = m_enemy->v.velocity;
				m_trackTime = engine->GetTime() + (static_cast<float>(m_ping[2]) * 0.0025f);
			}
			else
			{
				m_tempAim.x += m_enemy->v.velocity.x * g_pGlobals->frametime;
				m_tempAim.y += m_enemy->v.velocity.y * g_pGlobals->frametime;

				// careful only
				if (m_personality == PERSONALITY_CAREFUL)
					m_tempAim.z += m_enemy->v.velocity.z * g_pGlobals->frametime;
			}

			// set position
			m_lookAt = m_tempAim;
		}
		else
		{
			m_lookAt.x += m_enemy->v.velocity.x * g_pGlobals->frametime;
			m_lookAt.y += m_enemy->v.velocity.y * g_pGlobals->frametime;
			m_lookAt.z += m_enemy->v.velocity.z * g_pGlobals->frametime;
		}

		m_playerTargetTime = engine->GetTime();
	}

	// no spinbot please...
	if (m_lastAimTime < engine->GetTime() - 1.0f)
		m_lastAimTime = engine->GetTime();

	m_aimInterval = engine->GetTime() - m_lastAimTime;
	m_lastAimTime = engine->GetTime();

	if (ebot_aim_type.GetInt() == 2)
	{
		m_idealAngles = pev->v_angle;
		Vector direction = (m_lookAt - EyePosition()).ToAngles() + pev->punchangle;
		direction.x = -direction.x; // invert for engine

		const float floatSkill = ((static_cast<float>(m_skill) * 0.033f) + 9.0f) * m_aimInterval;
		m_idealAngles.x += AngleNormalize(direction.x - m_idealAngles.x) * floatSkill;
		m_idealAngles.y += AngleNormalize(direction.y - m_idealAngles.y) * floatSkill;

		if (m_idealAngles.x < -89.0f)
			m_idealAngles.x = -89.0f;
		else if (m_idealAngles.x > 89.0f)
			m_idealAngles.x = 89.0f;

		pev->v_angle = m_idealAngles;
		pev->angles.x = -pev->v_angle.x * 0.33333333333f;
		pev->angles.y = pev->v_angle.y;
		return;
	}

	if (m_aimFlags & AIM_NAVPOINT)
	{
		if (IsOnLadder() || m_waypoint.flags & WAYPOINT_LADDER)
			m_aimStopTime = 0.0f;

		if (m_aimStopTime > engine->GetTime())
			return;
	}
	else
		m_aimStopTime = 0.0f;

	// adjust all body and view angles to face an absolute vector
	Vector direction = (m_lookAt - EyePosition()).ToAngles() + pev->punchangle;
	direction.x = -direction.x; // invert for engine

	m_idealAngles = pev->v_angle;

	const float angleDiffPitch = AngleNormalize(direction.x - m_idealAngles.x);
	const float angleDiffYaw = AngleNormalize(direction.y - m_idealAngles.y);
	const float lockn = 0.11111f / m_aimInterval;

	if (angleDiffYaw < lockn && angleDiffYaw > -lockn)
	{
		m_lookYawVel = 0.0f;
		m_idealAngles.y = direction.y;
		m_aimStopTime = engine->GetTime() + crandomfloat(0.25f, 1.25f);
	}
	else
	{
		const float floatSkill = static_cast<float>(m_skill);
		const float accelerate = floatSkill * 40.0f;
		const float stiffness = floatSkill * 4.0f;
		const float damping = floatSkill * 0.4f;
		m_lookYawVel += m_aimInterval * cclampf((stiffness * angleDiffYaw) - (damping * m_lookYawVel), -accelerate, accelerate);
		m_idealAngles.y += m_aimInterval * m_lookYawVel;
	}

	if (angleDiffPitch < lockn && angleDiffPitch > -lockn)
	{
		m_lookPitchVel = 0.0f;
		m_idealAngles.x = direction.x;
	}
	else
	{
		const float floatSkill = static_cast<float>(m_skill);
		const float accelerate = floatSkill * 40.0f;
		const float stiffness = floatSkill * 4.0f;
		const float damping = floatSkill * 0.4f;
		m_lookPitchVel += m_aimInterval * cclampf(2.0f * stiffness * angleDiffPitch - (damping * m_lookPitchVel), -accelerate, accelerate);
		m_idealAngles.x += m_aimInterval * m_lookPitchVel;
	}

	if (m_idealAngles.x < -89.0f)
		m_idealAngles.x = -89.0f;
	else if (m_idealAngles.x > 89.0f)
		m_idealAngles.x = 89.0f;

	pev->v_angle = m_idealAngles;

	// set the body angles to point the gun correctly
	pev->angles.x = -pev->v_angle.x * 0.33333333333f;
	pev->angles.y = pev->v_angle.y;
}

void Bot::SetStrafeSpeed(const Vector moveDir, const float strafeSpeed)
{
	if (CheckWallOnBehind())
	{
		if (CheckWallOnRight())
			m_tempstrafeSpeed = -strafeSpeed;
		else if (CheckWallOnLeft())
			m_tempstrafeSpeed = strafeSpeed;
		
		m_strafeSpeed = m_tempstrafeSpeed;

		if ((m_isStuck || pev->speed >= pev->maxspeed) && !IsOnLadder() && m_jumpTime + 5.0f < engine->GetTime() && IsOnFloor())
			pev->button |= IN_JUMP;
	}
	else
	{
		MakeVectors(pev->angles);
		if (((moveDir - pev->origin).Normalize2D() | g_pGlobals->v_forward.SkipZ()) > 0 && !CheckWallOnRight())
			m_strafeSpeed = strafeSpeed;
		else if (!CheckWallOnLeft())
			m_strafeSpeed = -strafeSpeed;
		else if (GetCurrentTaskID() == TASK_CAMP)
			m_lastEnemy = nullptr;
	}
}

// find hostage improve
int Bot::FindHostage(void)
{
	if (m_team != TEAM_COUNTER || !(g_mapType & MAP_CS) || m_isZombieBot)
		return -1;

	edict_t* ent = nullptr;
	while (!FNullEnt(ent = FIND_ENTITY_BY_CLASSNAME(ent, "hostage_entity")))
	{
		bool canF = true;
		int8_t j;

		for (const auto& bot : g_botManager->m_bots)
		{
			if (bot != nullptr && bot->m_isAlive)
			{
				for (j = 0; j < Const_MaxHostages; j++)
				{
					if (bot->m_hostages[j] == ent)
						canF = false;
				}
			}
		}

		const Vector entOrigin = GetEntityOrigin(ent);
		j = g_waypoint->FindNearest(entOrigin, 512.0f, -1, ent);
		if (IsValidWaypoint(j) && canF)
			return j;
		else
		{
			// do we need second try?
			j = g_waypoint->FindNearest(entOrigin);
			if (IsValidWaypoint(j) && canF)
				return j;
		}
	}

	return -1;
}

int Bot::FindLoosedBomb(void)
{
	if (GetGameMode() != MODE_BASE || m_team != TEAM_TERRORIST || !(g_mapType & MAP_DE))
		return -1; // don't search for bomb if the player is CT, or it's not defusing bomb

	edict_t* bombEntity = nullptr; // temporaly pointer to bomb

	// search the bomb on the map
	while (!FNullEnt(bombEntity = FIND_ENTITY_BY_CLASSNAME(bombEntity, "weaponbox")))
	{
		if (cstrcmp(STRING(bombEntity->v.model) + 9, "backpack.mdl") == 0)
		{
			const Vector bombOrigin = GetEntityOrigin(bombEntity);
			int nearestIndex = g_waypoint->FindNearest(bombOrigin, 512.0f, -1, bombEntity);
			if (IsValidWaypoint(nearestIndex))
				return nearestIndex;
			else
			{
				// do we need second try?
				nearestIndex = g_waypoint->FindNearest(bombOrigin);
				if (IsValidWaypoint(nearestIndex))
					return nearestIndex;
			}

			break;
		}
	}

	return -1;
}

bool Bot::IsWaypointOccupied(const int index)
{
	if (pev->solid == SOLID_NOT)
		return false;

	Bot* bot;
	Path* pointer;
	for (const auto& client : g_clients)
	{
		if (client.index < 0)
			continue;

		if (FNullEnt(client.ent))
			continue;

		if (!(client.flags & CFLAG_USED) || !(client.flags & CFLAG_ALIVE) || client.team != m_team || client.ent == GetEntity())
			continue;

		bot = g_botManager->GetBot(client.index);
		if (bot != nullptr)
		{
			if (bot->m_chosenGoalIndex == index)
				return true;

			if (bot->m_currentWaypointIndex == index)
				return true;

			if (bot->m_prevWptIndex == index)
				return true;
		}
		else
		{
			pointer = g_waypoint->GetPath(index);
			if (pointer != nullptr && ((client.ent->v.origin + client.ent->v.velocity * m_frameInterval) - pointer->origin).GetLengthSquared() < squaredi(pointer->radius + 54))
				return true;
		}
	}

	return false;
}

// this function tries to find nearest to current bot button, and returns pointer to
// it's entity, also here must be specified the target, that button must open
edict_t* Bot::FindNearestButton(const char* className)
{
	if (IsNullString(className))
		return nullptr;

	float distance;
	float nearestDistance = 65355.0f;
	edict_t* searchEntity = nullptr;
	edict_t* foundEntity = nullptr;

	// find the nearest button which can open our target
	while (!FNullEnt(searchEntity = FIND_ENTITY_BY_TARGET(searchEntity, className)))
	{
		distance = (pev->origin - GetEntityOrigin(searchEntity)).GetLengthSquared();
		if (distance < nearestDistance)
		{
			nearestDistance = distance;
			foundEntity = searchEntity;
		}
	}

	return foundEntity;
}