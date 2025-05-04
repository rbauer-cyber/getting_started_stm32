#include "main.h"
#include "fastRotaryEncoder.hpp"
#include "console.h"

enum EEncoderState 
{
    DIR_NONE    = 0x00,           // No complete step yet.
    DIR_CW      = 0x10,           // Clockwise step.
    DIR_CCW     = 0x20,           // Anti-clockwise step.
    R_START     = 0x03,
    R_CW_BEGIN  = 0x01,
    R_CW_NEXT   = 0x00,
    R_CW_FINAL  = 0x02,
    R_CCW_BEGIN = 0x06,
    R_CCW_NEXT  = 0x04,
    R_CCW_FINAL = 0x05
};

const uint8_t s_ttable[8][4] =
{
    // Clockwise Rotation States
    {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},                // R_CW_NEXT
    {R_CW_NEXT,  R_CW_BEGIN,  R_CW_BEGIN,  R_START},                // R_CW_BEGIN
    {R_CW_NEXT,  R_CW_FINAL,  R_CW_FINAL,  R_START | DIR_CW},       // R_CW_FINAL
    // Startup, waiting for direction, either CW or CCW
    {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},                // R_START
    // Counter Clockwise Rotation States
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},                // R_CCW_NEXT
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_FINAL, R_START | DIR_CCW},      // R_CCW_FINAL
    {R_CCW_NEXT, R_CCW_BEGIN, R_CCW_BEGIN, R_START},                // R_CCW_BEGIN
    {R_START,    R_START,     R_START,     R_START}                 // ILLEGAL
};

uint8_t s_stateHistory[200];
uint8_t s_stateIndex = 0;

//void FastRotaryChanged(); //we need to declare the func above so Rotary goes to the one below

// Initialize singleton for Fast Rotary Fncoder
// Pins 3 (DT), 2 (CLK), 1 (SW)
//CFastRotaryEncoder FastRotaryEncoder(&FastRotaryChanged, kDigitalPin05, kDigitalPin04, 0);
CFastRotaryEncoder FastRotaryEncoder(kDigitalPin05, kDigitalPin04, 0);

void FastRotaryChanged()
{
    const unsigned int state = FastRotaryEncoder.GetState();

    if (state & DIR_CW)  
        FastRotaryEncoder.Increment();
    else if (state & DIR_CCW)  
        FastRotaryEncoder.Decrement(); 

    if ( s_stateIndex >= 200 ) s_stateIndex = 0;
    s_stateHistory[s_stateIndex++] = state;
}

void CFastRotaryEncoder::FastRotaryChangedCallback()
{
    const unsigned int state = FastRotaryEncoder.GetState();

    if (state & DIR_CW)
        FastRotaryEncoder.Increment();
    else if (state & DIR_CCW)
        FastRotaryEncoder.Decrement();

    if ( s_stateIndex >= 200 ) s_stateIndex = 0;
    s_stateHistory[s_stateIndex++] = state;
}

void CFastRotaryEncoder::Increment()
{
    m_lastCount = m_counter;
    m_counter += 1;
}

void CFastRotaryEncoder::Decrement()
{
    m_lastCount = m_counter;
    m_counter -= 1;
}

void CFastRotaryEncoder::SetPosition(int position)
{
    m_counter = position;
}

int CFastRotaryEncoder::GetPosition()
{
    return m_counter;
}

const uint8_t s_numbersPerLine = 20;

void CFastRotaryEncoder::DumpStateTable()
{
#if 0
    Serial.print("Dump State Table Index: "); Serial.println(s_stateIndex);

    for ( size_t i = 0; i < s_stateIndex; i += s_numbersPerLine )
    {
        for ( size_t j = 0; j < s_numbersPerLine && (j+i) < s_stateIndex; j++ )
        {
            Serial.print((s_stateHistory[j+i]), HEX);
            Serial.print(',');
        }

        Serial.println(' ');
    }

    Serial.println(' ');
    s_stateIndex = 0;
#endif
}

CFastRotaryEncoder::CFastRotaryEncoder(
    const CallbackFunc FastRotaryCallback,
    const uint8_t DT_Pin, const uint8_t CLK_Pin, const uint8_t SW_Pin)
: FastRotaryCallback(FastRotaryCallback), m_DT_Pin(DT_Pin), m_CLK_Pin(CLK_Pin), m_SW_Pin(SW_Pin)
{
    //pinMode(m_DT_Pin, INPUT);
    //pinMode(m_CLK_Pin, INPUT);
    //pinMode(m_SW_Pin, INPUT);
    m_buttonDown = 0;
    m_counter = 2;
    m_lastCount = 2; 

    //attachInterrupt(digitalPinToInterrupt(DT_Pin), this->FastRotaryCallback, CHANGE);
    //attachInterrupt(digitalPinToInterrupt(CLK_Pin), this->FastRotaryCallback, CHANGE);
}

CFastRotaryEncoder::CFastRotaryEncoder(
    const uint8_t DT_Pin, const uint8_t CLK_Pin, const uint8_t SW_Pin)
:   FastRotaryCallback(FastRotaryChanged), m_DT_Pin(DT_Pin), m_CLK_Pin(CLK_Pin), m_SW_Pin(SW_Pin)
{
    //pinMode(m_DT_Pin, INPUT);
    //pinMode(m_CLK_Pin, INPUT);
    //pinMode(m_SW_Pin, INPUT);
    m_buttonDown = 0;
    m_counter = 2;
    m_lastCount = 2;
}

int CFastRotaryEncoder::Setup()
{
    uint8_t dt = static_cast<uint8_t>( CDigitalOut::Read(EDigitalPin(m_DT_Pin)) );
    uint8_t clk = static_cast<uint8_t>( CDigitalOut::Read(EDigitalPin(m_CLK_Pin)) );

    consoleDisplay("initializing Fast Rotary Encoder\r\n");
    HAL_Delay(1000);

	m_state = (dt << 1) | clk;
    m_buttonDown = 0;
    m_counter = 800;
    consoleDisplayArgs("(MSB)DT = %d\r\n", dt);
    consoleDisplayArgs("(LSB)CLK = %d\r\n", clk);
    consoleDisplayArgs("state = %d\r\n", m_state);

    //int position = GetPosition();
    //Serial.println("Fast Rotary Encoder initialized");
    //Serial.print("Encoder position = "); Serial.println( position );
    return m_counter;
}
	
const unsigned int CFastRotaryEncoder::GetState()
{		
    const unsigned char pinstate =
    		(CDigitalOut::Read(EDigitalPin(m_DT_Pin)) << 1) |
			CDigitalOut::Read(EDigitalPin(m_CLK_Pin));
    m_state = s_ttable[m_state & 0x07][pinstate];
    return m_state;
}
	
const bool CFastRotaryEncoder::GetButtonDown()
{
#if 0
    bool newButtonState = !digitalRead(m_SW_Pin);

    if ( newButtonState != m_buttonDown )
    {
        m_buttonDown = newButtonState;

        if ( m_buttonDown )
        {
            m_counter = 200;
            m_lastCount = m_counter;
        }
    }
    return m_buttonDown;
#else
    return false;
#endif

}


