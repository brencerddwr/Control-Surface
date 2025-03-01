/* ✔ */

#pragma once

#include "ExtendedInputOutput.hpp"
#include "StaticSizeExtendedIOElement.hpp"
#include <Helpers/Array.hpp>
#include <stdlib.h>

/**
 * @brief   A class for reading multiplexed analog inputs.
 *          Supports 74HC4067, 74HC4051, etc.
 * 
 * You can use many multiplexers on the same address lines if each of the 
 * multiplexers has a different enable line.
 * 
 * @tparam  N 
 *          The number of address lines.
 * 
 * @ingroup ExtIO
 */
template <uint8_t N>
class AnalogMultiplex : public StaticSizeExtendedIOElement<1 << N> {
  public:
    /**
     * @brief   Create a new AnalogMultiplex object on the given pins.
     * 
     * @param   analogPin
     *          The analog input pin connected to the output of the multiplexer.
     * @param   addressPins
     *          An array of the pins connected to the address lines of the
     *          multiplexer. (Labeled S0, S1, S2 ... in the datasheet.)
     * @param   enablePin
     *          The digital output pin connected to the enable pin of the
     *          multiplexer. (Labeled Ē in the datasheet.)  
     *          If you don't need the enable pin, you can use NO_PIN, which is 
     *          the default.
     */
    AnalogMultiplex(pin_t analogPin, const Array<pin_t, N> &addressPins,
                    pin_t enablePin = NO_PIN)
        : analogPin(analogPin), addressPins(addressPins), enablePin(enablePin) {
    }

    /**
     * @brief   Set the pin mode of the analog input pin.  
     *          This allows you to enable the internal pull-up resistor, for
     *          use with buttons or open-collector outputs.
     * 
     * @note    This applies to all pins of this multiplexer.  
     *          This affects all pins of the multiplexer, because it has only
     *          a single common pin.  
     * 
     * @param   pin
     *          (Unused)
     * @param   mode
     *          The new mode of the input pin: 
     *          either INPUT or INPUT_PULLUP.
     */
    void pinMode(pin_t, uint8_t mode) override;

    /**
     * @brief   The digitalWrite function is not implemented because writing an
     *          output to a multiplexer is not useful.
     */
    void digitalWrite(UNUSED_PARAM pin_t pin, UNUSED_PARAM uint8_t val) override
        __attribute__((deprecated)) {}

    /**
     * @brief   Read the digital state of the given input.
     * 
     * @param   pin
     *          The multiplexer's pin number to read from.
     */
    int digitalRead(pin_t pin) override;

    /**
     * @brief   Read the analog value of the given input.
     * 
     * @param   pin
     *          The multiplexer's pin number to read from.
     */
    analog_t analogRead(pin_t pin) override;

    /**
     * @brief   The analogWrite function is not implemented because writing an
     *          output to a multiplexer is not useful.
     */
    void analogWrite(UNUSED_PARAM pin_t pin, UNUSED_PARAM analog_t val) override
        __attribute__((deprecated)) {}

    /**
     * @brief   Initialize the multiplexer: set the pin mode of the address pins
     *          and the enable pin to output mode.
     */
    void begin() override;

    /**
     * @brief   No periodic updating of the state is necessary, all actions are 
     *          carried out when the user calls analogRead or digitalRead.
     */
    void update() override {}

  private:
    const pin_t analogPin;
    const Array<pin_t, N> addressPins;
    const pin_t enablePin;

    /**
     * @brief   Write the pin number/address to the address pins of the 
     *          multiplexer.
     * 
     * @param   address
     *          The address to select.
     */
    void setMuxAddress(uint8_t address);

    /**
     * @brief   Select the correct address and enable the multiplexer.
     * 
     * @param   address
     *          The address to select.
     */
    void prepareReading(uint8_t address);

    /**
     * @brief   Disable the multiplexer.
     */
    void afterReading();

    // The enable pin is active low.
    constexpr static uint8_t MUX_ENABLED = LOW;
    constexpr static uint8_t MUX_DISABLED = HIGH;
};

/**
 * @brief   An alias for AnalogMultiplex<4> to use with CD74HC4067 analog 
 *          multiplexers.
 * 
 * @ingroup ExtIO
 */
using CD74HC4067 = AnalogMultiplex<4>;

/**
 * @brief   An alias for AnalogMultiplex<3> to use with CD74HC4051 analog 
 *          multiplexers.
 * 
 * @ingroup ExtIO
 */
using CD74HC4051 = AnalogMultiplex<3>;

// -------------------------------------------------------------------------- //

template <uint8_t N>
void AnalogMultiplex<N>::pinMode(pin_t, uint8_t mode) {
    ExtIO::pinMode(analogPin, mode);
}

template <uint8_t N>
int AnalogMultiplex<N>::digitalRead(pin_t pin) {
    prepareReading(pin);
    int result = ExtIO::digitalRead(analogPin);
    afterReading();
    return result;
}

template <uint8_t N>
analog_t AnalogMultiplex<N>::analogRead(pin_t pin) {
    prepareReading(pin);
    ExtIO::analogRead(analogPin); // Discard first reading
    analog_t result = ExtIO::analogRead(analogPin);
    afterReading();
    return result;
}

template <uint8_t N>
void AnalogMultiplex<N>::begin() {
    for (const pin_t &addressPin : addressPins)
        ExtIO::pinMode(addressPin, OUTPUT);
    if (enablePin != NO_PIN) {
        ExtIO::pinMode(enablePin, OUTPUT);
        ExtIO::digitalWrite(enablePin, MUX_DISABLED);
    }
}

template <uint8_t N>
void AnalogMultiplex<N>::setMuxAddress(uint8_t address) {
    uint8_t mask = 1;
    for (const pin_t &addressPin : addressPins) {
        ExtIO::digitalWrite(addressPin, (address & mask) != 0);
        mask <<= 1;
    }
}

template <uint8_t N>
void AnalogMultiplex<N>::prepareReading(uint8_t address) {
    setMuxAddress(address);
    if (enablePin != NO_PIN)
        ExtIO::digitalWrite(enablePin, MUX_ENABLED);
}

template <uint8_t N>
void AnalogMultiplex<N>::afterReading() {
    if (enablePin != NO_PIN)
        ExtIO::digitalWrite(enablePin, MUX_DISABLED);
}