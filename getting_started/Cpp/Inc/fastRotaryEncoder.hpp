#ifndef FastRotaryEncoder_h
#define FastRotaryEncoder_h

#include <stdint.h>
#include <digitalOut.hpp>

using CallbackFunc = void(*)();

class CFastRotaryEncoder
{
protected:
	const CallbackFunc FastRotaryCallback;
    const uint8_t m_DT_Pin, m_CLK_Pin, m_SW_Pin;
	unsigned int m_state;
	int m_buttonDown;
    int m_counter;
    int m_lastCount; 

public:
    CFastRotaryEncoder(const CallbackFunc FastRotaryCallback, 
        const uint8_t DT_Pin=3, const uint8_t CLK_Pin=2, const uint8_t SW_Pin=0);

    CFastRotaryEncoder(const uint8_t DT_Pin=3, const uint8_t CLK_Pin=2, const uint8_t SW_Pin=0);

    // Callback function
    static void FastRotaryChangedCallback();

    int Setup();
	const unsigned int GetState();
	const bool GetButtonDown();
    void SetPosition(int position);
    int GetPosition();
    void Increment();
    void Decrement();
    void DumpStateTable();
};

extern CFastRotaryEncoder FastRotaryEncoder;
#endif
