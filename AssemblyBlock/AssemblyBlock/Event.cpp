#include "Event.h"

namespace Engine
{
	namespace Events
	{
		Listener::~Listener()
		{
			DisconnectAll();
		}

		void Listener::Connect(BaseEvent* e)
		{
			m_events.insert(e);
		}
		void Listener::Disconnect(BaseEvent* e)
		{
			m_events.erase(e);
		}
		void Listener::DisconnectAll()
		{
			std::set<BaseEvent*> copy(m_events);
			for (BaseEvent* it : copy)
			{
				it->Disconnect(*this);
			}
		}
	}
}