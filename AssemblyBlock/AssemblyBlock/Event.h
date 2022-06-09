#pragma once
#include <functional>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include <vector>

namespace Engine
{
	namespace Events
	{
		class Listener;

		class BaseEvent
		{
		public:
			virtual void Disconnect(Listener& listener) = 0;
		};

		class Listener
		{
			template<class TEventArgs> friend class Event;

		public:
			~Listener();

		private:
			void Connect(BaseEvent* e);
			void Disconnect(BaseEvent* e);
			void DisconnectAll();
			std::set<BaseEvent*> m_events;
		};

		template<class TEventArgs>
		class Event : public BaseEvent
		{
		private:
			using Callback_t = std::function<void(void*, TEventArgs)>;
			std::map<Listener*, Callback_t> m_listeners;

		public:
			~Event() { DisconnectAll(); }

			void operator()(void* sender, TEventArgs args) const
			{
				for (auto it : m_listeners)
				{
					it.second(sender, args);
				}
			}

			void Connect(Listener& listener, Callback_t callback)
			{
				listener.Connect(this);
				m_listeners.insert({ &listener, callback });
			}
			
			void Disconnect(Listener& listener)
			{
				listener.Disconnect(this);
				m_listeners.erase(&listener);
			}
			void DisconnectAll()
			{
				for (auto it : m_listeners)
				{
					it.first->Disconnect(this);
				}
				m_listeners.clear();
			}
		};
	}
}