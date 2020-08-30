#pragma once

#include <cstdint>
#include "ili9341.h"
#include "input/input_keypad.h"

namespace display
{
	using color_t = uint32_t;
	using coord_t = uint16_t;

	class Screen
	{
	public:
		Screen(input::Keypad* keypad);
		virtual ~Screen();
		Screen(const Screen &) = delete;
		Screen &operator=(const Screen &) = delete;

		void Update();
		void BacklightOn();
		void BacklightOff();
		void SetBacklightBrightness(uint8_t brightness);

		void SetBatteryStatus(uint8_t stateOfCharge, bool isCharging);

		void DisplaySleep();
		void DisplayWake();

		inline bool IsInitialized() { return initialized; }
		static constexpr uint32_t GraphicsTickMs = 1;
	private:
		static bool initialized;
		static void Tick(void* context);
		input::Keypad* keypad;
	};
} // namespace display
