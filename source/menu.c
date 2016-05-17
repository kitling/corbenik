#include "common.h"
#include "firm/firm.h"
#include "firm/headers.h"
#define MENU_BOOTME  -1
#define MENU_MAIN     1

#define MENU_OPTIONS  2
#define MENU_PATCHES  3
#define MENU_INFO     4
#define MENU_HELP     5
#define MENU_RESET    6
#define MENU_POWER    7

static int cursor_y = 0;
static int which_menu = 1;
static int need_redraw = 1;

uint32_t wait_key() {
    uint32_t get = 0;
    while(get == 0) {
        if(HID_PAD & BUTTON_UP)
            get = BUTTON_UP;
        else if (HID_PAD & BUTTON_DOWN)
            get = BUTTON_DOWN;
        else if (HID_PAD & BUTTON_A)
            get = BUTTON_A;
        else if (HID_PAD & BUTTON_B)
            get = BUTTON_B;
    }
    while(HID_PAD&get);
    return get;
}

void header() {
    fprintf(stdout, "\x1b[33;40m[.corbenik//%s]\x1b[0m\n", VERSION);
}

int menu_patches() { return MENU_MAIN; }

int menu_options() {
    set_cursor(TOP_SCREEN, 0, 0);

    const char *list[] = {
        "Signature Patch",
        "FIRM Write Protection",
        "Inject Loader (NYI)",
		"Inject Services",
        "Enable ARM9 Thread",

        "Autoboot",
        "Silence debug w/ autoboot",
        "Pause for input on steps",

        "Don't draw background color (NYI)",
        "Preserve current framebuffer (NYI)",
        "Hide Readme/Help from menu",

        "Ignore dependencies (NYI)",
        "Allow enabling broken (NYI)",
    };
    const int menu_max = 12;

    header();

    for(int i=0; i < menu_max; i++) {
        if (cursor_y == i)
            fprintf(TOP_SCREEN, "\x1b[32m>>\x1b[0m ");
        else
            fprintf(TOP_SCREEN, "   ");

        if (need_redraw)
			fprintf(TOP_SCREEN, "[%c] %s\n", (config.options[i] ? 'X' : ' '), list[i]);
		else {
			// Yes, this is weird. printf does a large number of extra things we don't
			// want computed at the moment; this is faster.
			putc(TOP_SCREEN, '[');
			putc(TOP_SCREEN, (config.options[i] ? 'X' : ' '));
			putc(TOP_SCREEN, ']');
			putc(TOP_SCREEN, '\n');
		}
    }

	need_redraw = 0;

    uint32_t key = wait_key();

    switch(key) {
        case BUTTON_UP:
            cursor_y -= 1;
            break;
        case BUTTON_DOWN:
            cursor_y += 1;
            break;
        case BUTTON_A:
            // TODO - Value options
            config.options[cursor_y] = !config.options[cursor_y];
            break;
        case BUTTON_B:
			need_redraw = 1;
		    clear_screen(TOP_SCREEN);
			cursor_y = 0;
            return MENU_MAIN;
            break;
    }

    // Loop around the cursor.
    if (cursor_y < 0)
        cursor_y = menu_max - 1;
    if (cursor_y > menu_max - 1)
        cursor_y = 0;

    return 0;
}

int menu_info() {
    clear_screen(TOP_SCREEN);

    set_cursor(TOP_SCREEN, 0, 0);

    header();
	struct firm_signature *native = get_firm_info(firm_loc);
	struct firm_signature *agb = get_firm_info(agb_firm_loc);
	struct firm_signature *twl = get_firm_info(twl_firm_loc);

    fprintf(stdout,     "\nNATIVE_FIRM / Firmware:\n"
                        "  Version: %s (%x)\n"
                        "AGB_FIRM / GBA Firmware:\n"
                        "  Version: %s (%x)\n"
                        "TWL_FIRM / DSi Firmware:\n"
                        "  Version: %s (%x)\n"
                        "\n"
						"[Press any key]\n",
						native->version_string, native->version,
						agb->version_string, agb->version,
						twl->version_string, twl->version);
    while (1) {
        if (wait_key() & BUTTON_ANY)
            break;
    }

	need_redraw = 1;
    clear_screen(TOP_SCREEN);

    return MENU_MAIN;
}

int menu_help() {
    clear_screen(TOP_SCREEN);

    set_cursor(TOP_SCREEN, 0, 0);

    header();

    fprintf(stdout,     "\nCorbenik is a 3DS firmware patching tool;\n"
                        "  commonly known as a CFW. It seeks to address\n"
                        "  some faults in other CFWs and is generally\n"
                        "  just another choice for users - but primarily\n"
                        "  is intended for developers.\n"
                        "\n"
                        "Credits to people who've helped me put this\n"
                        "  together either by code or helpfulness:\n"
                        "  @mid-kid, @Wolfvak, @Reisyukaku, @AuroraWright\n"
                        "  @d0k3, @TuxSH, and others\n"
                        "\n"
                        "[PROTECT BREAK] DATA DRAIN: OK\n"
                        "\n"
                        " <https://github.com/chaoskagami/corbenik>\n"
                        "\n");

    while (1) {
        if (wait_key() & BUTTON_ANY)
            break;
    }

	need_redraw = 1;
    clear_screen(TOP_SCREEN);

    return MENU_MAIN;
}

int menu_reset() {
    fumount(); // Unmount SD.

    // Reboot.
    fprintf(BOTTOM_SCREEN, "Resetting system.\n");
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 2);
    while(1);
}

int menu_poweroff() {
    fumount(); // Unmount SD.

    // Reboot.
    fprintf(BOTTOM_SCREEN, "Powering off system.\n");
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while(1);
}

int menu_main() {
    set_cursor(TOP_SCREEN, 0, 0);

    const char *list[] = {
        "Options",
        "Patches",
        "Info",
        "Help/Readme",
        "Reset",
        "Power off",
        "Boot firmware"
    };
    int menu_max = 7;

    header();

    for(int i=0; i < menu_max; i++) {
        if (!(i+2 == MENU_HELP && config.options[OPTION_READ_ME])) {
	        if (cursor_y == i)
	            fprintf(TOP_SCREEN, "\x1b[32m>>\x1b[0m ");
	        else
	            fprintf(TOP_SCREEN, "   ");

        	if (need_redraw)
	        	fprintf(TOP_SCREEN, "%s\n", list[i]);
			else
				putc(TOP_SCREEN, '\n');
		}
    }

	need_redraw = 0;

    uint32_t key = wait_key();

	int ret = cursor_y+2;

    switch(key) {
        case BUTTON_UP:
            cursor_y -= 1;
			if (config.options[OPTION_READ_ME] && cursor_y+2 == MENU_HELP)
				cursor_y -= 1; // Disable help.
            break;
        case BUTTON_DOWN:
            cursor_y += 1;
			if (config.options[OPTION_READ_ME] && cursor_y+2 == MENU_HELP)
				cursor_y += 1; // Disable help.
            break;
        case BUTTON_A:
			need_redraw = 1;
			cursor_y = 0;
            if (ret == menu_max + 2)
				return MENU_BOOTME; // Boot meh, damnit!
            return ret;
    }

    // Loop around the cursor.
    if (cursor_y < 0)
        cursor_y = menu_max -1;
    if (cursor_y > menu_max - 1)
        cursor_y = 0;

    return 0;
}

int menu_handler() {
    int to_menu = 0;
    switch(which_menu) {
        case MENU_MAIN:
            to_menu = menu_main();
            break;
        case MENU_OPTIONS:
            to_menu = menu_options();
            break;
        case MENU_PATCHES:
            to_menu = menu_patches();
            break;
        case MENU_INFO:
            to_menu = menu_info();
            break;
        case MENU_HELP:
            to_menu = menu_help();
            break;
        case MENU_BOOTME:
            return 0;
        case MENU_RESET:
            menu_reset();
        case MENU_POWER:
            menu_poweroff();
    }

    if (to_menu != 0)
        which_menu = to_menu;

    return 1;
}

