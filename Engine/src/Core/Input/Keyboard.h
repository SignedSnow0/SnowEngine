#pragma once

namespace SnowEngine {
    class Keyboard
    {
        friend class Window;
    public:
        static bool IsKeyPressed(int keycode);

        Keyboard() = delete;
        Keyboard(const Keyboard&) = delete;
        Keyboard& operator=(const Keyboard&) = delete;
        ~Keyboard() = default;

    private:
        static void Init();
        static void SetKey(int keycode, bool value);
    };

#define SNOW_KEY_SPACE              32
#define SNOW_KEY_APOSTROPHE         39
#define SNOW_KEY_COMMA              44
#define SNOW_KEY_MINUS              45
#define SNOW_KEY_PERIOD             46
#define SNOW_KEY_SLASH              47
#define SNOW_KEY_0                  48
#define SNOW_KEY_1                  49
#define SNOW_KEY_2                  50
#define SNOW_KEY_3                  51
#define SNOW_KEY_4                  52
#define SNOW_KEY_5                  53
#define SNOW_KEY_6                  54
#define SNOW_KEY_7                  55
#define SNOW_KEY_8                  56
#define SNOW_KEY_9                  57
#define SNOW_KEY_SEMICOLON          59
#define SNOW_KEY_EQUAL              61
#define SNOW_KEY_A                  65
#define SNOW_KEY_B                  66
#define SNOW_KEY_C                  67
#define SNOW_KEY_D                  68
#define SNOW_KEY_E                  69
#define SNOW_KEY_F                  70
#define SNOW_KEY_G                  71
#define SNOW_KEY_H                  72
#define SNOW_KEY_I                  73
#define SNOW_KEY_J                  74
#define SNOW_KEY_K                  75
#define SNOW_KEY_L                  76
#define SNOW_KEY_M                  77
#define SNOW_KEY_N                  78
#define SNOW_KEY_O                  79
#define SNOW_KEY_P                  80
#define SNOW_KEY_Q                  81
#define SNOW_KEY_R                  82
#define SNOW_KEY_S                  83
#define SNOW_KEY_T                  84
#define SNOW_KEY_U                  85
#define SNOW_KEY_V                  86
#define SNOW_KEY_W                  87
#define SNOW_KEY_X                  88
#define SNOW_KEY_Y                  89
#define SNOW_KEY_Z                  90
#define SNOW_KEY_LEFT_BRACKET       91 
#define SNOW_KEY_BACKSLASH          92 
#define SNOW_KEY_RIGHT_BRACKET      93 
#define SNOW_KEY_GRAVE_ACCENT       96 
#define SNOW_KEY_WORLD_1            161
#define SNOW_KEY_WORLD_2            162

#define SNOW_KEY_ESCAPE             256
#define SNOW_KEY_ENTER              257
#define SNOW_KEY_TAB                258
#define SNOW_KEY_BACKSPACE          259
#define SNOW_KEY_INSERT             260
#define SNOW_KEY_DELETE             261
#define SNOW_KEY_RIGHT              262
#define SNOW_KEY_LEFT               263
#define SNOW_KEY_DOWN               264
#define SNOW_KEY_UP                 265
#define SNOW_KEY_PAGE_UP            266
#define SNOW_KEY_PAGE_DOWN          267
#define SNOW_KEY_HOME               268
#define SNOW_KEY_END                269
#define SNOW_KEY_CAPS_LOCK          280
#define SNOW_KEY_SCROLL_LOCK        281
#define SNOW_KEY_NUM_LOCK           282
#define SNOW_KEY_PRINT_SCREEN       283
#define SNOW_KEY_PAUSE              284
#define SNOW_KEY_F1                 290
#define SNOW_KEY_F2                 291
#define SNOW_KEY_F3                 292
#define SNOW_KEY_F4                 293
#define SNOW_KEY_F5                 294
#define SNOW_KEY_F6                 295
#define SNOW_KEY_F7                 296
#define SNOW_KEY_F8                 297
#define SNOW_KEY_F9                 298
#define SNOW_KEY_F10                299
#define SNOW_KEY_F11                300
#define SNOW_KEY_F12                301
#define SNOW_KEY_F13                302
#define SNOW_KEY_F14                303
#define SNOW_KEY_F15                304
#define SNOW_KEY_F16                305
#define SNOW_KEY_F17                306
#define SNOW_KEY_F18                307
#define SNOW_KEY_F19                308
#define SNOW_KEY_F20                309
#define SNOW_KEY_F21                310
#define SNOW_KEY_F22                311
#define SNOW_KEY_F23                312
#define SNOW_KEY_F24                313
#define SNOW_KEY_F25                314
#define SNOW_KEY_KP_0               320
#define SNOW_KEY_KP_1               321
#define SNOW_KEY_KP_2               322
#define SNOW_KEY_KP_3               323
#define SNOW_KEY_KP_4               324
#define SNOW_KEY_KP_5               325
#define SNOW_KEY_KP_6               326
#define SNOW_KEY_KP_7               327
#define SNOW_KEY_KP_8               328
#define SNOW_KEY_KP_9               329
#define SNOW_KEY_KP_DECIMAL         330
#define SNOW_KEY_KP_DIVIDE          331
#define SNOW_KEY_KP_MULTIPLY        332
#define SNOW_KEY_KP_SUBTRACT        333
#define SNOW_KEY_KP_ADD             334
#define SNOW_KEY_KP_ENTER           335
#define SNOW_KEY_KP_EQUAL           336
#define SNOW_KEY_LEFT_SHIFT         340
#define SNOW_KEY_LEFT_CONTROL       341
#define SNOW_KEY_LEFT_ALT           342
#define SNOW_KEY_LEFT_SUPER         343
#define SNOW_KEY_RIGHT_SHIFT        344
#define SNOW_KEY_RIGHT_CONTROL      345
#define SNOW_KEY_RIGHT_ALT          346
#define SNOW_KEY_RIGHT_SUPER        347
#define SNOW_KEY_MENU               348

#define SNOW_KEY_LAST               SNOW_KEY_MENU
}