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
		class DelegateBase
		{
			virtual void Invoke(const void* args) = 0;
		};

		template<class T>
		concept Function = std::is_function_v<T>;

		template<class TCallbackArgs, Function TCallbackType>
		class Delegate : public DelegateBase
		{
			TCallbackType callback;
		public:
			void Invoke(const TCallbackArgs* args) final
			{
				callback(args);
			}
		};

		class EventBase
		{
			virtual void RemoveListener(void* listener) = 0;
		};

		template<class TEventArgs>
		class Event : public EventBase
		{
			std::map<void*, DelegateBase*> m_listeners;

		public:
			void AddListener(void* listener, DelegateBase* delegate)
			{
				m_listeners.insert({ listener, delegate });
			}
			void Invoke(TEventArgs args)
			{
				for (auto it : m_listeners)
				{
					it.second->Invoke(&args);
				}
			}
			void RemoveListener(void* listener)
			{
				m_listeners.erase(listener);
			}
		};
	}
}