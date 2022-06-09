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
		class Delegate
		{
		public:
			Delegate() : object_ptr(0), stub_ptr(0) {}

			// Create
			template <class TEventArgs, class T, void (T::*TMethod)(TEventArgs)>
			static Delegate FromMethod(T* object_ptr)
			{
				Delegate d;
				d.object_ptr = object_ptr;
				d.stub_ptr = &MethodStub<TEventArgs, T, TMethod>; // #1
				return d;
			}

			// Call
			void operator()(void* param_ptr) const
			{
				return (*stub_ptr)(object_ptr, param_ptr);
			}

		private:
			void* object_ptr;
			void (*stub_ptr)(void* object_ptr, void* param_ptr);

			template <class TEventArgs, class T, void(T::*TMethod)(void*)>
			static void MethodStub(void* object_ptr, TEventArgs param_ptr)
			{
				T* p = static_cast<T*>(object_ptr);
				return (p->*TMethod)(param_ptr); // #2
			}
		};

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
			// using Callback_t = std::function<void(void*, TEventArgs)>;
			std::map<Listener*, Delegate> m_listeners;

		public:
			~Event() { DisconnectAll(); }

			void operator()(void* sender, TEventArgs args) const
			{
				for (auto it : m_listeners)
				{
					it.second(sender, &args);
				}
			}

			void Connect(Listener& listener, Delegate callback)
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