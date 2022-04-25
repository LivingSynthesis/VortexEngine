#include "RingMenu.h"

#include "LedControl.h"
#include "Button.h"

// how long must hold to trigger ring menu
#define MENU_TRIGGER_THRESHOLD 10
// how long each ring menu takes to fill
#define MENU_DURATION 30

RingMenu::RingMenu() :
  m_randomizer(),
  m_colorSelect(),
  m_patternSelect(),
  m_globalBrightness(),
  m_factoryReset(),
  m_modeSharing(),
  //m_menuList(),
  m_selection(0),
  m_isOpen(false)
{
}

bool RingMenu::init()
{
  // Some sort of auto-registration mechanism for this would be nice
  // but in reality how often are people going to create new Menus
  return true;
}

void RingMenu::registerMenu(Menu *menu, RGBColor color)
{
  //m_menuList.push_back(MenuEntry(menu, color));
}

Menu *RingMenu::run()
{
  // if the button was released this tick and the ringmenu was open 
  // then close the ringmenu and return the current menu selection
  if (g_pButton->onRelease() && m_isOpen) {
    // the menu is no longer open
    m_isOpen = false;
    // return the menu that was selected
    return m_menuList[m_selection].menu;
  }
  // make sure the button is pressed and held for at least one second
  if (!g_pButton->isPressed() || g_pButton->holdDuration() < MENU_TRIGGER_THRESHOLD) {
    // no menu selected yet
    return nullptr;
  }
  // if the ring menu just opened this tick
  if (!m_isOpen) {
    // reset the current selection just in case
    m_selection = 0;
    // the ring menu is now open
    m_isOpen = true;
    //Debug("RingMenu open");
  }
  // calculate how long into the current menu the button was held
  // this will be a value between 0 and LED_COUNT based on the current
  // menu selection and hold time
  // the leds turn on in sequence every tick another turns on:
  //  000ms = led 0 to 0
  //  100ms = led 0 to 1
  //  200ms = led 0 to 2
  LedPos led = calcLedPos();
  //Debug("Led: %d", (uint32_t)led);
  // turn on leds LED_FIRST through led with the selected menu's given color
  g_pLedControl->setRange(LED_FIRST, led, m_menuList[m_selection].color);
  // no menu selected yet
  return nullptr;
}

// helper to calculate the relative hold time for the current menu
LedPos RingMenu::calcLedPos()
{
  // this allows the menu to wrap around to beginning after the end
  // if the user never lets go of the button
  int holdDuration = g_pButton->holdDuration() % (MENU_TRIGGER_THRESHOLD + (MENU_DURATION * numMenus()));
  if (holdDuration < MENU_TRIGGER_THRESHOLD) {
      holdDuration = MENU_TRIGGER_THRESHOLD;
  }
  // the time when the current menu starts trigger threshold + duration per menu
  int menuStartTime = MENU_TRIGGER_THRESHOLD + (MENU_DURATION * m_selection);
  if (holdDuration >= menuStartTime) {
    // the amount of time held in the current menu, should be 0 to MENU_DURATION ticks
    int holdTime = (holdDuration - menuStartTime);
    // if the holdTime is within MENU_DURATION then it's valid
    if (holdTime < MENU_DURATION) {
      return (LedPos)(((double)holdTime / MENU_DURATION) * LED_COUNT);
    }
  }
  // otherwise increment selection and wrap around at numMenus
  m_selection = (m_selection + 1) % numMenus();
  // then re-calculate the holdTime it should be less than 10
  return calcLedPos();
}
