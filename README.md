# Adrien's Tablet

This project is a custom, open-source firmware for the M5Paper E-Ink device, designed to create a simple, safe, and personalized tablet for my 4-year-old son, Adrien.

## The "Why"

In a world of hyper-stimulating, addictive screens, I wanted to create something different for my child. A device that could connect him to the digital world in a calmer, more intentional way. The M5Paper, with its beautiful, paper-like E-Ink display, is the perfect canvas.

This tablet is a sandboxed environment where he can have his own "internet in his pocket," but in a way that I can guide and curate. It's a place for him to check the weather to see if he needs a coat, to control his bedroom lights via Home Assistant, to read simple stories, and to play simple, creative games. It's a tool, not just a toy.

## The Architecture: A "Kernel" for E-Ink Apps

The firmware is designed with a simple, powerful, and modular architecture, much like a mini-operating system or "kernel."

The core system is responsible for the basics:
- Hardware Initialization (E-Ink screen, touch, battery)
- WiFi Connectivity
- A clean, consistent GUI framework

All applications are treated as self-contained modules that are "loaded" by the kernel at startup. They live in their own folders inside the `src/apps/` directory.

### Key Principles

- **Modular:** Every app is an independent unit.
- **Decoupled:** The core system doesn't know or care about the specifics of any individual app. It only communicates with them through a common `AppBase` interface.
- **Easy to Extend:** Adding a new application is a straightforward and simple process.

## How to Create a New App

Thanks to the new architecture, adding a new application is incredibly simple.

### 1. Create Your App Folder

Create a new folder for your app inside `src/apps/`. For example: `src/apps/MyNewApp/`.

### 2. Create Your App Files

Inside your new folder, create your header and source files. At a minimum, you'll need a header file (e.g., `MyNewApp.h`) that defines your app's class.

Your app must inherit from `AppBase` and implement a few key methods:

```cpp
// src/apps/MyNewApp/MyNewApp.h
#ifndef _MY_NEW_APP_H_
#define _MY_NEW_APP_H_

#include "core/gui/AppBase.h"
#include "core/resources/ImageResource.h" // For icons

class MyNewApp : public AppBase {
public:
    // The name that appears in the top bar
    MyNewApp() : AppBase("My App") {}

    // The icon that appears on the home screen
    const uint8_t* GetIcon() override {
        // You can use a default icon or create your own!
        return ImageResource_main_icon_default_92x92;
    }

    // This is where your app's logic goes!
    // It's called when the user launches your app.
    int init(epdgui_args_vector_t &args) override {
        _is_run = 1;
        M5.EPD.Clear(); // Clear the screen

        // Create a canvas to draw on
        M5EPD_Canvas canvas(&M5.EPD);
        canvas.createCanvas(540, 800); // Create a canvas at 0, 100 (below the title bar)
        canvas.setTextSize(36);
        canvas.drawString("Hello, Adrien!", 100, 100);
        canvas.pushCanvas(0, 100, UPDATE_MODE_GC16); // Push to screen

        // Provides a standard "back" button
        exitbtn("Back");
        _key_exit->AddArgs(EPDGUI_Button::EVENT_RELEASED, 0, (void *)(&_is_run));
        _key_exit->Bind(EPDGUI_Button::EVENT_RELEASED, &Frame_Base::exit_cb);

        return 3;
    }
};

#endif
```

### 3. Register Your App

Open `src/core/system/AppLoader.h` and add your app to the list.

First, include your new header file:

```cpp
// ... other includes
#include "apps/MyNewApp/MyNewApp.h"
```

Then, add it to the `GetApps()` vector, wrapped in an `ENABLE_APP` define:

```cpp
// In AppRegistry::GetApps()
#ifdef ENABLE_APP_MYNEWAPP
        apps.push_back(new MyNewApp());
#endif
```

### 4. Enable Your App

Finally, open `src/core/config/SystemConfig.h` and add the define to enable your app:

```cpp
// ... other defines
#define ENABLE_APP_MYNEWAPP
```

That's it! The next time you build and upload the firmware, your new app will appear in the launcher.