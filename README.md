# Header-only C++20 simple state machine which uses my Event Manager

## Description

- Requires [EventManager](https://github.com/3lyrion/EventManager).

- The state must have 3 overrided methods:<br/>
  1. `void subscribe()` – subscribing to events (lambda expressions only)<br/>
  2. `void onEnter()` – activation action<br/>
  3. `void onExit()` – deactivation action<br/>

- The state machine and the states themselves are aware of the owner. Each state knows its own key and can access the machine. States can be changed by key.

- The state machine subscribes only to those events that are used by the states.

- You can turn on/off the status machine. You can also prohibit some states and then allow them.

- You can assign a function to the state machine that can detect the next state.

## Example

```cpp
#define self *this
```

```cpp
namespace CTR_States
{
    class First;
    class Second;
    class Third;
}

class Controller
{
public:
    enum class State
    {
        First = 0,
        Second,
        Third
    };

    Controller() : m_states(self)
    {
        m_states.add(std::make_unique<CTR_States::First>());
        m_states.add(std::make_unique<CTR_States::Second>());
        m_states.add(std::make_unique<CTR_States::Third>());

        m_states.detectState = [this](void const* param)
        {
            auto& some_event = *static_cast<SomeEvent const*>(param);

            if (some_event.value > 10)
                m_states.setNext(States::Second);

            // ...
        };
    }

    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
        m_states.setEnabled(enabled);
    }

    void doSomething1()
    {
        // ...

        m_states.prohibitStates({ State::Second, State::Third });
    }

    void doSomething2()
    {
        // ...

        m_states.allowOnly(State::First);
    }

private:
    bool m_enabled = false;

    el::StateMachine<Controller, State> m_states;
};

namespace CTR_States
{
    using CState = Controller::State;
    using ST_CTR = el::State<Controller, CState>;

    class First : public ST_CTR
    {
    public:
        First() : State(CState::First) { }

    private:
        void subscribe()
        {
            machine->subscribe<SomeEvent>(self,
                [this](SomeEvent const& e)
                {
                    // ...
                    else
                        machine->detectState(&e);
                }
            );

            machine->subscribe<SomeEvent2>(self,
                [this](SomeEvent2 const& e)
                {
                    // ... 
                }
            );
        }

        void onEnter()
        {
            // ...
        }

        void onExit()
        {
            // ...
        }
    }

    // class Second, class Third ...
}

```

## License

This project is under MIT License.

Copyright (c) 2025 3lyrion

> Permission is hereby granted, free of charge, to any person obtaining a copy  
> of this software and associated documentation files (the "Software"), to deal  
> in the Software without restriction, including without limitation the rights  
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell  
> copies of the Software, and to permit persons to whom the Software is  
> furnished to do so, subject to the following conditions:  
> 
> 
> The above copyright notice and this permission notice shall be included in all  
> copies or substantial portions of the Software.  
> 
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,  
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE  
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER  
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  
> SOFTWARE.
