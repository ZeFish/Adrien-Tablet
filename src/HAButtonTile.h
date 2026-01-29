#ifndef _HA_BUTTON_TILE_H_
#define _HA_BUTTON_TILE_H_

/*
 * HAButtonTile.h
 *
 * Small self-contained UI tile that encapsulates:
 *  - a visual tile (title/subtitle/icons)
 *  - a toggle/button control (EPDGUI_Switch)
 *  - optional +/- numeric parameter controls
 *  - behavior to call Home Assistant services (via ha_api helpers)
 *
 * Modes:
 *  - MODE_TOGGLE_ENTITY: regular domain/entity toggle (turn_on / turn_off)
 *  - MODE_SCRIPT_TOGGLE: switch that calls script.turn_on with different variables for ON and OFF
 *  - MODE_SCRIPT_BUTTON: one-shot script trigger (runs script.turn_on on press)
 *
 * The tile keeps its drawing and event callbacks internal so it behaves like a
 * simple OOP component you can instantiate and configure from `HomeControl`.
 *
 * Note: This header declares the interface. Implementation goes in HAButtonTile.cpp.
 */

#include <Arduino.h>

class EPDGUI_Switch;
class EPDGUI_Button;
struct epdgui_args_vector_t;

class HAButtonTile {
public:
    enum Mode {
        MODE_TOGGLE_ENTITY = 0,
        MODE_SCRIPT_TOGGLE = 1,
        MODE_SCRIPT_BUTTON = 2
    };

    // Configuration structure - set values in code (or from macros) and pass to init()
    struct Config {
        Mode mode = MODE_TOGGLE_ENTITY;

        // Visuals
        String title = "";
        String subtitle = "";
        const uint8_t *icon_off = nullptr;
        const uint8_t *icon_on  = nullptr;

        // Entity / script target
        // - For MODE_TOGGLE_ENTITY: set entity to "light.kitchen"
        // - For MODE_SCRIPT_*, set to "script.my_script"
        String entity = "";

        // Optional JSON strings for script variables (used for script toggle)
        // Example: "{\"mode\":\"night\"}"
        String onVars  = "";
        String offVars = "";

        // Optional numeric parameter controls (useful for numeric script parameters)
        // When paramIsNumeric == true, the tile shows + / - and the numeric value is
        // passed as `paramName` in variables for script calls if specific onVars/offVars
        // are not supplied.
        bool paramIsNumeric = false;
        String paramName = "value";
        int paramInit = 0;
        int paramMin  = 0;
        int paramMax  = 100;
        int paramStep = 1;

        // Position and size (defaults mirror current tiles)
        int x = 0;
        int y = 0;
        int w = 228;
        int h = 228;
    };

    HAButtonTile();
    ~HAButtonTile();

    // Initialize the tile (prepare canvases, store config)
    // After init() call attach() to add it to the GUI.
    void init(const Config &cfg);

    // Add internal GUI objects to the global EPDGUI object list (calls EPDGUI_AddObject)
    // Call this once after init() when you want the tile to be visible.
    void attach();

    // Parameter accessors (used when paramIsNumeric == true)
    void setParam(int v);
    int  getParam() const;

private:
    Config _cfg;

    // Underlying GUI controls
    EPDGUI_Switch *_sw = nullptr; // used for both toggles and one-shot button (we flip back on one-shot)
    EPDGUI_Button *_plus = nullptr;
    EPDGUI_Button *_minus = nullptr;

    // Numeric parameter (if used)
    int _param = 0;

    // Redraws the tile canvases to reflect title/subtitle/icon/param/state
    void redraw();

    // Static callbacks (connected to GUI)
    static void _sw_cb(epdgui_args_vector_t &args);
    static void _plus_cb(epdgui_args_vector_t &args);
    static void _minus_cb(epdgui_args_vector_t &args);

    // Instance handlers called from static callbacks
    void onSwitchChanged(int state);
    void onPlusPressed();
    void onMinusPressed();
};

#endif // _HA_BUTTON_TILE_H_