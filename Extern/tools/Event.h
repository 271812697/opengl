#pragma once
#include <functional>
namespace Tools::Eventing
{
	using ListenerID = uint64_t;
	template<class... ArgTypes>
	class Event
	{
	public:

		using Callback = std::function<void(ArgTypes...)>;

		ListenerID AddListener(Callback p_callback);

		ListenerID operator+=(Callback p_callback);

		bool RemoveListener(ListenerID p_listenerID);

		bool operator-=(ListenerID p_listenerID);

		void RemoveAllListeners();

		uint64_t GetListenerCount();

		void Invoke(ArgTypes... p_args);

	private:
		std::unordered_map<ListenerID, Callback>	m_callbacks;
		ListenerID									m_availableListenerID = 0;
	};
}