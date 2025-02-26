/*

░██████╗████████╗░█████╗░████████╗███████╗███╗░░░███╗░█████╗░░█████╗░██╗░░██╗██╗███╗░░██╗███████╗
██╔════╝╚══██╔══╝██╔══██╗╚══██╔══╝██╔════╝████╗░████║██╔══██╗██╔══██╗██║░░██║██║████╗░██║██╔════╝
╚█████╗░░░░██║░░░███████║░░░██║░░░█████╗░░██╔████╔██║███████║██║░░╚═╝███████║██║██╔██╗██║█████╗░░
░╚═══██╗░░░██║░░░██╔══██║░░░██║░░░██╔══╝░░██║╚██╔╝██║██╔══██║██║░░██╗██╔══██║██║██║╚████║██╔══╝░░
██████╔╝░░░██║░░░██║░░██║░░░██║░░░███████╗██║░╚═╝░██║██║░░██║╚█████╔╝██║░░██║██║██║░╚███║███████╗
╚═════╝░░░░╚═╝░░░╚═╝░░╚═╝░░░╚═╝░░░╚══════╝╚═╝░░░░░╚═╝╚═╝░░╚═╝░╚════╝░╚═╝░░╚═╝╚═╝╚═╝░░╚══╝╚══════╝

Header-only C++20 simple state machine which uses my Event Manager
https://github.com/3lyrion/StateMachine
https://github.com/3lyrion/EventManager

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
Copyright (c) 2025 3lyrion

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files( the "Software" ), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#pragma once

#include <EventManager.hpp>

#include <any>
#include <unordered_set>

namespace el
{

template <typename T>
concept EnumClass = std::is_enum_v<T> && std::is_convertible_v<T, int>;

template <typename Owner, EnumClass Key>
class StateMachine;

template <typename Owner, EnumClass Key>
class State
{
	using Machine = StateMachine<Owner, Key>;

	friend class Machine;

public:
	const Key KEY;

	virtual ~State() = default;

protected:
	Owner*   owner{};
	Machine* machine{};

	State(Key key) :
		KEY(key) { }

	virtual void subscribe() = 0;

	virtual void onEnter	() = 0;
	virtual void onExit		() = 0;
};


template <typename Owner, EnumClass Key>
class StateMachine : public el::EventReceiver
{
public:
	std::function<void(void*)> detectState{};

	using MachineState = State<Owner, Key>;

	template <DerivedFromEventBase EventType>
	constexpr void subscribe(MachineState& state, std::function<void(EventType const&)>&& action)
	{
		auto& tid = typeid(EventType);

		auto entry = m_subscriptions.find(tid);
		if (entry == m_subscriptions.end())
		{
			EM.subscribe<EventType>(*this,
				[this, &tid](EventType const& e)
				{
					if (!m_enabled)
						return;

					auto& subs = m_subscriptions[tid];

					auto entry = subs.find(m_currState->KEY);
					if (entry != subs.end())
						std::any_cast<std::function<void(EventType const&)> >(entry->second)(e);
				}
			);

			m_subscriptions[tid][state.KEY] = std::move(action);
		}

		else
			entry->second[state.KEY] = std::move(action);
	}

public:
	StateMachine(const StateMachine&)              = delete;
	StateMachine& operator = (const StateMachine&) = delete;

	StateMachine(Owner& theOwner) :
		owner(theOwner) { }

	void add(std::unique_ptr<MachineState>&& state)
	{
		if (!m_currState)
			m_currState = state.get();

		state->owner   = &owner;
		state->machine = this;
		state->subscribe();

		m_states.emplace(state->KEY, std::move(state));
	}

	void prohibitState(Key key)
	{
		m_prohibited.insert(key);
	}

	void prohibitStates(std::unordered_set<Key>&& keys)
	{
		m_prohibited = std::move(keys);
	}

	void prohibitAllStates()
	{
		for (auto& [_key, _] : m_states)
			prohibitState(_key);
	}

	void allowState(Key key)
	{
		m_prohibited.erase(key);
	}

	void allowOnly(Key key)
	{
		prohibitAllStates();

		allowState(key);
	}

	void allowOnly(std::unordered_set<Key>&& keys)
	{
		prohibitAllStates();

		for (auto _key : keys)
			allowState(_key);
	}

	void allowAllStates()
	{
		m_prohibited.clear();
	}

	void setEnabled(bool enabled)
	{
		/*if (m_enabled == enabled)
			return;*/

		m_enabled = enabled;

		//if (enabled) m_currState->onEnter();
		//else         m_currState->onExit();
	}

	void setNext(Key key)
	{
		if (m_prohibited.contains(key) || m_currState->KEY == key)
			return;

		auto entry = m_states.find(key);
		if (entry != m_states.end())
		{
			m_currState->onExit();

			allowAllStates();

			m_prevState = m_currState;
			m_currState = entry->second.get();

			m_currState->onEnter();

		}
	}

	MachineState const* getCurrentState() const
	{
		return m_currState;
	}

	MachineState const* getPreviousState() const
	{
		return m_prevState;
	}

private:
	using StatePtr  = std::unique_ptr<MachineState>;
	using ActionMap = std::unordered_map<Key, std::any>;

	Owner& owner;

	bool m_enabled = true;

	MachineState*									m_currState{};
	MachineState*									m_prevState{};
	std::unordered_map<std::type_index, ActionMap>	m_subscriptions;
	std::unordered_map<Key, StatePtr>				m_states;
	std::unordered_set<Key>							m_prohibited;
};

} // namespace el